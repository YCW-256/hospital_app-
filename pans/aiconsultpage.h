#ifndef AICONSULTPAGE_H
#define AICONSULTPAGE_H

#include <QImage>
#include <QWidget>

class CameraSerial;
class DeviceCamera;
class QLabel;
class QLineEdit;
class QPushButton;
class QScrollArea;
class QThread;
class QVBoxLayout;

/**
 * @brief 页面4：AI 快速问诊页（QStackedWidget 索引 4，由首页【药费查询】按钮进入）
 *
 * 左右两栏 + 底部通栏布局的聊天问诊界面：
 *  - 左侧：白色圆角视频卡片（顶部【打开/关闭】按钮 + 内部 QLabel 承载画面，
 *    纯代码、不依赖 multimedia 模块）：初始化即连接舌苔检测摄像头（RV1106），
 *    【打开】后 QLabel 铺满显示硬件上传的图像，未打开 / 未收到帧时显示纯黑。
 *    QLabel 下方单行 4 个常见症状快捷按钮（与右侧【返回首页】同行）。
 *  - 右侧：白色圆角聊天窗口（深蓝标题栏：医生头像 + "AI 快速问诊" + 在线状态
 *          + 可滚动的 ChatBubble 对话气泡区）+ 蓝色【返回首页】按钮。
 *    顶部用透明占位与左侧【症状输入】标题等高，使视频区域与聊天框高度对齐。
 *  - 底部通栏：蓝色圆形语音麦克风按钮（按住说话，松手自动识别，
 *    识别文字直接发给 AI 医生）+ 症状描述输入框（占位"请描述症状，不超过 300 字"）
 *    + 蓝色【开始问诊】按钮，回车或点击发送。
 *
 * 问答走独立的 AI 快速问诊智能体（ConsultAgent，医疗问诊提示词 + 独立对话记忆，
 * 与个人信息助手 MyAgent 分离），AI 的欢迎语与每条回复自动语音播报
 * （TtsPlayer，Windows SAPI）。
 */
class AIConsultPage : public QWidget
{
    Q_OBJECT

public:
    explicit AIConsultPage(QWidget *parent = nullptr);
    ~AIConsultPage() override;

signals:
    void backRequested();                        // 【返回首页】→ 首页（索引 0）
    void consultRequested(const QString &symptom); // 每次问诊携带用户症状文本

private:
    void initLayout();  // 构建页面布局
    QWidget *createLeftArea();  // 左侧：视频区域 + 症状快捷按钮
    QWidget *createRightArea(); // 右侧：聊天窗口 + 返回首页
    QWidget *createBottomBar(); // 底部通栏：输入框 + 开始问诊

    void addMessage(const QString &text, bool isUser); // 追加一条气泡
    void sendToAgent(const QString &userText);         // 用户消息 → ConsultAgent → TTS
    void submitText(const QString &text);              // 统一发送入口（打字回车 / 语音识别共用）
    void onSend();                                     // 发送输入框内容（回车/按钮）
    void onVoicePressed();                             // 按住麦克风：开始录音
    void onVoiceReleased();                            // 松开麦克风：自动识别并直接发送
    void onVoiceError(const QString &message);         // 语音识别错误（气泡提示）

    void initCamera();          // 初始化硬件摄像头（启动即连接设备）
    void refreshVideoLabel();   // 按开关状态刷新视频标签（画面铺满 / 纯黑）

private slots:
    void onCameraFrame(const QImage &frame); // 收到硬件上传图像帧
    void onToggleCamera(bool checked);       // 【打开/关闭】按钮切换
    void onDetectTongue();                   // 【舌苔检测】点击：跨线程下发 0x0010 触发单帧检测
    void onResumeVideo();                    // 【再次检测】点击：跨线程下发 0x0001 恢复实时推流
    void onTongueDetected(int classId, float confidence); // 收到舌苔上行帧：qDebug 打印结果

private: // 成员
    QLabel       *m_videoLabel  = nullptr; // 视频承载 QLabel（承载硬件图像 / 纯黑）
    QPushButton  *m_toggleBtn   = nullptr; // 摄像头【打开/关闭】按钮
    QPushButton  *m_detectBtn   = nullptr; // 【舌苔检测】按钮（下发 0x0010 触发单帧检测）
    QPushButton  *m_resumeBtn   = nullptr; // 【再次检测】按钮（下发 0x0001 恢复实时推流）
    QPushButton  *m_voiceBtn    = nullptr; // 语音输入麦克风（按住说话，松手自动识别发送）
    QLineEdit    *m_inputEdit   = nullptr; // 症状描述输入框
    bool          m_voiceBound  = false;   // 是否已绑定语音识别单例（首次按住时懒绑定）
    QScrollArea  *m_scroll      = nullptr; // 对话气泡滚动区
    QWidget      *m_chatContent = nullptr; // 气泡内容区
    QVBoxLayout  *m_chatLayout  = nullptr; // 气泡垂直布局（末尾 stretch）
    QString       m_doctorAvatar;          // 医生头像（AI 侧）
    QString       m_userAvatar;            // 用户头像（用户侧）

    DeviceCamera *m_camera       = nullptr; // 舌苔摄像头 TCP 接收器（独立线程）
    QThread      *m_cameraThread = nullptr; // 摄像头网络线程（不阻塞 UI）
    CameraSerial *m_serialCtl    = nullptr; // 舌苔摄像头串口控制器（独立线程）
    QThread      *m_serialThread = nullptr; // 摄像头串口线程（不阻塞 UI）
    QImage        m_latestFrame;            // 最近收到的一帧（未打开时也持续缓存）
    bool          m_cameraOn     = false;   // 是否显示摄像头画面（默认关闭 = 纯黑）
};

#endif // AICONSULTPAGE_H
