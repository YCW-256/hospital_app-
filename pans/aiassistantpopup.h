#ifndef AIASSISTANTPOPUP_H
#define AIASSISTANTPOPUP_H

#include <QPoint>
#include <QWidget>

class QLabel;
class QLineEdit;
class QMouseEvent;
class QScrollArea;
class QVBoxLayout;

/**
 * @brief 悬浮 AI 信息助手弹窗
 *
 * 无边框、置顶（WindowStaysOnTopHint），显示在主窗口卡片上层。
 * 结构：标题栏（医生圆形头像 + "AI信息助手" + 关闭）
 *      + 对话气泡区（ChatBubble）
 *      + 三个快捷按钮（查看认证状态 / 修改手机号 / 修改医保信息）
 *      + 语音输入区（输入框占位【按住说话】 + 蓝色圆形搜索按钮）
 *
 * 快捷按钮走本地应答；输入框内容调用 DeepSeek 智能体（MyAgent）。
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



    QString m_avatarPath;   // 医生头像（AI 侧）
    QString m_userAvatar;   // 用户头像

    QWidget     *m_titleBar   = nullptr;
    QVBoxLayout *m_chatLayout = nullptr;
    QScrollArea *m_scroll     = nullptr;
    QWidget     *m_chatContent = nullptr;
    QLineEdit   *m_inputEdit  = nullptr;
    QPoint       m_dragOffset;
    bool         m_dragging   = false;
signals:
    void full_text(int decison,QString content); // 发送AI决策码与内容
};

#endif // AIASSISTANTPOPUP_H
