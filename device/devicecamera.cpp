#include "devicecamera.h"

#include <QDataStream>
#include <QDebug>
#include <QTimer>

// 视频帧头魔数（RV1106 硬件固定）
static const QByteArray kFrameMagic = QByteArrayLiteral("LZDZ");
// 单帧图像上限 5MB，防止脏数据/超大包拖垮内存
static constexpr qint64 kMaxImageBytes = 5 * 1024 * 1024;

DeviceCamera::DeviceCamera(const QString &host, quint16 port, QObject *parent)
    : QObject(parent)
    , m_socket(new QTcpSocket(this))
    , m_reconnectTimer(new QTimer(this))
    , m_host(host)
    , m_port(port)
    , m_running(false)
    , m_state(WaitingMagic)
    , m_dataLen(0)
    , m_rows(0)
    , m_cols(0)
{
    // 绑定 socket 信号（UniqueConnection 防止重复绑定）
    connect(m_socket, &QTcpSocket::connected, this, &DeviceCamera::onConnected, Qt::UniqueConnection);
    connect(m_socket, &QTcpSocket::disconnected, this, &DeviceCamera::onDisconnected, Qt::UniqueConnection);
    connect(m_socket, &QTcpSocket::readyRead, this, &DeviceCamera::onReadyRead, Qt::UniqueConnection);
    connect(m_socket, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::errorOccurred),
            this, &DeviceCamera::onSocketError, Qt::UniqueConnection);

    // 自动重连单发定时器：断开或连接失败都只重启这一个定时器，避免重复触发
    m_reconnectTimer->setSingleShot(true);
    m_reconnectTimer->setInterval(2000);
    connect(m_reconnectTimer, &QTimer::timeout, this, &DeviceCamera::start);
}

DeviceCamera::~DeviceCamera()
{
    stop();
}

void DeviceCamera::start()
{
    m_running = true;
    m_reconnectTimer->stop(); // 已开始连接，取消可能挂起的重连定时
    // 重置解析状态机与接收缓存，防止上次残留脏数据串帧
    m_state = WaitingMagic;
    m_rxBuffer.clear();
    emit logMessage(QStringLiteral("[摄像头] 正在连接视频服务器 %1:%2").arg(m_host).arg(m_port));
    m_socket->connectToHost(m_host, m_port);
}

void DeviceCamera::stop()
{
    m_running = false;         // 关闭自动重连开关
    m_reconnectTimer->stop();  // 取消待执行的重连定时
    m_state = WaitingMagic;    // 重置解析状态
    m_rxBuffer.clear();        // 清空缓存脏数据

    if (m_socket->state() != QAbstractSocket::UnconnectedState)
        m_socket->disconnectFromHost();
    m_socket->close();
    emit connected(false);
    emit logMessage(QStringLiteral("[摄像头] 视频接收已停止"));
}

// 断开或连接失败后统一安排自动重连（单发定时器，多次调用自动去重）
void DeviceCamera::scheduleReconnect()
{
    // 仅运行状态自动重连；手动 stop() 后 m_running=false 不再进入
    if (!m_running)
        return;
    emit logMessage(QStringLiteral("[摄像头] 2 秒后自动重连…"));
    m_reconnectTimer->start();
}

void DeviceCamera::onConnected()
{
    emit connected(true);
    emit logMessage(QStringLiteral("[摄像头] 视频服务器连接成功"));
}

void DeviceCamera::onDisconnected()
{
    emit connected(false);
    emit logMessage(QStringLiteral("[摄像头] 视频连接断开"));
    scheduleReconnect(); // 仍运行 → 2 秒后自动重连
}

void DeviceCamera::onReadyRead()
{
    m_rxBuffer.append(m_socket->readAll());
    parseFrame();
}

