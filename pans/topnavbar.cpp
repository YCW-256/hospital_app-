#include "topnavbar.h"
#include "../core/iconfactory.h"

#include <QDateTime>
#include <QHBoxLayout>
#include <QLabel>
#include <QLocale>
#include <QMouseEvent>
#include <QPainter>
#include <QPushButton>
#include <QRadialGradient>
#include <QTimer>
#include <QVBoxLayout>

/* ==================== CircularLogo ==================== */

CircularLogo::CircularLogo(QWidget *parent)
    : QWidget(parent)
{
    setFixedSize(56, 56);
}

void CircularLogo::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    // 蓝青径向渐变圆底
    QRadialGradient gradient(rect().center(), width() / 2.0);
    gradient.setColorAt(0.0, QColor(QStringLiteral("#43B6E8")));
    gradient.setColorAt(1.0, QColor(QStringLiteral("#1565C0")));
    painter.setPen(Qt::NoPen);
    painter.setBrush(gradient);
    painter.drawEllipse(rect());

    // 白色医疗十字
    const int cx = width() / 2;
    const int cy = height() / 2;
    painter.setPen(QPen(Qt::white, 5.0, Qt::SolidLine, Qt::RoundCap));
    painter.drawLine(cx - 12, cy, cx + 12, cy);
    painter.drawLine(cx, cy - 12, cx, cy + 12);
}

void CircularLogo::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
        emit clicked();
    QWidget::mousePressEvent(event);
}

/* ==================== TopNavBar ==================== */

TopNavBar::TopNavBar(QWidget *parent)
    : QWidget(parent)
{
    setObjectName(QStringLiteral("topNavBar"));
    setFixedHeight(96);

    // 白色背景 + 底部浅色分隔线
    setStyleSheet(QStringLiteral(
        "#topNavBar {"
        "    background-color: #FFFFFF;"
        "    border-bottom: 2px solid #E3EAF3;"
        "}"));

    initLayout();
    updateDateTime();

    // 每秒刷新一次日期时间
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &TopNavBar::updateDateTime);
    m_timer->start(1000);
}

void TopNavBar::initLayout()
{
    auto *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(28, 6, 28, 6);
    mainLayout->setSpacing(14);

    /* ---------- 左侧：圆形 LOGO + 医院中英文名称 ---------- */
    auto *logo = new CircularLogo(this);
    connect(logo, &CircularLogo::clicked, this, &TopNavBar::logoClicked);
    mainLayout->addWidget(logo, 0, Qt::AlignVCenter);

    auto *nameLayout = new QVBoxLayout;
    nameLayout->setSpacing(2);

    auto *nameCn = new QLabel(QStringLiteral("仁济医院"), this);
    nameCn->setStyleSheet(QStringLiteral(
        "color: #1F4E79; font-size: 20px; font-weight: bold; background: transparent;"));
    auto *nameEn = new QLabel(QStringLiteral("RENJI HOSPITAL"), this);
    nameEn->setStyleSheet(QStringLiteral(
        "color: #7A93B0; font-size: 11px; background: transparent;"));

    nameLayout->addWidget(nameCn);
    nameLayout->addWidget(nameEn);
    mainLayout->addLayout(nameLayout);

    mainLayout->addStretch(1);

    /* ---------- 中间：标题（默认"自助终端"，随页面切换） ---------- */
    m_titleLabel = new QLabel(QStringLiteral("自助终端"), this);
    m_titleLabel->setAlignment(Qt::AlignCenter);
    m_titleLabel->setStyleSheet(QStringLiteral(
        "color: #1F4E79; font-size: 30px; font-weight: bold; background: transparent;"));
    mainLayout->addWidget(m_titleLabel, 0, Qt::AlignVCenter);

    mainLayout->addStretch(1);

    /* ---------- 右侧：时钟图标 + 实时日期时间 ---------- */
    auto *clockIcon = new QLabel(this);
    clockIcon->setPixmap(IconFactory::renderSvg(IconFactory::clockSvg(), QSize(30, 30)));
    mainLayout->addWidget(clockIcon, 0, Qt::AlignVCenter);

    auto *timeLayout = new QVBoxLayout;
    timeLayout->setSpacing(2);

    m_timeLabel = new QLabel(this);
    m_timeLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_timeLabel->setStyleSheet(QStringLiteral(
        "color: #1F4E79; font-size: 22px; font-weight: bold; background: transparent;"));

    m_dateLabel = new QLabel(this);
    m_dateLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_dateLabel->setStyleSheet(QStringLiteral(
        "color: #7A93B0; font-size: 13px; background: transparent;"));

    timeLayout->addWidget(m_timeLabel);
    timeLayout->addWidget(m_dateLabel);
    mainLayout->addLayout(timeLayout);

    /* ---------- 最右侧：关闭按钮（无边框窗口的退出入口） ---------- */
    auto *closeBtn = new QPushButton(QStringLiteral("×"), this);
    closeBtn->setObjectName(QStringLiteral("closeBtn"));
    closeBtn->setCursor(Qt::PointingHandCursor);
    closeBtn->setToolTip(QStringLiteral("关闭"));
    closeBtn->setFixedSize(40, 40);
    closeBtn->setStyleSheet(QStringLiteral(
        "QPushButton#closeBtn {"
        "    color: #9FB3C8;"
        "    background-color: transparent;"
        "    border: none;"
        "    border-radius: 20px;"
        "    font-size: 24px;"
        "}"
        "QPushButton#closeBtn:hover   { background-color: #E81123; color: #FFFFFF; }"
        "QPushButton#closeBtn:pressed { background-color: #B3071B; color: #FFFFFF; }"));
    connect(closeBtn, &QPushButton::clicked, this, [this] { window()->close(); });
    mainLayout->addWidget(closeBtn, 0, Qt::AlignVCenter);
}

void TopNavBar::setCenterText(const QString &text)
{
    if (m_titleLabel)
        m_titleLabel->setText(text);
}

void TopNavBar::updateDateTime()
{
    const QDateTime now = QDateTime::currentDateTime();
    // 使用中文区域格式化，保证"星期X"为中文
    const QLocale zhLocale(QLocale::Chinese, QLocale::China);

    if (m_timeLabel)
        m_timeLabel->setText(now.toString(QStringLiteral("HH:mm:ss")));
    if (m_dateLabel)
        m_dateLabel->setText(zhLocale.toString(now, QStringLiteral("yyyy年MM月dd日 dddd")));
}

/* ---------- 无边框窗口拖动支持 ---------- */

void TopNavBar::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_dragOffset = event->globalPosition().toPoint()
                       - window()->frameGeometry().topLeft();
        event->accept();
        return;
    }
    QWidget::mousePressEvent(event);
}

void TopNavBar::mouseMoveEvent(QMouseEvent *event)
{
    if ((event->buttons() & Qt::LeftButton) && !m_dragOffset.isNull()) {
        window()->move(event->globalPosition().toPoint() - m_dragOffset);
        event->accept();
        return;
    }
    QWidget::mouseMoveEvent(event);
}
