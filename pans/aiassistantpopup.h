#ifndef AIASSISTANTPOPUP_H
#define AIASSISTANTPOPUP_H

#include <QPoint>
#include <QWidget>

class QLabel;
class QLineEdit;
class QMouseEvent;
class QPushButton;
class QScrollArea;
class QVBoxLayout;

/**
 * @brief 悬浮 AI 信息助手弹窗
 *
 * 无边框、置顶（WindowStaysOnTopHint），显示在主窗口卡片上层。
 * 结构：标题栏（医生圆形头像 + "AI信息助手" + 关闭）
 *      + 对话气泡区（ChatBubble）
 *      + 三个快捷按钮（查看认证状态 / 修改手机号 / 修改医保信息）
 *      + 语音输入区（输入框 + 蓝色圆形语音按钮【按住说话】 + 发送按钮）
 *
 * 快捷按钮走本地应答；输入框内容调用 DeepSeek 智能体（MyAgent）。
 * 语音输入：按住麦克风说话，松开自动识别，识别出的文字直接发给 MyAgent，
 * 无需再点发送按钮。
 */
class AIAssistantPopup : public QWidget
{
    Q_OBJECT

public:
    /**
     * @param doctorAvatarPath 医生头像路径（标题栏与 AI 气泡使用）
     * @param parent           父窗口（通常为主窗口）
     */
    explicit AIAssistantPopup(const QString &doctorAvatarPath, QWidget *parent = nullptr);
    ~AIAssistantPopup() override;

protected:
    void mousePressEvent(QMouseEvent *event) override;   // 按住标题栏拖动
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    void initUi();                                          // 构建界面
    void addMessage(const QString &text, bool isUser);      // 追加一条气泡
    void replyTo(const QString &userText, const QString &aiText); // 用户 + AI 两条气泡
    void onQuickAction(const QString &action);              // 快捷按钮点击
    void onSend();                                          // 发送输入框内容
    void submitText(const QString &text);                   // 统一发送入口（打字 / 语音共用）
    void onVoicePressed();                                  // 按住麦克风：开始录音
    void onVoiceReleased();                                 // 松开麦克风：自动识别并直接发送

    QString m_avatarPath;   // 医生头像（AI 侧）
    QString m_userAvatar;   // 用户头像

    QWidget     *m_titleBar   = nullptr;
    QVBoxLayout *m_chatLayout = nullptr;
    QScrollArea *m_scroll     = nullptr;
    QWidget     *m_chatContent = nullptr;
    QLineEdit   *m_inputEdit  = nullptr;
    QPushButton *m_voiceBtn   = nullptr; // 语音麦克风（按住说话，松手自动识别发送）
    bool         m_voiceBound = false;   // 是否已绑定语音识别单例（首次按住时懒绑定）
    QPoint       m_dragOffset;
    bool         m_dragging   = false;
signals:
    void full_text(int decison,QString content); // 发送AI决策码与内容
};

#endif // AIASSISTANTPOPUP_H
