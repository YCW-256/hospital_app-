#include "aiconsultpage.h"
#include "../agent/consultagent.h"
#include "../audio/ttsplayer.h"
#include "../core/iconfactory.h"
#include "../core/uistyle.h"
#include "childs/chatbubble.h"
#include "childs/circularavatar.h"

#include <QApplication>
#include <QButtonGroup>
#include <QDebug>
#include <QFrame>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QLineEdit>
#include <QPixmap>
#include <QPushButton>
#include <QScrollArea>
#include <QScrollBar>
#include <QSize>
#include <QStringList>
#include <QTimer>
#include <QVBoxLayout>

namespace {
/* 底部按钮行高度（左侧症状按钮与右侧【返回首页】同一行、等高） */
const int kBottomRowHeight = 64;

/* 单个"常见症状"快捷按钮：左侧蓝色圆形图标 + 文字，选中高亮 */
QPushButton *createSymptomButton(const QString &text, const QPixmap &icon, QWidget *parent)
{
    auto *btn = new QPushButton(text, parent);
    btn->setCursor(Qt::PointingHandCursor);
    btn->setCheckable(true);           // 可选中，配合 QButtonGroup 互斥
    btn->setFixedHeight(kBottomRowHeight);
    btn->setIcon(QIcon(icon));
    btn->setIconSize(QSize(36, 36));
    btn->setStyleSheet(QStringLiteral(
        "QPushButton {"
        "    color: #1F4E79;"
        "    background-color: #F4F8FC;"
        "    border: 2px solid #C8D4E2;"
        "    border-radius: 20px;"
        "    font-size: 18px;"
        "    font-weight: bold;"
        "    padding-left: 12px;"
        "    text-align: left;"
        "}"
        "QPushButton:hover   { background-color: #EAF2FB; border-color: #2E86DE; }"
        "QPushButton:checked {"
        "    background-color: #E4F0FB;"
        "    border-color: #2E86DE;"
        "    color: #2E86DE;"
        "}"));
    return btn;
}
} // namespace

AIConsultPage::AIConsultPage(QWidget *parent)
    : QWidget(parent)
    , m_doctorAvatar(UIStyle::resolveImagePath(QStringLiteral("icons/doctor2.jpg")))
    , m_userAvatar(UIStyle::resolveImagePath(QStringLiteral("icons/user.png")))
{
    initLayout();

    // 初始欢迎气泡 + 语音播报
    const QString welcome = QStringLiteral("您好，我是AI快速问诊医生，请选择或输入您的主要症状，"
                                           "我将为您做初步问诊分析。");
    addMessage(welcome, false);
    // 播报延迟到事件循环启动后执行：SAPI 语音播放依赖消息泵，
    // 若在 QApplication::exec() 启动前（如 MainWindow 构造期）调用会阻塞导致窗口无法显示
    QTimer::singleShot(0, this, [welcome] {
        TtsPlayer::instance()->speak(welcome);
    });
}

void AIConsultPage::initLayout()
{
    UIStyle::styleTransparentPage(this); // 透出主窗口蓝色渐变
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(64, 36, 64, 30);
    root->setSpacing(24);

    // 主区域：左侧视频 + 症状按钮，右侧聊天窗口
    auto *bodyRow = new QHBoxLayout;
    bodyRow->setSpacing(30);
    bodyRow->addWidget(createLeftArea(), 5);
    bodyRow->addWidget(createRightArea(), 4);
    root->addLayout(bodyRow, 1);

    // 底部通栏输入区
    root->addWidget(createBottomBar());
}

