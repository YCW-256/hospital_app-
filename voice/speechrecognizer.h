#ifndef SPEECHRECOGNIZER_H
#define SPEECHRECOGNIZER_H

#include <QObject>
#include <QString>

#include <vector>

class QThread;
class AudioRecorder;
class SenseVoiceEngine;

/**
 * @brief 语音识别控制器（voice/ 语音识别模块，全局单例）
 *
 * 把"按住说话 → 录音 → 松手 → 自动识别 → 回文字"的完整流程封装成一个
 * 接口，供 AI 聊天界面（AI 快速问诊页 / AI 信息助手弹窗）直接调用：
 *
 *   按住麦克风： startListening(this)  → 开始录音
 *   松手：        stopListening()      → 停止录音，送工作线程识别
 *   结果：        recognized(requester, text)  → 界面收到文字后直接发给 AI 智能体
 *
 * 线程模型：
 *  - AudioRecorder 在主线程（麦克风采集轻量）；
 *  - SenseVoiceEngine 移入独立工作线程（ONNX 推理较耗时，避免卡 UI）。
 *
 * 使用方路由：由于两个聊天界面可能都持有单例连接，recognized/errorOccurred
 * 信号均携带发起者指针 requester，界面侧只在 requester == 自身时才处理，
 * 从而保证只有当前"按下麦克风"的那个聊天框收到识别文字并自动发送。
 */
class SpeechRecognizer : public QObject
{
    Q_OBJECT

public:
    /** 全局单例（懒创建：首次按下麦克风时才建立） */
    static SpeechRecognizer *instance();

    /** 解析 SenseVoice 模型目录：优先编译期宏 voice/model，其次运行期相对路径 */
    static QString modelDir();

    /** 开始录音（界面的麦克风按钮被按住时调用） */
    void startListening(QObject *requester);

    /** 停止录音并自动送识别（界面按钮松开时调用） */
    void stopListening();

    /** 界面销毁时取消：若它正在录音/识别则中断并释放麦克风 */
    void cancelFor(QObject *requester);

    bool isRecording() const { return m_recording; } // 正在录音
    bool isBusy() const { return m_busy; }           // 正在识别（占用中）

signals:
    /** 录音状态变化（用于按钮样式：true=正在录音） */
    void recordingChanged(bool recording);

    /** 识别忙碌状态变化（用于禁点等 UI 反馈） */
    void busyChanged(bool busy);

    /** 识别出非空文字（requester 与发起 startListening 的对象一致才处理） */
    void recognized(QObject *requester, const QString &text);

    /** 出错（模型缺失 / 无麦克风 / 未录到声音等） */
    void errorOccurred(QObject *requester, const QString &message);

private:
    explicit SpeechRecognizer(QObject *parent = nullptr);

private slots:
    void onModelLoaded(bool ok);                             // 引擎线程模型加载结束
    void onEngineError(const QString &errMsg);               // 引擎报错（识别期转发给界面）
    void onEngineResult(const QString &text, qint64 elapsedMs); // 识别完成（工作线程→主线程）

signals:
    void recognizeRequested(std::vector<float> data); // 主线程发往引擎线程的识别请求（仅内部 emit）

private:
    AudioRecorder     *m_recorder = nullptr; // 录音器（主线程）
    SenseVoiceEngine  *m_engine   = nullptr; // 识别引擎（工作线程）
    QThread           *m_thread   = nullptr; // 引擎工作线程

    QObject *m_requester   = nullptr; // 当前发起者（录音→识别期间保持，用于结果路由）
    bool     m_recording   = false;   // 录音中
    bool     m_busy        = false;   // 识别中
    bool     m_modelReady  = false;   // 模型已加载
    bool     m_modelLoading = false;  // 模型加载已排队/进行中（避免重复排队）

    static SpeechRecognizer *m_instance;
};

#endif // SPEECHRECOGNIZER_H
