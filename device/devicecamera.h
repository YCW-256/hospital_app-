#ifndef DEVICECAMERA_H
#define DEVICECAMERA_H

#include <QByteArray>
#include <QImage>
#include <QObject>
#include <QString>
#include <QTcpSocket>

class QTimer;

/**
 * @brief 舌苔检测摄像头（RV1106）TCP 视频流接收器
 *
 * 移植自参考工程 rv1106_test01 上位机测试代码的 VideoReceiver，
 * 负责接收硬件上传的图像帧并解析为 QImage。帧协议（大端序）：
 *
 *     帧魔数 "LZDZ"(4B)
 *     | 图像数据长度 dataLen(4B)
 *     | 行数 rows(4B)
 *     | 列数 cols(4B)
 *     | 原始像素 dataLen 字节
 *
 * 像素判定：dataLen == rows*cols*3 → BGR888（按 RGB888 读入后红蓝交换还原）；
 *          dataLen == rows*cols*1 → 灰度图（Grayscale8）。
 *
 * 发出 frameReady() 的图像已 .copy() 深拷贝为独立内存，跨线程安全。
 * 本类设计为 moveToThread 到独立线程运行，start()/stop() 以 public slot
 * 供跨线程调用；运行期间(m_running)断开或 connectToHost 失败后每 2 秒自动重连
 * （单发定时器去重），手动 stop() 后不再重连。
 */
class DeviceCamera : public QObject
{
    Q_OBJECT

public:
    explicit DeviceCamera(const QString &host, quint16 port, QObject *parent = nullptr);
    ~DeviceCamera() override;

public slots:
    void start(); // 发起连接（重置接收状态后 connectToHost）
    void stop();  // 断开连接并关闭自动重连

signals:
    void connected(bool ok);               // 连接成功 / 断开
    void frameReady(const QImage &frame);  // 解析出一帧完整图像
    void logMessage(const QString &msg);   // 日志（界面可接 qDebug 输出）

private slots:
    void onConnected();
    void onDisconnected();
    void onReadyRead();
    void onSocketError(QAbstractSocket::SocketError socketError);

private:
    bool readAllBytes(QByteArray &dst, qint64 size); // 从接收缓存取出足量字节
    void parseFrame();                              // 状态机解析完整视频帧
    void scheduleReconnect();                       // 单发定时器自动重连（去重）

    QTcpSocket *m_socket;
    QTimer *m_reconnectTimer;       // 自动重连单发定时器（断开/连接失败都触发）
    QString m_host;
    quint16 m_port;
    bool m_running;                 // true 时断开/连接失败自动重连
    QByteArray m_rxBuffer;          // 接收缓存

    enum State { WaitingMagic, WaitingLen, WaitingRows, WaitingCols, WaitingData };
    State m_state;                  // 当前解析状态
    quint32 m_dataLen;              // 当前帧图像数据长度
    quint32 m_rows;                 // 当前帧行数
    quint32 m_cols;                 // 当前帧列数
};

#endif // DEVICECAMERA_H
