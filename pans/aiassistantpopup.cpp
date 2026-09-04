#include "aiassistantpopup.h"
#include "../agent/myagent.h"
#include "../audio/ttsplayer.h"
#include "../core/iconfactory.h"
#include "../core/uistyle.h"
#include "../voice/speechrecognizer.h"
#include "childs/chatbubble.h"
#include "childs/circularavatar.h"

#include <QApplication>
#include <QFrame>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QLineEdit>
#include <QMouseEvent>
#include <QPushButton>
#include <QScrollArea>
#include <QScrollBar>
#include <QStringList>
#include <QTimer>
#include <QVBoxLayout>

AIAssistantPopup::AIAssistantPopup(const QString &doctorAvatarPath, QWidget *parent)
    : QWidget(parent)
    , m_avatarPath(doctorAvatarPath)
    , m_userAvatar(UIStyle::resolveImagePath(QStringLiteral("icons/user.png")))
{
    // 置顶、无边框、不在任务栏显示
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
    setAttribute(Qt::WA_DeleteOnClose);
    setFixedSize(460, 620);
    setObjectName(QStringLiteral("aiPopup"));
    setAttribute(Qt::WA_TranslucentBackground, true); // 透明窗口底，实现圆角

    initUi();
}

AIAssistantPopup::~AIAssistantPopup()
{
    // 语音识别若正在进行则取消（释放麦克风 / 丢弃识别结果）
    if (m_voiceBound)
        SpeechRecognizer::instance()->cancelFor(this);
}

