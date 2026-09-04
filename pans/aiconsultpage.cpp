#include "aiconsultpage.h"
#include "../agent/consultagent.h"
#include "../audio/ttsplayer.h"
#include "../core/iconfactory.h"
#include "../core/uistyle.h"
#include "../device/cameraserial.h"
#include "../device/devicecamera.h"
#include "../voice/speechrecognizer.h"
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
#include <QMetaObject>
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>
#include <QPushButton>
#include <QScrollArea>
#include <QScrollBar>
#include <QSize>
#include <QStringList>
#include <QThread>
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

/* 视频画面圆角半径：与 QLabel#videoLabel 样式保持一致 */
const int kVideoLabelRadius = 16;

/* 生成"铺满且不变形"的画面位图：等比放大、居中裁剪，圆角之外透明以露出纯黑底 */
QPixmap makeCoverPixmap(const QImage &frame, const QSize &target)
{
    QPixmap cover(target);
    cover.fill(Qt::transparent);
    if (frame.isNull() || target.width() <= 0 || target.height() <= 0)
        return cover;

    QPixmap scaled = QPixmap::fromImage(frame);
    scaled = scaled.scaled(target, Qt::KeepAspectRatioByExpanding,
                           Qt::SmoothTransformation);

    QPainter painter(&cover);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    QPainterPath clip;
    clip.addRoundedRect(QRectF(0, 0, target.width(), target.height()),
                        kVideoLabelRadius, kVideoLabelRadius);
    painter.setClipPath(clip);
    painter.drawPixmap((target.width() - scaled.width()) / 2,
                       (target.height() - scaled.height()) / 2, scaled);
    return cover;
}
} // namespace

AIConsultPage::AIConsultPage(QWidget *parent)
    : QWidget(parent)
    , m_doctorAvatar(UIStyle::resolveImagePath(QStringLiteral("icons/doctor2.jpg")))
    , m_userAvatar(UIStyle::resolveImagePath(QStringLiteral("icons/user.png")))
{
    initLayout();
    initCamera(); // 初始化即连接舌苔检测摄像头（TCP 接收图像）

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

    // ---------- 白色圆角视频卡片：舌苔检测摄像头画面（纯代码，不依赖 multimedia 模块）----------
    auto *videoCard = new QWidget(left);
    UIStyle::styleCard(videoCard);
    layout->addWidget(videoCard, 1);

    auto *videoLayout = new QVBoxLayout(videoCard);
    videoLayout->setContentsMargins(12, 12, 12, 12);
    videoLayout->setSpacing(10);

    // 顶部一行：摄像头提示文字 + 【打开/关闭】显示开关
    auto *ctrlRow = new QHBoxLayout;
    ctrlRow->setSpacing(10);
    auto *hintLabel = new QLabel(QStringLiteral("舌苔摄像头"), videoCard);
    hintLabel->setStyleSheet(QStringLiteral(
        "color: #1F4E79;"
        "font-size: 16px;"
        "font-weight: bold;"
        "background: transparent;"));
    ctrlRow->addWidget(hintLabel);
    ctrlRow->addStretch(1);

    m_toggleBtn = new QPushButton(QStringLiteral("打开"), videoCard);
    m_toggleBtn->setObjectName(QStringLiteral("cameraToggleBtn"));
    m_toggleBtn->setCheckable(true);   // 勾选=正在显示画面
    m_toggleBtn->setCursor(Qt::PointingHandCursor);
    m_toggleBtn->setFixedSize(120, 40);
    m_toggleBtn->setStyleSheet(QStringLiteral(
        "QPushButton#cameraToggleBtn {"
        "    color: #FFFFFF;"
        "    background-color: #1F4E79;"
        "    border: none;"
        "    border-radius: 20px;"
        "    font-size: 15px;"
        "    font-weight: bold;"
        "}"
        "QPushButton#cameraToggleBtn:hover { background-color: #2E86DE; }"
        "QPushButton#cameraToggleBtn:checked { background-color: #2E86DE; }"
        "QPushButton#cameraToggleBtn:checked:hover { background-color: #4A9CE8; }"));
    connect(m_toggleBtn, &QPushButton::toggled, this, &AIConsultPage::onToggleCamera);
    ctrlRow->addWidget(m_toggleBtn);
    videoLayout->addLayout(ctrlRow);

    // 画面承载 QLabel：默认纯黑；【打开】后铺满显示硬件上传图像（onCameraFrame 更新）
    m_videoLabel = new QLabel(videoCard);
    m_videoLabel->setObjectName(QStringLiteral("videoLabel"));
    m_videoLabel->setAlignment(Qt::AlignCenter);
    m_videoLabel->setStyleSheet(QStringLiteral(
        "QLabel#videoLabel {"
        "    background-color: #000000;"
        "    color: #9FB3C8;"
        "    border-radius: 16px;"
        "    font-size: 16px;"
        "}"));
    videoLayout->addWidget(m_videoLabel, 1);

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

    // 语音输入麦克风按钮：按住开始录音，松开自动识别并直接把文字发给 AI 医生
    m_voiceBtn = new QPushButton(bar);
    m_voiceBtn->setCursor(Qt::PointingHandCursor);
    m_voiceBtn->setFixedSize(60, 60);
    m_voiceBtn->setIcon(QIcon(IconFactory::renderSvg(IconFactory::micSvg(), QSize(32, 32))));
    m_voiceBtn->setIconSize(QSize(32, 32));
    m_voiceBtn->setToolTip(QStringLiteral("按住说话，松开自动识别并发送"));
    m_voiceBtn->setStyleSheet(QStringLiteral(
        "QPushButton { background-color: #2E86DE; border: none; border-radius: 30px; }"
        "QPushButton:hover { background-color: #4A9CE8; }"
        "QPushButton:pressed { background-color: #E81123; }"));
    connect(m_voiceBtn, &QPushButton::pressed, this, &AIConsultPage::onVoicePressed);
    connect(m_voiceBtn, &QPushButton::released, this, &AIConsultPage::onVoiceReleased);
    layout->addWidget(m_voiceBtn);

    // 症状描述输入框：占位提示，上限 300 字
    m_inputEdit = new QLineEdit(bar);
    m_inputEdit->setPlaceholderText(QStringLiteral("请描述症状，不超过 300 字"));
    m_inputEdit->setMaxLength(300);
    m_inputEdit->setClearButtonEnabled(true);
    m_inputEdit->setFixedHeight(60);
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
    submitText(text);
}

