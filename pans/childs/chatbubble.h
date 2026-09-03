#ifndef CHATBUBBLE_H
#define CHATBUBBLE_H

#include <QWidget>

class CircularAvatar;
class QTextEdit;

/**
 * @brief 聊天气泡（纯代码，自适应宽度）
 *
 * 复刻 chat 工程 ChatBom 的自适应算法：气泡宽度随文本内容伸缩——
 * 短文本窄气泡、长文本按封顶宽度自动换行；高度随文本行数自适应。
 * AI 侧白底深字靠左，用户侧蓝底白字靠右。
 * 本项目用纯代码重写（原 ChatBom 依赖 .ui/.qrc，不符合本工程约束）。
 */
class ChatBubble : public QWidget
{
    Q_OBJECT

public:
    /**
     * @param text        消息文本
     * @param isUser      true=用户消息（蓝底靠右），false=AI 消息（白底靠左）
     * @param avatarPath  头像图片路径
     * @param parent      父控件
     */
    explicit ChatBubble(const QString &text, bool isUser, const QString &avatarPath,
                        QWidget *parent = nullptr);

private:
    CircularAvatar *m_avatar   = nullptr; // 圆形头像
    QTextEdit      *m_textEdit = nullptr; // 自适应文字气泡
};

#endif // CHATBUBBLE_H
