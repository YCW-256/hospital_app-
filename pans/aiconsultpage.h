#ifndef AICONSULTPAGE_H
#define AICONSULTPAGE_H

#include <QWidget>

class QLabel;
class QLineEdit;
class QScrollArea;
class QVBoxLayout;

/**
 * @brief 页面4：AI 快速问诊页（QStackedWidget 索引 4，由首页【药费查询】按钮进入）
 *
 * 左右两栏 + 底部通栏布局的聊天问诊界面：
 *  - 左侧：白色圆角视频区域（内部 QLabel 承载内容，纯代码、不依赖
 *    multimedia 模块，默认显示医生占位图）+ 下方单行 4 个常见症状
 *    快捷按钮（与右侧【返回首页】同行）。
 *  - 右侧：白色圆角聊天窗口（深蓝标题栏：医生头像 + "AI 快速问诊" + 在线状态
 *          + 可滚动的 ChatBubble 对话气泡区）+ 蓝色【返回首页】按钮。
 *    顶部用透明占位与左侧【症状输入】标题等高，使视频区域与聊天框高度对齐。
 *  - 底部通栏：带麦克风图标的症状描述输入框（占位"请描述症状，不超过 300 字"）
 *          + 蓝色【开始问诊】按钮，回车或点击发送。
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
    void onSend();                                     // 发送输入框内容（回车/按钮）

    QLabel       *m_videoLabel  = nullptr; // 视频承载 QLabel（QLabel 承载内容，后续可换图/换动画）
    QLineEdit    *m_inputEdit   = nullptr; // 症状描述输入框
    QScrollArea  *m_scroll      = nullptr; // 对话气泡滚动区
    QWidget      *m_chatContent = nullptr; // 气泡内容区
    QVBoxLayout  *m_chatLayout  = nullptr; // 气泡垂直布局（末尾 stretch）
    QString       m_doctorAvatar;          // 医生头像（AI 侧）
    QString       m_userAvatar;            // 用户头像（用户侧）
};

#endif // AICONSULTPAGE_H
