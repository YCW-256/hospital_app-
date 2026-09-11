#include "cameraserial.h"

#include <QSerialPort>
#include <QTimer>

// 下行帧约定（PC→RV1106），与参考工程 SerialWorker 一致
static constexpr quint8  kDownFrameHead = 0x5A;  // 下行帧头
static constexpr quint8  kUpFrameHead   = 0x5B;  // 上行帧头（RV1106 推理结果回报）
static constexpr quint8  kFrameTail     = 0xA5;  // 上下行共用帧尾
static constexpr quint16 kCmdStartVideo = 0x0001;   // 自动播放/推视频指令
static constexpr quint16 kCmdTongueDetect = 0x0010; // 单帧舌苔推理指令（检测到即上行回报）
static constexpr int     kUpFrameLen    = 6;      // 上行推理帧固定 6 字节
static constexpr int     kConfMaxHex    = 0x64;   // 置信度最大编码 = 100%

CameraSerial::CameraSerial(const QString &portName, qint32 baudRate, QObject *parent)
    : QObject(parent)
    , m_serial(new QSerialPort(this))
    , m_retryTimer(new QTimer(this))
    , m_portName(portName)
    , m_baudRate(baudRate)
    , m_running(false)
{
    // 打开失败自动重试（单发去重）
    m_retryTimer->setSingleShot(true);
    m_retryTimer->setInterval(3000);
    connect(m_retryTimer, &QTimer::timeout, this, &CameraSerial::start);
}

CameraSerial::~CameraSerial()
{
    stop();
}

void CameraSerial::start()
{
    m_running = true;
    m_retryTimer->stop(); // 已尝试打开，取消可能挂起的重试

    if (m_serial->isOpen()) {
        emit logMessage(QStringLiteral("[串口] %1 已处于打开状态").arg(m_portName));
        return;
    }

    m_serial->setPortName(m_portName);
    m_serial->setBaudRate(m_baudRate);
    m_serial->setDataBits(QSerialPort::Data8);
    m_serial->setParity(QSerialPort::NoParity);
    m_serial->setStopBits(QSerialPort::OneStop);
    m_serial->setFlowControl(QSerialPort::NoFlowControl);

    if (!m_serial->open(QIODevice::ReadWrite)) {
        emit connected(false);
        emit logMessage(QStringLiteral("").arg(m_portName, m_serial->errorString()));
        scheduleRetry(); // 设备未枚举/端口被占用时稍后重试
        return;
    }

    connect(m_serial, &QSerialPort::readyRead, this, &CameraSerial::onReadyRead);
    emit connected(true);
    emit logMessage(QStringLiteral("[串口] 已连接 %1 @ %2").arg(m_portName).arg(m_baudRate));

    // 连接成功自动下发 0x0001：让摄像头开始推视频（随后 DeviceCamera 经 TCP 收图）
    sendDownFrame(kCmdStartVideo);
}

void CameraSerial::stop()
{
    m_running = false;        // 关闭自动重试开关
    m_retryTimer->stop();     // 取消待执行的重试

    if (m_serial && m_serial->isOpen()) {
        disconnect(m_serial, &QSerialPort::readyRead, this, &CameraSerial::onReadyRead);
        m_serial->close();
    }
    emit connected(false);
    emit logMessage(QStringLiteral("[串口] 串口已断开"));
}

void CameraSerial::triggerTongueDetect()
{
    if (!m_serial->isOpen()) {
        emit logMessage(QStringLiteral("[串口] 串口未连接，无法触发舌苔检测"));
        return;
    }
    // 下发 0x0010：RV1106 进入单帧推理，当前画面一旦检测到舌苔即上行回报 6 字节帧
    sendDownFrame(kCmdTongueDetect);
}

void CameraSerial::resumeVideo()
{
    if (!m_serial->isOpen()) {
        emit logMessage(QStringLiteral("[串口] 串口未连接，无法恢复视频"));
        return;
    }
    // 下发 0x0001：设备回到"仅播放视频"模式，解除单帧检测后的冻结，恢复实时推流
    sendDownFrame(kCmdStartVideo);
}

