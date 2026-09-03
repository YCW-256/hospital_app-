#ifndef CAMERASERIAL_H
#define CAMERASERIAL_H

#include <QObject>
#include <QString>

class QSerialPort;
class QTimer;

/**
 * @brief 舌苔检测摄像头（RV1106）串口控制器
 *
 * 移植自参考工程 rv1106_test01 的 SerialWorker 下行部分。PC→RV1106
 * 固定 4 字节帧：0x5A(帧头) | CMD_H | CMD_L | 0xA5(帧尾)，大端序。
 * 本类只做"打开串口 + 成功即自动下发 0x0001（自动播放/推视频）"，
 * 摄像头收到后即开始经 TCP 向 DeviceCamera 推送图像帧；
 * 上行 6 字节推理帧（0x5B…0xA5）本工程暂不解析，读入后直接丢弃以免阻塞。
 *
 * 设计为 moveToThread 到独立线程运行，start()/stop() 以 public slot
 * 跨线程调用；打开失败时（设备未枚举/串口被占用）每 3 秒自动重试，stop() 后不再重试。
 */
class CameraSerial : public QObject
{
    Q_OBJECT

public:
    explicit CameraSerial(const QString &portName, qint32 baudRate, QObject *parent = nullptr);
    ~CameraSerial() override;

public slots:
    void start(); // 打开串口，成功自动下发 0x0001 开始推视频
    void stop();  // 关闭串口

signals:
    void connected(bool ok);             // 串口连接成功/断开
    void logMessage(const QString &msg); // 日志（界面可接 qDebug 输出）

private slots:
    void onReadyRead(); // 读入上行数据，暂不解析直接丢弃

private:
    void scheduleRetry(); // 打开失败时安排 3 秒后重试

    QSerialPort *m_serial;
    QTimer *m_retryTimer;
    QString m_portName;
    qint32 m_baudRate;
    bool m_running; // true 时打开失败自动重试
};

#endif // CAMERASERIAL_H
