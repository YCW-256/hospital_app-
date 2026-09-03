#include "cameraserial.h"

#include <QSerialPort>
#include <QTimer>

// 下行帧约定（PC→RV1106），与参考工程 SerialWorker 一致
static constexpr quint8 kDownFrameHead = 0x5A; // 下行帧头
static constexpr quint8 kFrameTail      = 0xA5; // 上下行共用帧尾
static constexpr quint16 kCmdStartVideo = 0x0001; // 自动播放/推视频指令

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
        emit logMessage(QStringLiteral("[串口] 打开 %1 失败：%2").arg(m_portName, m_serial->errorString()));
        scheduleRetry(); // 设备未枚举/端口被占用时稍后重试
        return;
    }

    connect(m_serial, &QSerialPort::readyRead, this, &CameraSerial::onReadyRead);
    emit connected(true);
    emit logMessage(QStringLiteral("[串口] 已连接 %1 @ %2").arg(m_portName).arg(m_baudRate));

    // 连接成功自动下发 0x0001：让摄像头开始推视频（随后 DeviceCamera 经 TCP 收图）
    QByteArray frame;
    frame.append(static_cast<char>(kDownFrameHead));
    frame.append(static_cast<char>((kCmdStartVideo >> 8) & 0xFF));
    frame.append(static_cast<char>(kCmdStartVideo & 0xFF));
    frame.append(static_cast<char>(kFrameTail));

    const qint64 written = m_serial->write(frame);
    if (written == frame.size())
        emit logMessage(QStringLiteral("[串口] 下发 4 字节帧 HEX: %1").arg(
            QString::fromLatin1(frame.toHex(' ').toUpper())));
    else
        emit logMessage(QStringLiteral("[串口] 下发 0x0001 失败"));
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

void CameraSerial::onReadyRead()
{
    // 上行 6 字节推理帧（0x5B…0xA5）本次不解析，直接读走丢弃避免串口缓存阻塞
    m_serial->readAll();
}

// 打开失败且仍运行 → 3 秒后重试（单发定时器，多次调用自动去重）
void CameraSerial::scheduleRetry()
{
    if (!m_running)
        return;
    emit logMessage(QStringLiteral("[串口] 3 秒后自动重试连接…"));
    m_retryTimer->start();
}
