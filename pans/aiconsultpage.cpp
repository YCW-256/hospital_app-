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
#include <QBuffer>
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
#include <QProgressBar>          // ★ 新增
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
    btn->setCheckable(true);
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

/* 视频画面圆角半径 */
const int kVideoLabelRadius = 16;

/* 【上传】按钮输出图片的边长 */
const int kUploadImageSize = 640;

/* 把画面做成上传用的 640×640 正方形图片 */
QImage makeUploadImage(const QImage &frame)
{
    QImage out(kUploadImageSize, kUploadImageSize, QImage::Format_RGB888);
    out.fill(Qt::black);
    if (frame.isNull())
        return out;

    QImage src = frame.convertToFormat(QImage::Format_RGB888);
    src = src.scaled(kUploadImageSize, kUploadImageSize,
                     Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);

    QPainter painter(&out);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    painter.drawImage((kUploadImageSize - src.width()) / 2,
                      (kUploadImageSize - src.height()) / 2, src);
    return out;
}

/* 把图片编码为字节流 */
QByteArray saveToBytes(const QImage &image, const char *format, int quality)
{
    QByteArray data;
    QBuffer buffer(&data);
    if (!buffer.open(QIODevice::WriteOnly))
        return QByteArray();
    if (!image.save(&buffer, format, quality))
        return QByteArray();
    buffer.close();
    return data;
}

/* 编码上传图片：优先 JPEG，回退 PNG */
QByteArray encodeUploadImage(const QImage &image, QString *formatOut)
{
    QByteArray data = saveToBytes(image, "JPEG", 90);
    if (!data.isEmpty()) {
        if (formatOut)
            *formatOut = QStringLiteral("JPEG");
        return data;
    }
    data = saveToBytes(image, "PNG", -1);
    if (formatOut)
        *formatOut = data.isEmpty() ? QStringLiteral("未知") : QStringLiteral("PNG");
    return data;
}

/* 生成"铺满且不变形"的画面位图 */
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
    initCamera();

    const QString welcome = QStringLiteral("您好，我是AI快速问诊医生，请选择或输入您的主要症状，"
                                           "我将为您做初步问诊分析。");
    addMessage(welcome, false);
    QTimer::singleShot(0, this, [welcome] {
        TtsPlayer::instance()->speak(welcome);
    });
}

void AIConsultPage::initLayout()
{
    UIStyle::styleTransparentPage(this);
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(64, 36, 64, 30);
    root->setSpacing(24);

    auto *bodyRow = new QHBoxLayout;
    bodyRow->setSpacing(30);
    bodyRow->addWidget(createLeftArea(), 5);
    bodyRow->addWidget(createRightArea(), 4);
    root->addLayout(bodyRow, 1);

    root->addWidget(createBottomBar());
}