void AIAssistantPopup::initUi()
{
    // ---------- 外层白色圆角容器 ----------
    auto *container = new QWidget(this);
    container->setObjectName(QStringLiteral("aiContainer"));
    container->setAttribute(Qt::WA_StyledBackground, true);
    container->setStyleSheet(QStringLiteral(
        "#aiContainer {"
        "    background-color: #FFFFFF;"
        "    border: 2px solid #D5DEEA;"
        "    border-radius: 18px;"
        "}"));
    auto *outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);
    outer->addWidget(container);

    auto *root = new QVBoxLayout(container);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    /* ---------- 标题栏：医生头像 + AI信息助手 + 关闭 ---------- */
    m_titleBar = new QWidget(container);
    m_titleBar->setFixedHeight(56);
    m_titleBar->setStyleSheet(QStringLiteral(
        "background-color: #1F4E79;"
        "border-top-left-radius: 16px;"
        "border-top-right-radius: 16px;"));
    auto *titleLayout = new QHBoxLayout(m_titleBar);
    titleLayout->setContentsMargins(16, 8, 8, 8);
    titleLayout->setSpacing(10);

    auto *avatar = new CircularAvatar(m_titleBar);
    avatar->setFixedSize(34, 34);
    avatar->setAvatar(m_avatarPath);
    titleLayout->addWidget(avatar);

    auto *titleLabel = new QLabel(QStringLiteral("AI信息助手"), m_titleBar);
    titleLabel->setStyleSheet(QStringLiteral(
        "color: #FFFFFF; font-size: 18px; font-weight: bold; background: transparent;"));
    titleLayout->addWidget(titleLabel);
    titleLayout->addStretch(1);

    auto *closeBtn = new QPushButton(QStringLiteral("×"), m_titleBar);
    closeBtn->setCursor(Qt::PointingHandCursor);
    closeBtn->setFixedSize(32, 32);
    closeBtn->setStyleSheet(QStringLiteral(
        "QPushButton { color: #9FB3C8; background: transparent; border: none;"
        "               border-radius: 16px; font-size: 20px; }"
        "QPushButton:hover { background-color: #E81123; color: #FFFFFF; }"));
    connect(closeBtn, &QPushButton::clicked, this, &QWidget::close);
    titleLayout->addWidget(closeBtn);
    root->addWidget(m_titleBar);

    /* ---------- 对话气泡区 ---------- */
    m_scroll = new QScrollArea(container);
    m_scroll->setWidgetResizable(true);
    m_scroll->setFrameShape(QFrame::NoFrame);
    m_scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scroll->viewport()->setAutoFillBackground(false);
    m_scroll->setStyleSheet(QStringLiteral(
        "QScrollArea { background: transparent; border: none; }"
        "QScrollBar:vertical { background: transparent; width: 8px; margin: 0; }"
        "QScrollBar::handle:vertical { background: #C8D4E2; border-radius: 4px; min-height: 30px; }"
        "QScrollBar::handle:vertical:hover { background: #2E86DE; }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }"
        "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { background: transparent; }"));

    // 对话内容区（浅灰底，随视口铺满）
    m_chatContent = new QWidget;
    m_chatContent->setObjectName(QStringLiteral("chatContent"));
    m_chatContent->setAttribute(Qt::WA_StyledBackground, true);
    m_chatContent->setStyleSheet(QStringLiteral(
        "#chatContent { background-color: #F4F8FC; }"));
    m_chatLayout = new QVBoxLayout(m_chatContent);
    m_chatLayout->setContentsMargins(12, 14, 12, 14);
    m_chatLayout->setSpacing(10);
    m_chatLayout->setAlignment(Qt::AlignTop);
    m_chatLayout->addStretch(1); // 气泡靠上，新气泡插在 stretch 之前
    m_scroll->setWidget(m_chatContent);
    root->addWidget(m_scroll, 1);

    /* ---------- 三个快捷按钮 ---------- */
    auto *quickRow = new QHBoxLayout;
    quickRow->setContentsMargins(14, 12, 14, 4);
    quickRow->setSpacing(8);
    const QStringList actions = { QStringLiteral("查看认证状态"),
                                  QStringLiteral("修改手机号"),
                                  QStringLiteral("修改医保信息") };
    for (const QString &action : actions) {
        auto *btn = new QPushButton(action, container);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setMinimumHeight(40);
        btn->setStyleSheet(QStringLiteral(
            "QPushButton {"
            "    color: #1F4E79;"
            "    background-color: #FFFFFF;"
            "    border: 1px solid #C8D4E2;"
            "    border-radius: 20px;"
            "    font-size: 14px;"
            "    padding: 0 6px;"
            "}"
            "QPushButton:hover { background-color: #EAF2FB; }"
            "QPushButton:pressed { background-color: #D6E6F7; }"));
        connect(btn, &QPushButton::clicked, this, [this, action] { onQuickAction(action); });
        quickRow->addWidget(btn, 1);
    }
    root->addLayout(quickRow);

    /* ---------- 语音输入区：输入框 + 蓝色圆形语音按钮 + 发送按钮 ---------- */
    auto *inputRow = new QHBoxLayout;
    inputRow->setContentsMargins(14, 8, 14, 16);
    inputRow->setSpacing(10);

    m_inputEdit = new QLineEdit(container);
    m_inputEdit->setPlaceholderText(QStringLiteral("请输入问题，或按住麦克风说话"));
    m_inputEdit->setMinimumHeight(48);
    m_inputEdit->setStyleSheet(QStringLiteral(
        "QLineEdit {"
        "    color: #1F4E79;"
        "    background-color: #FFFFFF;"
        "    border: 1px solid #C8D4E2;"
        "    border-radius: 24px;"
        "    padding: 0 18px;"
        "    font-size: 16px;"
        "}"));
    connect(m_inputEdit, &QLineEdit::returnPressed, this, &AIAssistantPopup::onSend);
    inputRow->addWidget(m_inputEdit, 1);

    // 语音麦克风按钮：按住开始录音，松开自动识别并直接把文字发给 AI 助手
    m_voiceBtn = new QPushButton(container);
    m_voiceBtn->setCursor(Qt::PointingHandCursor);
    m_voiceBtn->setFixedSize(48, 48);
    m_voiceBtn->setIcon(QIcon(IconFactory::renderSvg(IconFactory::micSvg(), QSize(26, 26))));
    m_voiceBtn->setIconSize(QSize(26, 26));
    m_voiceBtn->setToolTip(QStringLiteral("按住说话，松开自动识别并发送"));
    m_voiceBtn->setStyleSheet(QStringLiteral(
        "QPushButton { background-color: #2E86DE; border: none; border-radius: 24px; }"
        "QPushButton:hover { background-color: #4A9CE8; }"
        "QPushButton:pressed { background-color: #E81123; }"));
    connect(m_voiceBtn, &QPushButton::pressed, this, &AIAssistantPopup::onVoicePressed);
    connect(m_voiceBtn, &QPushButton::released, this, &AIAssistantPopup::onVoiceReleased);
    inputRow->addWidget(m_voiceBtn);

    auto *sendBtn = new QPushButton(container);
    sendBtn->setCursor(Qt::PointingHandCursor);
    sendBtn->setFixedSize(48, 48);
    sendBtn->setIcon(QIcon(IconFactory::renderSvg(IconFactory::searchSvg(), QSize(24, 24))));
    sendBtn->setIconSize(QSize(24, 24));
    sendBtn->setStyleSheet(QStringLiteral(
        "QPushButton { background-color: #2E86DE; border: none; border-radius: 24px; }"
        "QPushButton:hover { background-color: #4A9CE8; }"
        "QPushButton:pressed { background-color: #1F6FB8; }"));
    connect(sendBtn, &QPushButton::clicked, this, &AIAssistantPopup::onSend);
    inputRow->addWidget(sendBtn);
    root->addLayout(inputRow);

    // 初始欢迎气泡 + 语音播报
    const QString welcome = QStringLiteral("我可以帮您查看或修改个人信息，您想查看哪一项？");
    addMessage(welcome, false);
    TtsPlayer::instance()->speak(welcome);
}