void AIConsultPage::submitText(const QString &text)
{
    const QString trimmed = text.trimmed();
    if (trimmed.isEmpty())
        return;
    emit consultRequested(trimmed);
    sendToAgent(trimmed);
}

void AIConsultPage::onVoicePressed()
{
    // 首次按住麦克风时懒绑定语音识别单例的信号（此后正常按住/松开即可）
    if (!m_voiceBound) {
        m_voiceBound = true;
        auto *sr = SpeechRecognizer::instance();
        // 识别出文字 → 直接作为一条用户消息发给 AI 医生（无需点【开始问诊】）
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
                    onVoiceError(message);
                });
    }
    SpeechRecognizer::instance()->startListening(this); // 按住：开始录音
}

void AIConsultPage::onVoiceReleased()
{
    SpeechRecognizer::instance()->stopListening(); // 松开：自动识别并直接发送
}

void AIConsultPage::onVoiceError(const QString &message)
{
    addMessage(message, false); // 以一条 AI 侧气泡展示提示（不做语音播报）
}

AIConsultPage::~AIConsultPage()
{
    // 语音识别若正在进行则取消（释放麦克风 / 丢弃结果）
    if (m_voiceBound)
        SpeechRecognizer::instance()->cancelFor(this);

    // 安全停止摄像头串口线程：先跨线程执行 stop，再退出事件循环
    if (m_serialThread) {
        if (m_serialCtl && m_serialThread->isRunning()) {
            QMetaObject::invokeMethod(m_serialCtl, "stop", Qt::BlockingQueuedConnection);
            m_serialThread->quit();
            m_serialThread->wait();
        }
        delete m_serialCtl;
        m_serialCtl = nullptr;
        delete m_serialThread;
        m_serialThread = nullptr;
    }
    // 安全停止摄像头网络线程：先跨线程执行 stop（关闭 socket + 关闭自动重连），再退出事件循环
    if (m_cameraThread) {
        if (m_camera && m_cameraThread->isRunning()) {
            QMetaObject::invokeMethod(m_camera, "stop", Qt::BlockingQueuedConnection);
            m_cameraThread->quit();
            m_cameraThread->wait();
        }
        // 线程已停，直接在持有线程释放两个对象
        delete m_camera;
        m_camera = nullptr;
        delete m_cameraThread;
        m_cameraThread = nullptr;
    }
}

void AIConsultPage::initCamera()
{
    // 硬件默认参数取自参考上位机 rv1106_test01 的 ui 预设，此处写死
    const QString cameraHost = QStringLiteral("10.1.1.144");
    const quint16 cameraPort = 6868;
    const QString serialPort = QStringLiteral("COM9"); // 用户实际使用的串口号
    const qint32  serialBaud = 115200;

    // 串口控制器：打开 COM9 成功后自动下发 0x0001，让摄像头开始推视频
    m_serialCtl = new CameraSerial(serialPort, serialBaud);
    m_serialThread = new QThread;
    m_serialCtl->moveToThread(m_serialThread);
    connect(m_serialThread, &QThread::started, m_serialCtl, &CameraSerial::start);
    connect(m_serialCtl, &CameraSerial::logMessage, this,
            [](const QString &msg) { qDebug().noquote() << msg; });
    m_serialThread->start();

    // 网络接收器：接收摄像头经 TCP 推来的图像帧（初始化即连接设备）
    m_camera = new DeviceCamera(cameraHost, cameraPort);
    m_cameraThread = new QThread;
    m_camera->moveToThread(m_cameraThread);
    connect(m_cameraThread, &QThread::started, m_camera, &DeviceCamera::start);
    connect(m_camera, &DeviceCamera::frameReady, this, &AIConsultPage::onCameraFrame);
    connect(m_camera, &DeviceCamera::logMessage, this,
            [](const QString &msg) { qDebug().noquote() << msg; });
    m_cameraThread->start();
}

void AIConsultPage::onCameraFrame(const QImage &frame)
{
    // 始终缓存最新帧（关闭时也继续接收），【打开】瞬间即可显示最新画面
    m_latestFrame = frame;
    if (m_cameraOn)
        refreshVideoLabel();
}

void AIConsultPage::onToggleCamera(bool checked)
{
    m_cameraOn = checked;
    m_toggleBtn->setText(checked ? QStringLiteral("关闭") : QStringLiteral("打开"));
    refreshVideoLabel();
    qDebug().noquote() << (checked ? QStringLiteral("[摄像头] 画面已打开")
                                   : QStringLiteral("[摄像头] 画面已关闭，显示纯黑"));
}

void AIConsultPage::refreshVideoLabel()
{
    if (!m_videoLabel)
        return;
    if (m_cameraOn && !m_latestFrame.isNull()) {
        // 打开且有图像：等比放大铺满（居中裁剪，不变形）
        m_videoLabel->setPixmap(makeCoverPixmap(m_latestFrame, m_videoLabel->size()));
    } else {
        // 关闭或尚未收到帧：清空内容，露出纯黑底色
        m_videoLabel->clear();
    }
}