void CameraSerial::onReadyRead()
{
    // 读入上行字节并追加进缓冲（可能粘包/半包，统一缓冲后解析）
    QByteArray bytes = m_serial->readAll();
    // 调试：打印每次收到的上行裸数据，用于确认设备是否真的回报了推理帧
    if (!bytes.isEmpty()) {
        emit logMessage(QStringLiteral("[串口] 上行收到 %1 字节 HEX: %2")
                            .arg(bytes.size())
                            .arg(QString::fromLatin1(bytes.toHex(' ').toUpper())));
    }
    m_rxBuffer.append(bytes);

    while (true) {
        // 缓冲内未凑够一整帧，等待下一次 readyRead 继续拼接
        if (m_rxBuffer.size() < kUpFrameLen)
            break;

        // 查找上行帧头 0x5B；没有则整段丢弃（杂散数据），找到则丢弃其前的字节
        const int headIdx = m_rxBuffer.indexOf(char(kUpFrameHead));
        if (headIdx < 0) {
            m_rxBuffer.clear();
            break;
        }
        if (headIdx > 0)
            m_rxBuffer.remove(0, headIdx);

        // 帧尾不匹配 → 可能是误同步，丢掉首字节重新找帧头
        if (static_cast<quint8>(m_rxBuffer.at(kUpFrameLen - 1)) != kFrameTail) {
            m_rxBuffer.remove(0, 1);
            continue;
        }

        // 合法上行推理帧：[0x5B][CMD_H][CMD_L][类别编码][置信度(0~0x64)][0xA5]
        const quint16 upCmd = (static_cast<quint16>(static_cast<quint8>(m_rxBuffer.at(1))) << 8)
                            | static_cast<quint8>(m_rxBuffer.at(2));
        const quint8 clsCode    = static_cast<quint8>(m_rxBuffer.at(3));
        const quint8 confHex    = static_cast<quint8>(m_rxBuffer.at(4));
        m_rxBuffer.remove(0, kUpFrameLen);

        // 置信度解码：0x00~0x64 → 0.0 ~ 1.0
        const float confidence = qBound(0.0f, confHex / static_cast<float>(kConfMaxHex), 1.0f);

        emit logMessage(QStringLiteral("[串口] 舌苔检测上行帧：CMD=0x%1 类别编码=0x%2 置信度=%3%")
                            .arg(upCmd, 4, 16, QLatin1Char('0'))
                            .arg(clsCode, 2, 16, QLatin1Char('0'))
                            .arg(qRound(confidence * 100)));
        emit tongueDetected(clsCode, confidence);
    }
}

// 下发固定 4 字节下行帧：0x5A | CMD_H | CMD_L | 0xA5（大端序）
void CameraSerial::sendDownFrame(quint16 cmd)
{
    QByteArray frame;
    frame.append(static_cast<char>(kDownFrameHead));
    frame.append(static_cast<char>((cmd >> 8) & 0xFF));
    frame.append(static_cast<char>(cmd & 0xFF));
    frame.append(static_cast<char>(kFrameTail));

    const qint64 written = m_serial->write(frame);
    if (written == frame.size()) {
        emit logMessage(QStringLiteral("[串口] 下发 4 字节帧 0x%1 HEX: %2")
                            .arg(cmd, 4, 16, QLatin1Char('0'))
                            .arg(QString::fromLatin1(frame.toHex(' ').toUpper())));
    } else {
        emit logMessage(QStringLiteral("[串口] 下发 0x%1 失败").arg(cmd, 4, 16, QLatin1Char('0')));
    }
}

// 打开失败且仍运行 → 3 秒后重试（单发定时器，多次调用自动去重）
void CameraSerial::scheduleRetry()
{
    if (!m_running)
        return;
    emit logMessage(QStringLiteral(""));
    m_retryTimer->start();
}
