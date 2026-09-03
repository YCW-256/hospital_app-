#include "chatbubble.h"
#include "circularavatar.h"

#include <QFontMetrics>
#include <QHBoxLayout>
#include <QTextEdit>
#include <QTextOption>

namespace {
const int kAvatarSize     = 36;       // 头像尺寸
const int kMaxBubbleWidth = 340;      // 气泡宽度封顶（超过则换行）
const int kDocMargin      = 10;       // 文档内边距（文字距气泡边缘）
const int kWidthExtra     = 26;       // 宽度余量（内边距 + 边框 + 缓冲）
const int kHeightExtra    = 8;        // 高度余量（边框 + 缓冲）
} // namespace

ChatBubble::ChatBubble(const QString &text, bool isUser, const QString &avatarPath, QWidget *parent)
    : QWidget(parent)
{
    /* ---------- 圆形头像 ---------- */
    m_avatar = new CircularAvatar(this);
    m_avatar->setFixedSize(kAvatarSize, kAvatarSize);
    m_avatar->setAvatar(avatarPath);

    /* ---------- 文字气泡（只读，自动换行） ---------- */
    m_textEdit = new QTextEdit(this);
    m_textEdit->setReadOnly(true);
    m_textEdit->setWordWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
    m_textEdit->setLineWrapMode(QTextEdit::WidgetWidth);
    m_textEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_textEdit->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_textEdit->document()->setDocumentMargin(kDocMargin);
    m_textEdit->setText(text);

    // 气泡样式（与终端整体配色一致）
    m_textEdit->setStyleSheet(isUser
        ? QStringLiteral(
            "QTextEdit {"
            "    color: #FFFFFF;"
            "    background-color: #2E86DE;"
            "    border: none;"
            "    border-radius: 12px;"
            "    font-size: 15px;"
            "}")
        : QStringLiteral(
            "QTextEdit {"
            "    color: #1F4E79;"
            "    background-color: #FFFFFF;"
            "    border: 1px solid #D5DEEA;"
            "    border-radius: 12px;"
            "    font-size: 15px;"
            "}"));

    // 关键：让 QTextEdit 视口透明，否则白色矩形视口会盖住圆角背景
    m_textEdit->viewport()->setAutoFillBackground(false);

    /* ---------- 自适应尺寸（复刻原 ChatBom 算法） ----------
     * 气泡宽度 = 文本单行宽度 + 余量，超出封顶宽度则换行；
     * 高度由文档在固定宽度下的实际换行高度决定。 */
    const QFontMetrics fm(m_textEdit->font());
    const int textWidth = fm.horizontalAdvance(text);
    const int targetWidth = qMin(textWidth + kWidthExtra, kMaxBubbleWidth);
    m_textEdit->setFixedWidth(targetWidth);
    m_textEdit->document()->setTextWidth(targetWidth); // 强制按该宽度排版

    const int idealHeight = int(m_textEdit->document()->size().height()) + kHeightExtra;
    m_textEdit->setFixedHeight(idealHeight);

    /* ---------- 布局：AI [头像][气泡][弹簧] / 用户 [弹簧][气泡][头像] ---------- */
    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(2, 2, 2, 2);
    layout->setSpacing(8);
    if (isUser) {
        layout->addStretch(1);
        layout->addWidget(m_textEdit);
        layout->addWidget(m_avatar, 0, Qt::AlignTop);
    } else {
        layout->addWidget(m_avatar, 0, Qt::AlignTop);
        layout->addWidget(m_textEdit);
        layout->addStretch(1);
    }
}
