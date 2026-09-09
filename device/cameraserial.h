#ifndef CAMERASERIAL_H
#define CAMERASERIAL_H

#include <QByteArray>
#include <QObject>
#include <QString>

class QSerialPort;
class QTimer;

/**
 * @brief 舌苔检测摄像头（RV1106）串口控制器
 *
 * 移植自参考工程 rv1106_test01 的 SerialWorker 下行部分。PC→RV1106
 * 固定 4 字节帧：0x5A(帧头) | CMD_H | CMD_L | 0xA5(帧尾)，大端序。
 * 连接成功后自动下发 0x0001（自动播放/推视频），摄像头随即经 TCP 向
 * DeviceCamera 推送图像帧；另提供 triggerTongueDetect() 下发 0x0010，
 * 让设备进入"单帧推理"模式，一旦在当前画面检测到舌苔即上行回报。
 *
 * 上行 6 字节推理帧：0x5B(帧头) | CMD_H | CMD_L | 类别编码(0x00~0x04
 * /0xFF 未识别) | 置信度(0x00~0x64，对应 0~100%) | 0xA5(帧尾)，
 * onReadyRead() 用缓冲+状态机解析（兼容粘包/半包/杂散字节），成功后发
 * tongueDetected(int,float) 信号，由界面层映射舌苔名称并输出。
 *
 * 设计为 moveToThread 到独立线程运行，start()/stop()/triggerTongueDetect()
 * 以 public slot 跨线程调用；打开失败时（设备未枚举/串口被占用）每 3 秒
 * 自动重试，stop() 后不再重试。
 */
class CameraSerial : public QObject
{
    Q_OBJECT

public:
    explicit CameraSerial(const QString &portName, qint32 baudRate, QObject *parent = nullptr);
    ~CameraSerial() override;

public slots:
    void start();             // 打开串口，成功自动下发 0x0001 开始推视频
    void stop();              // 关闭串口
    void triggerTongueDetect(); // 下发 0x0010 触发一次单帧舌苔检测（检测到即上行回报并冻结画面）
    void resumeVideo();       // 下发 0x0001：解除检测后冻结，恢复实时推流（再次检测/重新预览）

signals:
    void connected(bool ok);                 // 串口连接成功/断开
    void logMessage(const QString &msg);     // 日志（界面可接 qDebug 输出）
    void tongueDetected(int classId, float confidence); // 解析到上行舌苔检测结果

private slots:
    void onReadyRead(); // 读入上行数据，缓冲 + 状态机解析推理帧

private:
    void sendDownFrame(quint16 cmd); // 下发固定 4 字节下行帧
    void scheduleRetry();            // 打开失败时安排 3 秒后重试

    QSerialPort *m_serial;
    QTimer *m_retryTimer;
    QString m_portName;
    qint32 m_baudRate;
    bool m_running;       // true 时打开失败自动重试
    QByteArray m_rxBuffer; // 上行接收缓冲（跨 readyRead 拼接，防半包）
};

#endif // CAMERASERIAL_H