QWidget *AIConsultPage::createLeftArea()
{
    auto *left = new QWidget(this);
    auto *layout = new QVBoxLayout(left);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(16);

    // ---------- 白色圆角视频卡片 ----------
    auto *videoCard = new QWidget(left);
    UIStyle::styleCard(videoCard);
    layout->addWidget(videoCard, 1);

    auto *videoLayout = new QVBoxLayout(videoCard);
    videoLayout->setContentsMargins(12, 12, 12, 12);
    videoLayout->setSpacing(10);

    // 顶部一行
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
    m_toggleBtn->setCheckable(true);
    m_toggleBtn->setCursor(Qt::PointingHandCursor);
    m_toggleBtn->setFixedSize(110, 40);
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

    m_detectBtn = new QPushButton(QStringLiteral("舌苔检测"), videoCard);
    m_detectBtn->setObjectName(QStringLiteral("cameraDetectBtn"));
    m_detectBtn->setCursor(Qt::PointingHandCursor);
    m_detectBtn->setFixedSize(110, 40);
    m_detectBtn->setStyleSheet(QStringLiteral(
        "QPushButton#cameraDetectBtn {"
        "    color: #FFFFFF;"
        "    background-color: #2A8A6A;"
        "    border: none;"
        "    border-radius: 20px;"
        "    font-size: 15px;"
        "    font-weight: bold;"
        "}"
        "QPushButton#cameraDetectBtn:hover { background-color: #33A37E; }"));
    connect(m_detectBtn, &QPushButton::clicked, this, &AIConsultPage::onDetectTongue);
    ctrlRow->addWidget(m_detectBtn);

    m_resumeBtn = new QPushButton(QStringLiteral("再次检测"), videoCard);
    m_resumeBtn->setObjectName(QStringLiteral("cameraResumeBtn"));
    m_resumeBtn->setCursor(Qt::PointingHandCursor);
    m_resumeBtn->setFixedSize(110, 40);
    m_resumeBtn->setStyleSheet(QStringLiteral(
        "QPushButton#cameraResumeBtn {"
        "    color: #FFFFFF;"
        "    background-color: #E67E22;"
        "    border: none;"
        "    border-radius: 20px;"
        "    font-size: 15px;"
        "    font-weight: bold;"
        "}"
        "QPushButton#cameraResumeBtn:hover { background-color: #F39C12; }"));
    connect(m_resumeBtn, &QPushButton::clicked, this, &AIConsultPage::onResumeVideo);
    ctrlRow->addWidget(m_resumeBtn);

    m_uploadBtn = new QPushButton(QStringLiteral("上传"), videoCard);
    m_uploadBtn->setObjectName(QStringLiteral("cameraUploadBtn"));
    m_uploadBtn->setCursor(Qt::PointingHandCursor);
    m_uploadBtn->setFixedSize(110, 40);
    m_uploadBtn->setToolTip(QStringLiteral("把当前画面缩放为 640×640 后上传"));
    m_uploadBtn->setStyleSheet(QStringLiteral(
        "QPushButton#cameraUploadBtn {"
        "    color: #FFFFFF;"
        "    background-color: #8E44AD;"
        "    border: none;"
        "    border-radius: 20px;"
        "    font-size: 15px;"
        "    font-weight: bold;"
        "}"
        "QPushButton#cameraUploadBtn:hover { background-color: #9B59B6; }"));
    connect(m_uploadBtn, &QPushButton::clicked, this, &AIConsultPage::onUploadImage);
    ctrlRow->addWidget(m_uploadBtn);
    videoLayout->addLayout(ctrlRow);

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

    // ★ 新增：浮层进度条 —— 直接挂在 videoCard 上，不进任何布局，不挤压控件
    m_progressBar = new QProgressBar(videoCard);
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(0);
    m_progressBar->setTextVisible(true);
    m_progressBar->setFormat(QStringLiteral("%p%"));
    m_progressBar->setStyleSheet(QStringLiteral(
        "QProgressBar {"
        "    background-color: rgba(255, 255, 255, 220);"
        "    border: 1px solid #2E86DE;"
        "    border-radius: 12px;"
        "    color: #1F4E79;"
        "    font-size: 13px;"
        "    font-weight: bold;"
        "    text-align: center;"
        "}"
        "QProgressBar::chunk {"
        "    background-color: #2E86DE;"
        "    border-radius: 11px;"
        "}"));
    m_progressBar->setFixedSize(360, 26);
    m_progressBar->move(240, 260);         // 固定在 videoCard 左上角 (20, 20)
    m_progressBar->setVisible(false);
    m_progressBar->raise();

    // ---------- 底部：常见症状快捷按钮 ----------
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
    group->setExclusive(true);
    for (int i = 0; i < symptomNames.size(); ++i) {
        auto *btn = createSymptomButton(symptomNames.at(i), symptomIcons.at(i), left);
        group->addButton(btn, i);

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

    auto *chatCard = new QWidget(right);
    UIStyle::styleCard(chatCard);
    layout->addWidget(chatCard, 1);

    auto *cardLayout = new QVBoxLayout(chatCard);
    cardLayout->setContentsMargins(0, 0, 0, 0);
    cardLayout->setSpacing(0);

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
    m_chatLayout->addStretch(1);
    m_scroll->setWidget(m_chatContent);
    cardLayout->addWidget(m_scroll, 1);

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
    m_chatLayout->insertWidget(m_chatLayout->count() - 1, bubble);

    QTimer::singleShot(0, m_scroll, [this] {
        m_scroll->verticalScrollBar()->setValue(m_scroll->verticalScrollBar()->maximum());
    });
}

void AIConsultPage::sendToAgent(const QString &userText)
{
    addMessage(userText, true);
    addMessage(QStringLiteral("正在思考……"), false);
    QApplication::processEvents();

    const QString reply = ConsultAgent::instance()->ask(userText);

    if (m_chatLayout->count() > 1) {
        QLayoutItem *last = m_chatLayout->takeAt(m_chatLayout->count() - 2);
        if (last) {
            delete last->widget();
            delete last;
        }
    }
    addMessage(reply, false);
    TtsPlayer::instance()->speak(reply);
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
    if (!m_voiceBound) {
        m_voiceBound = true;
        auto *sr = SpeechRecognizer::instance();
        connect(sr, &SpeechRecognizer::recognized, this,
                [this](QObject *requester, const QString &text) {
                    if (requester != this)
                        return;
                    submitText(text);
                });
        connect(sr, &SpeechRecognizer::errorOccurred, this,
                [this](QObject *requester, const QString &message) {
                    if (requester != this)
                        return;
                    onVoiceError(message);
                });
    }
    SpeechRecognizer::instance()->startListening(this);
}

void AIConsultPage::onVoiceReleased()
{
    SpeechRecognizer::instance()->stopListening();
}

void AIConsultPage::onVoiceError(const QString &message)
{
    addMessage(message, false);
}

AIConsultPage::~AIConsultPage()
{
    if (m_voiceBound)
        SpeechRecognizer::instance()->cancelFor(this);

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
    if (m_cameraThread) {
        if (m_camera && m_cameraThread->isRunning()) {
            QMetaObject::invokeMethod(m_camera, "stop", Qt::BlockingQueuedConnection);
            m_cameraThread->quit();
            m_cameraThread->wait();
        }
        delete m_camera;
        m_camera = nullptr;
        delete m_cameraThread;
        m_cameraThread = nullptr;
    }
}

void AIConsultPage::initCamera()
{
    const QString cameraHost = QStringLiteral("10.1.1.144");
    const quint16 cameraPort = 6868;
    const QString serialPort = QStringLiteral("COM9");
    const qint32  serialBaud = 115200;

    m_serialCtl = new CameraSerial(serialPort, serialBaud);
    m_serialThread = new QThread;
    m_serialCtl->moveToThread(m_serialThread);
    connect(m_serialThread, &QThread::started, m_serialCtl, &CameraSerial::start);
    connect(m_serialCtl, &CameraSerial::logMessage, this,
            [](const QString &msg) { qDebug().noquote() << msg; });
    connect(m_serialCtl, &CameraSerial::tongueDetected, this, &AIConsultPage::onTongueDetected);
    m_serialThread->start();

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

void AIConsultPage::onDetectTongue()
{
    if (!m_serialCtl) {
        qDebug().noquote() << QStringLiteral("[舌苔检测] 串口控制器未初始化");
        return;
    }
    QMetaObject::invokeMethod(m_serialCtl, "triggerTongueDetect", Qt::QueuedConnection);
    qDebug().noquote() << QStringLiteral("[舌苔检测] 已触发单帧舌苔检测（下发 0x0010），"
                                         "检测到舌苔后设备将自动上行回报…");
}

void AIConsultPage::onResumeVideo()
{
    if (!m_serialCtl) {
        qDebug().noquote() << QStringLiteral("[舌苔检测] 串口控制器未初始化");
        return;
    }
    QMetaObject::invokeMethod(m_serialCtl, "resumeVideo", Qt::QueuedConnection);
    qDebug().noquote() << QStringLiteral("[舌苔检测] 已请求恢复实时预览（下发 0x0001），"
                                         "可再次点击【舌苔检测】采集舌苔…");
}

void AIConsultPage::deal_img()
{
    if (m_uploadImageData.isEmpty()) {
        qDebug().noquote() << QStringLiteral("[舌苔上传] 无图片数据，跳过发送");
        return;
    }

    const int totalBytes = m_uploadImageData.size();
    const int chunkSize  = sizeof(IMG_T::img_data);   //

    const int totalFrags = (totalBytes + chunkSize - 1) / chunkSize;

    qDebug().noquote() << QStringLiteral("[舌苔上传] 开始分片：共 %1 字节，每片 %2，共 %3 片")
                              .arg(totalBytes).arg(chunkSize).arg(totalFrags);

    QString ts = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
    QString fullName = CData::m_name + "_" + ts;
    qDebug() << "总数" << totalFrags;

    // ★ 显示浮层进度条
    if (m_progressBar) {
        m_progressBar->setVisible(true);
        m_progressBar->setRange(0, totalFrags);
        m_progressBar->setValue(0);
        m_progressBar->raise();
    }

    for (int i = 0; i < totalFrags; ++i) {
        const int offset = i * chunkSize;
        const int remain = totalBytes - offset;
        const int thisLen = qMin(chunkSize, remain);

        IMG_T img;
        memset(&img, 0, sizeof(img));
        img.index  = i;
        img.total  = totalFrags;
        img.width  = 640;
        img.height = 640;
        img.id=CData::m_id;
        strcpy(img.file_name, fullName.toUtf8().constData());
        memcpy(img.img_data, m_uploadImageData.constData() + offset, thisLen);

        HEAD head;
        memset(&head, 0, sizeof(head));
        head.type        = SERVICE_TYPE::IMG_UPLOAD;
        head.is_fragment = 1;
        head.frag_index  = i;
        head.frag_total  = totalFrags;
        head.len         = sizeof(IMG_T);

        const int packetSize = sizeof(HEAD) + sizeof(IMG_T);
        QByteArray data;
        data.resize(packetSize);
        memcpy(data.data(),                &head, sizeof(HEAD));
        memcpy(data.data() + sizeof(HEAD), &img,  sizeof(IMG_T));

        emit data_ready(data, packetSize);

        // ★ 更新进度条
        if (m_progressBar) {
            m_progressBar->setValue(i + 1);
            if (i % 10 == 0 || i == totalFrags - 1) {
                QApplication::processEvents();
            }
        }

        //QThread::msleep(2);
    }

    // ★ 隐藏浮层进度条
    if (m_progressBar) {
        m_progressBar->setVisible(false);
        m_progressBar->setValue(0);
    }

    qDebug().noquote() << QStringLiteral("[舌苔上传] 分片发送完成，共 %1 片").arg(totalFrags);
}

void AIConsultPage::onUploadImage()
{
    const QImage upload = makeUploadImage(m_latestFrame);

    if (upload.isNull()) {
        qDebug() << "图片为空";
        return;
    }

    QImage rgb = upload.convertToFormat(QImage::Format_RGB888);

    const int w = rgb.width();
    const int h = rgb.height();
    const int channels = 3;

    QByteArray raw;
    raw.reserve(w * h * channels);
    for (int y = 0; y < h; ++y) {
        raw.append(reinterpret_cast<const char*>(rgb.constScanLine(y)),
                   w * channels);
    }

    m_uploadImageData = raw;
    qDebug().noquote() << QStringLiteral("[舌苔上传] 已生成像素数据：%1×%2 RGB，共 %3 字节")
                              .arg(w).arg(h).arg(m_uploadImageData.size());
    deal_img();
}

void AIConsultPage::onTongueDetected(int classId, float confidence)
{
    static const char *kTongueNames[] = {
        "灰黑苔", "镜面舌", "薄白苔", "白腻苔", "黄腻苔",
    };
    const int nameCount = static_cast<int>(sizeof(kTongueNames) / sizeof(kTongueNames[0]));
    const QString name = (classId >= 0 && classId < nameCount)
                             ? QString::fromUtf8(kTongueNames[classId])
                             : QStringLiteral("未识别(0x%1)").arg(classId, 2, 16, QLatin1Char('0'));

    qDebug().noquote() << QStringLiteral("[舌苔检测] 类别=%1(编码%2)  置信度=%3%")
                              .arg(name)
                              .arg(classId)
                              .arg(QString::number(confidence * 100.0f, 'f', 1));

    if (classId < 0 || classId >= nameCount)
        return;

    const QString ask = QStringLiteral("通过边缘模型检测用户的舌头为%1，请你给出建议").arg(name);
    submitText(ask);
}

void AIConsultPage::refreshVideoLabel()
{
    if (!m_videoLabel)
        return;
    if (m_cameraOn && !m_latestFrame.isNull()) {
        m_videoLabel->setPixmap(makeCoverPixmap(m_latestFrame, m_videoLabel->size()));
    } else {
        m_videoLabel->clear();
    }
}