void DeviceCamera::onSocketError(QAbstractSocket::SocketError socketError)
{
    Q_UNUSED(socketError);
    emit logMessage(QStringLiteral("[摄像头] 网络错误：%1").arg(m_socket->errorString()));
    emit connected(false);
    // connectToHost 失败（如开机时设备尚未就绪）通常只发 errorOccurred 而不发
    // disconnected，这里也必须安排重连，否则画面会永久停留在纯黑
    scheduleReconnect();
}

bool DeviceCamera::readAllBytes(QByteArray &dst, qint64 size)
{
    if (m_rxBuffer.size() < size)
        return false;
    dst = m_rxBuffer.left(size);
    m_rxBuffer.remove(0, size);
    return true;
}

// 状态机解析完整视频帧
void DeviceCamera::parseFrame()
{
    while (true) {
        switch (m_state) {
        case WaitingMagic: {
            // 在缓存中查找帧魔数，未找到说明数据不完整
            const int magicIdx = m_rxBuffer.indexOf(kFrameMagic);
            if (magicIdx == -1) {
                // 保留末尾 3 字节，防止魔数被分包截断丢失
                if (m_rxBuffer.size() > kFrameMagic.size() - 1)
                    m_rxBuffer = m_rxBuffer.right(kFrameMagic.size() - 1);
                return;
            }
            // 丢弃魔数之前的无用垃圾数据
            if (magicIdx > 0)
                m_rxBuffer.remove(0, magicIdx);
            // 移除 4 字节魔数
            m_rxBuffer.remove(0, kFrameMagic.size());
            m_state = WaitingLen;
            break;
        }
        case WaitingLen: {
            QByteArray lenBuf;
            if (!readAllBytes(lenBuf, 4))
                return;
            QDataStream ds(lenBuf);
            ds.setByteOrder(QDataStream::BigEndian);
            ds >> m_dataLen;

            // 校验图像长度，超大包/空包直接丢弃防止内存溢出
            if (m_dataLen == 0 || m_dataLen > kMaxImageBytes) {
                emit logMessage(QStringLiteral("[摄像头] 图像长度非法，丢弃该数据包"));
                m_state = WaitingMagic;
                m_rxBuffer.clear();
                break;
            }
            m_state = WaitingRows;
            break;
        }
        case WaitingRows: {
            QByteArray rowBuf;
            if (!readAllBytes(rowBuf, 4))
                return;
            QDataStream ds(rowBuf);
            ds.setByteOrder(QDataStream::BigEndian);
            ds >> m_rows;
            m_state = WaitingCols;
            break;
        }
        case WaitingCols: {
            QByteArray colBuf;
            if (!readAllBytes(colBuf, 4))
                return;
            QDataStream ds(colBuf);
            ds.setByteOrder(QDataStream::BigEndian);
            ds >> m_cols;
            m_state = WaitingData;
            break;
        }
        case WaitingData: {
            // 图像数据未收齐，等待下一次 readyRead
            if (m_rxBuffer.size() < (qint64)m_dataLen)
                return;

            QByteArray imgData = m_rxBuffer.left((int)m_dataLen);
            m_rxBuffer.remove(0, (int)m_dataLen);
            m_state = WaitingMagic; // 回到等待下一帧魔数

            const quint32 pixelCount = m_rows * m_cols;
            QImage img;
            if (m_dataLen == pixelCount * 3) {
                // 硬件上传为 BGR888：先按 RGB888 读入，再交换红蓝还原颜色
                QImage bgr(reinterpret_cast<const uchar *>(imgData.constData()),
                           m_cols, m_rows, QImage::Format_RGB888);
                img = bgr.rgbSwapped().copy();
            } else if (m_dataLen == pixelCount * 1) {
                // 灰度图（单通道），拷贝为独立内存
                img = QImage(reinterpret_cast<const uchar *>(imgData.constData()),
                             m_cols, m_rows, QImage::Format_Grayscale8).copy();
            } else {
                emit logMessage(QStringLiteral("[摄像头] 不支持的图像数据长度，丢弃当前帧"));
                break;
            }

            if (!img.isNull())
                emit frameReady(img); // 已深拷贝，跨线程安全
            break;
        }
        }
    }
}
