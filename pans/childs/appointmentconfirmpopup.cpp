#include "appointmentconfirmpopup.h"
#include "../../core/uistyle.h"

#include <QDate>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

AppointmentConfirmPopup::AppointmentConfirmPopup(QWidget *parent)
    : QWidget(parent)
{
    // 无边框、置顶、悬浮工具窗（不占用任务栏），与 AIAssistantPopup 弹窗写法一致
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground, true); // 透明窗口底，实现白色圆角容器
    setFixedSize(480, 400);

    initUi();
}

void AppointmentConfirmPopup::setAppointmentInfo(const QString &doctorName,
                                                 const QString &department,
                                                 int time)
{
    m_doctorValue->setText(doctorName);
    m_deptValue->setText(department);
    // 医生数据来自服务器"今日值班"，挂号日期取当天
    m_dateValue->setText(QDate::currentDate().toString(QStringLiteral("yyyy年MM月dd日")));
    m_sessionValue->setText(sessionText(time));
}

QString AppointmentConfirmPopup::sessionText(int time)
{
    switch (time) {
    case 1:  return QStringLiteral("下午");
    case 2:  return QStringLiteral("晚上");
    default: return QStringLiteral("上午"); // 0=上午
    }
}

void AppointmentConfirmPopup::initUi()
{
    /* ---------- 外层白色圆角容器 ---------- */
    auto *container = new QWidget(this);
    container->setObjectName(QStringLiteral("appointConfirmContainer"));
    container->setAttribute(Qt::WA_StyledBackground, true);
    container->setStyleSheet(QStringLiteral(
        "#appointConfirmContainer {"
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

    /* ---------- 深蓝标题栏：确认挂号 ---------- */
    auto *titleBar = new QWidget(container);
    titleBar->setFixedHeight(56);
    titleBar->setStyleSheet(QStringLiteral(
        "background-color: #1F4E79;"
        "border-top-left-radius: 16px;"
        "border-top-right-radius: 16px;"));
    auto *titleLayout = new QVBoxLayout(titleBar);
    titleLayout->setContentsMargins(0, 0, 0, 0);
    auto *title = new QLabel(QStringLiteral("确认挂号"), titleBar);
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet(QStringLiteral(
        "color: #FFFFFF; font-size: 20px; font-weight: bold; background: transparent;"));
    titleLayout->addWidget(title);
    root->addWidget(titleBar);

    /* ---------- 内容区 ---------- */
    auto *body = new QWidget(container);
    auto *bodyLayout = new QVBoxLayout(body);
    bodyLayout->setContentsMargins(30, 24, 30, 26);
    bodyLayout->setSpacing(14);

    // 提示文字
    auto *tip = new QLabel(QStringLiteral("请核对以下挂号信息"), body);
    tip->setAlignment(Qt::AlignCenter);
    tip->setStyleSheet(QStringLiteral(
        "color: #7A93B0; font-size: 15px; background: transparent;"));
    bodyLayout->addWidget(tip);

    // 挂号信息浅灰圆角卡片：医生 / 科室 / 挂号日期 / 就诊时段
    auto *infoCard = new QWidget(body);
    infoCard->setObjectName(QStringLiteral("confirmInfoCard"));
    infoCard->setAttribute(Qt::WA_StyledBackground, true);
    infoCard->setStyleSheet(QStringLiteral(
        "#confirmInfoCard { background-color: #F4F8FC; border-radius: 14px; }"));
    auto *infoLayout = new QGridLayout(infoCard);
    infoLayout->setContentsMargins(22, 16, 22, 16);
    infoLayout->setHorizontalSpacing(18);
    infoLayout->setVerticalSpacing(12);

    // 每一行：字段名（右侧对齐灰字） + 值（左侧对齐深蓝加粗）
    const auto addRow = [infoLayout, infoCard](int row, const QString &key, QLabel *&value) {
        auto *keyLabel = new QLabel(key, infoCard);
        keyLabel->setStyleSheet(QStringLiteral(
            "color: #7A93B0; font-size: 16px; background: transparent;"));
        keyLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        value = new QLabel(infoCard);
        value->setStyleSheet(QStringLiteral(
            "color: #1F4E79; font-size: 18px; font-weight: bold; background: transparent;"));
        value->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        infoLayout->addWidget(keyLabel, row, 0);
        infoLayout->addWidget(value, row, 1);
    };
    addRow(0, QStringLiteral("医　生："), m_doctorValue);
    addRow(1, QStringLiteral("科　室："), m_deptValue);
    addRow(2, QStringLiteral("挂号日期："), m_dateValue);
    addRow(3, QStringLiteral("就诊时段："), m_sessionValue);
    infoLayout->setColumnStretch(1, 1);
    bodyLayout->addWidget(infoCard);

    bodyLayout->addStretch(1);

    /* ---------- 底部按钮行：取消 / 确认挂号 ---------- */
    auto *btnRow = new QHBoxLayout;
    btnRow->setSpacing(20);
    btnRow->addStretch(1);

    auto *cancelBtn = UIStyle::createOutlineButton(QStringLiteral("取　消"), body);
    cancelBtn->setFixedSize(160, 50);
    connect(cancelBtn, &QPushButton::clicked, this, [this] {
        emit cancelled();
        hide(); // 取消不破坏已选中的卡片，仅收起弹窗
    });

    auto *okBtn = UIStyle::createPrimaryButton(QStringLiteral("确认挂号"), body);
    okBtn->setFixedSize(190, 50);
    connect(okBtn, &QPushButton::clicked, this, [this] {
        emit confirmed(); // 由 AppointmentPage 槽函数准备医生信息
        hide();
    });

    btnRow->addWidget(cancelBtn);
    btnRow->addWidget(okBtn);
    btnRow->addStretch(1);
    bodyLayout->addLayout(btnRow);

    root->addWidget(body, 1);
}