QWidget *AIConsultPage::createLeftArea()
{
    auto *left = new QWidget(this);
    auto *layout = new QVBoxLayout(left);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(16);

    // ---------- 白色圆角视频区域：内部 QLabel 承载视频 ----------
    auto *videoCard = new QWidget(left);
    UIStyle::styleCard(videoCard);
    layout->addWidget(videoCard, 1);

    auto *videoLayout = new QVBoxLayout(videoCard);
    videoLayout->setContentsMargins(12, 12, 12, 12);

    m_videoLabel = new QLabel(videoCard);
    m_videoLabel->setObjectName(QStringLiteral("videoLabel"));
    m_videoLabel->setAlignment(Qt::AlignCenter);
    m_videoLabel->setStyleSheet(QStringLiteral(
        "QLabel#videoLabel {"
        "    background-color: #1F2A3A;"
        "    color: #9FB3C8;"
        "    border-radius: 16px;"
        "    font-size: 16px;"
        "}"));
    videoLayout->addWidget(m_videoLabel, 1);

    // QLabel 直接承载视频区域内容（纯代码，不依赖 multimedia 模块）：
    // 默认显示医生占位图；后续如需视频，可直接对 m_videoLabel setMovie/setPixmap 更换内容。
    const QString placeholderPath =
        UIStyle::resolveImagePath(QStringLiteral("icons/doctor2.jpg"));
    QPixmap placeholder(placeholderPath);
    if (!placeholder.isNull())
        m_videoLabel->setPixmap(placeholder.scaled(360, 220, Qt::KeepAspectRatio,
                                                   Qt::SmoothTransformation));
    else
        m_videoLabel->setText(QStringLiteral("健康宣教视频"));

    // ---------- 底部：常见症状快捷按钮（单行 4 个，与右侧【返回首页】同一行） ----------
    const QStringList symptomNames = {
        QStringLiteral("发热咳嗽"), QStringLiteral("肠胃不适"),
        QStringLiteral("皮肤问题"), QStringLiteral("舌苔健康"),
    };
    const QList<QPixmap> symptomIcons = {
        IconFactory::renderSvg(IconFactory::symptomFeverSvg(), QSize(36, 36)),
        IconFactory::renderSvg(IconFactory::symptomStomachSvg(), QSize(36, 36)),
        IconFactory::renderSvg(IconFactory::symptomSkinSvg(), QSize(36, 36)),
        IconFactory::renderSvg(IconFactory::symptomTongueSvg(), QSize(36, 36)),
    };

    auto *symptomRow = new QHBoxLayout;
    symptomRow->setSpacing(14);
    auto *group = new QButtonGroup(this);
    group->setExclusive(true); // 症状单选，同一时间只选一种
    for (int i = 0; i < symptomNames.size(); ++i) {
        auto *btn = createSymptomButton(symptomNames.at(i), symptomIcons.at(i), left);
        group->addButton(btn, i);

        // 点击即作为一条用户消息发给 AI 医生
        connect(btn, &QPushButton::clicked, this, [this, symptomNames, i] {
            qDebug().noquote() << QStringLiteral("选择症状：%1").arg(symptomNames.at(i));
            sendToAgent(symptomNames.at(i));
        });
        symptomRow->addWidget(btn, 1);
    }
    layout->addLayout(symptomRow);

    return left;
}

QWidget *AIConsultPage::createRightArea()
{
    auto *right = new QWidget(this);
    auto *layout = new QVBoxLayout(right);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(16);

    // ---------- 白色圆角聊天窗口 ----------
    auto *chatCard = new QWidget(right);
    UIStyle::styleCard(chatCard);
    layout->addWidget(chatCard, 1);

    auto *cardLayout = new QVBoxLayout(chatCard);
    cardLayout->setContentsMargins(0, 0, 0, 0);
    cardLayout->setSpacing(0);

    // 标题栏：深蓝底 + 医生头像 + 标题 + 在线状态
    auto *header = new QWidget(chatCard);
    header->setFixedHeight(64);
    header->setStyleSheet(QStringLiteral(
        "background-color: #1F4E79;"
        "border-top-left-radius: 24px;"
        "border-top-right-radius: 24px;"));
    auto *headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(20, 10, 20, 10);
    headerLayout->setSpacing(12);

    auto *avatar = new CircularAvatar(header);
    avatar->setFixedSize(40, 40);
    avatar->setAvatar(m_doctorAvatar);
    headerLayout->addWidget(avatar);

    auto *title = new QLabel(QStringLiteral("AI 快速问诊"), header);
    title->setStyleSheet(QStringLiteral(
        "color: #FFFFFF; font-size: 20px; font-weight: bold; background: transparent;"));
    headerLayout->addWidget(title);

    auto *status = new QLabel(QStringLiteral("● 在线"), header);
    status->setStyleSheet(QStringLiteral(
        "color: #7CE0A5; font-size: 14px; background: transparent;"));
    headerLayout->addWidget(status);
    headerLayout->addStretch(1);
    cardLayout->addWidget(header);

    // 对话气泡滚动区（浅灰底）
    m_scroll = new QScrollArea(chatCard);
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
    cardLayout->addWidget(m_scroll, 1);

    // 聊天窗口下方的【返回首页】按钮（与左侧症状按钮行等高）
    auto *backBtn = UIStyle::createPrimaryButton(QStringLiteral("返回首页"), right);
    backBtn->setFixedSize(220, kBottomRowHeight);
    connect(backBtn, &QPushButton::clicked, this, &AIConsultPage::backRequested);
    layout->addWidget(backBtn, 0, Qt::AlignHCenter);

    return right;
}

