#include "doctorcard.h"
#include "circularavatar.h"

#include <QEnterEvent>
#include <QLabel>
#include <QMouseEvent>
#include <QVBoxLayout>

DoctorInfoCard::DoctorInfoCard(int id,
                               const QString &name,
                               const QString &dept,
                               int time,
                               const QString &avatarPath,
                               QWidget *parent)
    : QWidget(parent)
    , m_id(id)
    , m_time(time)
    , m_name(name)
    , m_dept(dept)
{
    setObjectName(QStringLiteral("doctorCard"));
    setCursor(Qt::PointingHandCursor);
    // 水平伸展填满网格列；垂直按内容自然高度，避免卡片被拉高
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    setMinimumSize(170, 180);

    // 纵向布局：圆形头像 / 姓名 / 科室
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(10, 16, 10, 14);
    layout->setSpacing(6);
    layout->setAlignment(Qt::AlignTop);

    m_avatar = new CircularAvatar(this);
    m_avatar->setFixedSize(84, 84);
    m_avatar->setAvatar(avatarPath);
    layout->addWidget(m_avatar, 0, Qt::AlignHCenter);

    m_nameLabel = new QLabel(m_name, this);
    m_nameLabel->setAlignment(Qt::AlignCenter);
    m_nameLabel->setStyleSheet(QStringLiteral(
        "color: #1F4E79; font-size: 18px; font-weight: bold; background: transparent;"));
    layout->addWidget(m_nameLabel);

    m_deptLabel = new QLabel(m_dept, this);
    m_deptLabel->setAlignment(Qt::AlignCenter);
    m_deptLabel->setStyleSheet(QStringLiteral(
        "color: #7A93B0; font-size: 14px; background: transparent;"));
    layout->addWidget(m_deptLabel);

    updateStyle();
}

int DoctorInfoCard::id() const
{
    return m_id;
}

QString DoctorInfoCard::name() const
{
    return m_name;
}

QString DoctorInfoCard::dept() const
{
    return m_dept;
}

int DoctorInfoCard::time() const
{
    return m_time;
}

bool DoctorInfoCard::isSelected() const
{
    return m_selected;
}

void DoctorInfoCard::setSelected(bool selected)
{
    if (m_selected == selected)
        return;
    m_selected = selected;
    updateStyle();
}

void DoctorInfoCard::updateStyle()
{
    const QString border = m_selected ? QStringLiteral("#2E86DE") : QStringLiteral("#C8D4E2");
    const QString bg = m_selected
                       ? QStringLiteral("#EAF2FB")
                       : (m_hover ? QStringLiteral("#F4F8FC") : QStringLiteral("#FFFFFF"));
    setStyleSheet(QStringLiteral(
        "#doctorCard {"
        "    background-color: %1;"
        "    border: 2px solid %2;"
        "    border-radius: 14px;"
        "}").arg(bg, border));
}

void DoctorInfoCard::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
        emit clicked();
    QWidget::mousePressEvent(event);
}

void DoctorInfoCard::enterEvent(QEnterEvent *event)
{
    m_hover = true;
    updateStyle();
    QWidget::enterEvent(event);
}

void DoctorInfoCard::leaveEvent(QEvent *event)
{
    m_hover = false;
    updateStyle();
    QWidget::leaveEvent(event);
}