void AIAssistantPopup::addMessage(const QString &text, bool isUser)
{
    const QString avatar = isUser ? m_userAvatar : m_avatarPath;
    auto *bubble = new ChatBubble(text, isUser, avatar, m_chatContent);
    m_chatLayout->insertWidget(m_chatLayout->count() - 1, bubble); // 插到末尾 stretch 之前

    // 等布局更新后滚动到底部
    QTimer::singleShot(0, m_scroll, [this] {
        m_scroll->verticalScrollBar()->setValue(m_scroll->verticalScrollBar()->maximum());
    });
}

void AIAssistantPopup::replyTo(const QString &userText, const QString &aiText)
{
    if (!userText.isEmpty())
        addMessage(userText, true);
    addMessage(aiText, false);
}

void AIAssistantPopup::onQuickAction(const QString &action)
{
    QString reply;
    if (action == QStringLiteral("查看认证状态")) {
        reply = QStringLiteral("您当前为【已认证】状态，医保卡号：************1234，认证信息有效。");
    } else if (action == QStringLiteral("修改手机号")) {
        reply = QStringLiteral("好的，请在下方输入框中输入新的手机号码，点击搜索按钮提交修改。");
    } else if (action == QStringLiteral("修改医保信息")) {
        reply = QStringLiteral("好的，请提供需要修改的医保信息（例如医保卡号），我将为您处理。");
    } else {
        reply = QStringLiteral("请问还需要我帮您做什么？");
    }
    replyTo(action, reply);
    TtsPlayer::instance()->speak(reply);
}

void AIAssistantPopup::onSend()
{
    const QString text = m_inputEdit->text().trimmed();
    if (text.isEmpty())
        return;
    m_inputEdit->clear();
    submitText(text);
}

void AIAssistantPopup::submitText(const QString &text)
{
    const QString trimmed = text.trimmed();
    if (trimmed.isEmpty())
        return;

    addMessage(trimmed, true);
    addMessage(QStringLiteral("正在思考……"), false);
    QApplication::processEvents(); // 先显示"正在思考"，再发起请求

    const QString reply = MyAgent::instance()->ask(trimmed);

    // 移除"正在思考……"气泡，再追加正式回复
    if (m_chatLayout->count() > 1) {
        QLayoutItem *last = m_chatLayout->takeAt(m_chatLayout->count() - 2);
        if (last) {
            delete last->widget();
            delete last;
        }
    }
    addMessage(reply, false);
    TtsPlayer::instance()->speak(reply);
    int outDecision = 0;
    QString outContent="";
    MyAgent::instance()->getDecisionAndContent(outDecision, outContent);
    emit full_text(outDecision,outContent); // 发送AI决策码与内容
    qDebug()<<"传"<<outDecision<<outContent;
}

void AIAssistantPopup::onVoicePressed()
{
    // 首次按住麦克风时懒绑定语音识别单例的信号（此后正常按住/松开即可）
    if (!m_voiceBound) {
        m_voiceBound = true;
        auto *sr = SpeechRecognizer::instance();
        // 识别出文字 → 直接发给 AI 助手（无需再点发送按钮）
        connect(sr, &SpeechRecognizer::recognized, this,
                [this](QObject *requester, const QString &text) {
                    if (requester != this)
                        return;
                    submitText(text);
                });
        // 识别出错（模型缺失 / 无麦克风 / 没录到声音等）→ 气泡提示
        connect(sr, &SpeechRecognizer::errorOccurred, this,
                [this](QObject *requester, const QString &message) {
                    if (requester != this)
                        return;
                    addMessage(message, false);
                });
    }
    SpeechRecognizer::instance()->startListening(this); // 按住：开始录音
}

void AIAssistantPopup::onVoiceReleased()
{
    SpeechRecognizer::instance()->stopListening(); // 松开：自动识别并直接发送
}


/* ---------- 标题栏拖动悬浮窗 ---------- */

void AIAssistantPopup::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton
        && m_titleBar && m_titleBar->geometry().contains(event->pos())) {
        m_dragging = true;
        m_dragOffset = event->globalPosition().toPoint() - frameGeometry().topLeft();
        event->accept();
        return;
    }
    QWidget::mousePressEvent(event);
}

void AIAssistantPopup::mouseMoveEvent(QMouseEvent *event)
{
    if (m_dragging) {
        move(event->globalPosition().toPoint() - m_dragOffset);
        event->accept();
        return;
    }
    QWidget::mouseMoveEvent(event);
}

void AIAssistantPopup::mouseReleaseEvent(QMouseEvent *event)
{
    m_dragging = false;
    QWidget::mouseReleaseEvent(event);
}