QWidget *AIConsultPage::createBottomBar()
{
    auto *bar = new QWidget(this);
    auto *layout = new QHBoxLayout(bar);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(20);

    // 左侧症状描述输入框：麦克风图标 + 占位提示，上限 300 字
    m_inputEdit = new QLineEdit(bar);
    m_inputEdit->setPlaceholderText(QStringLiteral("请描述症状，不超过 300 字"));
    m_inputEdit->setMaxLength(300);
    m_inputEdit->setClearButtonEnabled(true);
    m_inputEdit->setFixedHeight(60);
    m_inputEdit->addAction(
        QIcon(IconFactory::renderSvg(IconFactory::micBlueSvg(), QSize(28, 28))),
        QLineEdit::LeadingPosition);
    m_inputEdit->setStyleSheet(QStringLiteral(
        "QLineEdit {"
        "    background-color: #FFFFFF;"
        "    color: #1F4E79;"
        "    font-size: 18px;"
        "    border: 2px solid #C8D4E2;"
        "    border-radius: 30px;"
        "    padding: 0 18px;"
        "    placeholder-text-color: #9FB3C8;"
        "    selection-background-color: #2E86DE;"
        "    selection-color: #FFFFFF;"
        "}"
        "QLineEdit:focus { border-color: #2E86DE; }"));
    connect(m_inputEdit, &QLineEdit::returnPressed, this, &AIConsultPage::onSend);
    layout->addWidget(m_inputEdit, 1);

    // 右侧蓝色圆角【开始问诊】按钮
    auto *consultBtn = UIStyle::createPrimaryButton(QStringLiteral("开始问诊"), bar);
    consultBtn->setFixedSize(200, 60);
    connect(consultBtn, &QPushButton::clicked, this, &AIConsultPage::onSend);
    layout->addWidget(consultBtn);

    return bar;
}

void AIConsultPage::addMessage(const QString &text, bool isUser)
{
    const QString avatar = isUser ? m_userAvatar : m_doctorAvatar;
    auto *bubble = new ChatBubble(text, isUser, avatar, m_chatContent);
    m_chatLayout->insertWidget(m_chatLayout->count() - 1, bubble); // 插到末尾 stretch 之前

    // 等布局更新后滚动到底部
    QTimer::singleShot(0, m_scroll, [this] {
        m_scroll->verticalScrollBar()->setValue(m_scroll->verticalScrollBar()->maximum());
    });
}

void AIConsultPage::sendToAgent(const QString &userText)
{
    addMessage(userText, true);
    addMessage(QStringLiteral("正在思考……"), false);
    QApplication::processEvents(); // 先显示"正在思考"，再发起请求

    // 走独立的 AI 快速问诊智能体（ConsultAgent，医疗问诊提示词 + 独立对话记忆）
    const QString reply = ConsultAgent::instance()->ask(userText);

    // 移除"正在思考……"气泡，再追加正式回复
    if (m_chatLayout->count() > 1) {
        QLayoutItem *last = m_chatLayout->takeAt(m_chatLayout->count() - 2);
        if (last) {
            delete last->widget();
            delete last;
        }
    }
    addMessage(reply, false);
    TtsPlayer::instance()->speak(reply); // AI 回复自动语音播报
}

void AIConsultPage::onSend()
{
    const QString text = m_inputEdit->text().trimmed();
    if (text.isEmpty())
        return;
    m_inputEdit->clear();
    emit consultRequested(text);
    sendToAgent(text);
}
