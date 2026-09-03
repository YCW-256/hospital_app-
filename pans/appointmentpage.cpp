#include "appointmentpage.h"
#include "../core/uistyle.h"
#include "childs/appointmentconfirmpopup.h"
#include "childs/doctorcard.h"

#include <QButtonGroup>
#include <QDate>
#include <QDebug>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QHideEvent>
#include <QLabel>
#include <QPoint>
#include <QPushButton>
#include <QScrollArea>
#include <QStringList>
#include <QVBoxLayout>
#include "../MyTcp/cdata.h"
#include "../MyTcp/protecol.h"
namespace {
/* 左侧/右侧区域标题样式 */
void styleSectionHeader(QLabel *label)
{
    label->setAlignment(Qt::AlignCenter);
    label->setStyleSheet(QStringLiteral(
        "background-color: #EAF2FB;"
        "color: #1F4E79;"
        "border-radius: 10px;"
        "padding: 12px;"
        "font-size: 18px;"
        "font-weight: bold;"));
}
} // namespace

AppointmentPage::AppointmentPage(QWidget *parent)
    : QWidget(parent)
{
    initLayout();
}

void AppointmentPage::getDoctorInfo()
{
    PATIENT_GET_DOCTOR_REQ req;
    req.id=CData::m_id;
    HEAD head;
    head.len=sizeof(req);
    head.type=SERVICE_TYPE::PATIENT_GET_DOCTOR_INFO;
    QByteArray data;
    data.resize(1024);
    memcpy(data.data(),&head,sizeof(head));
    memcpy(data.data()+sizeof(head),&req,sizeof(req));
    emit data_ready(data,sizeof(head)+sizeof(req));
}

void AppointmentPage::flush()
{
    if (!m_doctorGrid)
        return; // 布局尚未构建完成

    // 1) 清空旧卡片
    for (DoctorInfoCard *c : m_doctorCards) {
        m_doctorGrid->removeWidget(c);
        c->deleteLater();
    }
    m_doctorCards.clear();
    m_selectedDoctor = nullptr;

    // 2) 当前筛选条件：科室 + 时段
    const QString dept = currentDept();          // 空串 = 不限科室
    const int sessionTime = currentSessionTime(); // -1 = 不限时段

    // 3) 遍历 CData::m_doctor_info，筛出符合条件的医生并重建卡片
    int row = 0, col = 0;
    for (const doctor_info_use &info : CData::m_doctor_info) {
        if (!dept.isEmpty() && info.department != dept)
            continue;
        if (sessionTime >= 0 && info.time != sessionTime)
            continue;

        auto *dc = new DoctorInfoCard(info.id, info.name, info.department, info.time,
                                      m_avatarPath, m_doctorContent);
        m_doctorGrid->addWidget(dc, row, col);
        m_doctorCards.append(dc);

        if (++col == 3) {
            col = 0;
            ++row;
        }
    }
    // 三列均分宽度
    for (int c = 0; c < 3; ++c)
        m_doctorGrid->setColumnStretch(c, 1);

    // 4) 卡片互斥选中 + 点击输出所选医生的完整信息（doctor_info_use 各属性），
    //    并弹出挂号确认弹窗（展示该医生的挂号信息，确认后触发 onAppointmentConfirmed）
    for (DoctorInfoCard *dc : m_doctorCards) {
        connect(dc, &DoctorInfoCard::clicked, this, [this, dc] {
            for (DoctorInfoCard *c : m_doctorCards)
                c->setSelected(c == dc);
            m_selectedDoctor = dc; // 记录当前选中的医生，供挂号确认提交使用
            qDebug().noquote() << QStringLiteral("选择医生：id=%1 姓名=%2 科室=%3 时段=%4")
                                      .arg(dc->id())
                                      .arg(dc->name())
                                      .arg(dc->dept())
                                      .arg(dc->time());
            showDoctorConfirm(dc); // 点击医生卡片 → 弹出确认挂号弹窗
        });
    }
}

void AppointmentPage::showDoctorConfirm(DoctorInfoCard *dc)
{
    if (!dc)
        return;
    m_selectedDoctor = dc; // 待确认医生（供确认槽函数读取）

    if (!m_confirmPopup) {
        // 弹窗父对象取主窗口（AppointmentPage 所在顶层窗口），复用单实例
        m_confirmPopup = new AppointmentConfirmPopup(window());
        // 弹窗【确认挂号】→ 本页槽函数：准备医生信息写入 CData
        connect(m_confirmPopup, &AppointmentConfirmPopup::confirmed,
                this, &AppointmentPage::onAppointmentConfirmed);
    }

    // 填充本次待确认医生的挂号信息
    m_confirmPopup->setAppointmentInfo(dc->name(), dc->dept(), dc->time());

    // 弹窗固定悬浮在主窗口正中央
    QWidget *host = m_confirmPopup->parentWidget(); // 主窗口
    const QPoint center = host ? host->rect().center()
                               : rect().center();
    m_confirmPopup->move(center - QPoint(m_confirmPopup->width() / 2-300,
                                         m_confirmPopup->height() / 2));
    m_confirmPopup->show();
    m_confirmPopup->raise();
}

void AppointmentPage::onAppointmentConfirmed()
{
    if (!m_selectedDoctor) {
        qDebug() << QStringLiteral("确认挂号：未选中医生，忽略");
        return;
    }
    // 把确认挂号的医生信息保存到 CData，供后续挂号提交流程使用
    auto id = m_selectedDoctor->id();
    auto name = m_selectedDoctor->name();
    auto department = m_selectedDoctor->dept();
    auto time = m_selectedDoctor->time();
    auto m_register_date = QDate::currentDate().toString(QStringLiteral("yyyy-MM-dd"));

    qDebug().noquote() << QStringLiteral("确认挂号（医生信息已写入 CData）：id=%1 姓名=%2 科室=%3 时段=%4 日期=%5")
                              .arg(id)
                              .arg(name)
                              .arg(department)
                              .arg(time)
                              .arg(m_register_date);
    //发送数据
    QByteArray send_data;
    send_data.resize(1024);
    HEAD head;
    PATIENT_APPOINTMENT_REQ req;
    head.type=SERVICE_TYPE::PATIENT_APPOINTMENT;
    head.len=sizeof(req);
    req.patient_id=CData::m_id;
    req.doctor_id=id;
    req.ob_time=time;

    memcpy(send_data.data(),&head,sizeof(head));
    memcpy(send_data.data()+sizeof(head),&req,sizeof(req));
    emit data_ready(send_data,sizeof(head)+sizeof(req));





}

void AppointmentPage::hideEvent(QHideEvent *event)
{
    // 页面切走（隐藏）时同步收起挂号确认弹窗，避免其仍悬浮在主窗口上
    if (m_confirmPopup && m_confirmPopup->isVisible())
        m_confirmPopup->hide();
    QWidget::hideEvent(event);
}

QString AppointmentPage::currentDept() const
{
    if (m_deptGroup && m_deptGroup->checkedButton())
        return m_deptGroup->checkedButton()->text();
    return QString(); // 未选择科室 → 不限科室
}

int AppointmentPage::currentSessionTime() const
{
    if (m_sessionGroup && m_sessionGroup->checkedButton()) {
        const QString text = m_sessionGroup->checkedButton()->text();
        if (text == QStringLiteral("下午"))
            return 1; // doctor_info_use.time：1=下午
        if (text == QStringLiteral("上午"))
            return 0; // doctor_info_use.time：0=上午
    }
    return -1; // 未选择时段 → 不限时段
}

void AppointmentPage::initLayout()
{
    UIStyle::styleTransparentPage(this); // 透出主窗口蓝色渐变
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(64, 40, 64, 32);
    root->setSpacing(22);

    /* ---------- 页面顶部：返回按钮 + 大标题 ---------- */
    auto *titleRow = new QHBoxLayout;
    titleRow->setSpacing(18);

    auto *backBtn = UIStyle::createOutlineButton(QStringLiteral("← 返回"), this);
    backBtn->setFixedSize(150, 52);
    connect(backBtn, &QPushButton::clicked, this, &AppointmentPage::backRequested);
    titleRow->addWidget(backBtn);

    auto *title = new QLabel(QStringLiteral("预约挂号"), this);
    UIStyle::stylePageTitle(title);
    titleRow->addWidget(title);
    titleRow->addStretch(1);
    root->addLayout(titleRow);

    /* ---------- 白色圆角卡片 ---------- */
    auto *card = new QWidget(this);
    UIStyle::styleCard(card);
    root->addWidget(card, 1);

    // 卡片整体纵向布局：上部左右分区 + 底部确认按钮
    auto *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(36, 30, 36, 24);
    cardLayout->setSpacing(22);

    auto *regions = new QHBoxLayout;
    regions->setSpacing(28);

    /* ---------- 左侧：选择科室 ---------- */
    auto *leftLayout = new QVBoxLayout;
    leftLayout->setSpacing(14);

    auto *leftHeader = new QLabel(QStringLiteral("选择科室"), card);
    styleSectionHeader(leftHeader);
    leftLayout->addWidget(leftHeader);

    // 科室列表（2 列网格），点击后打印"选择XXX科"
    const QStringList depts = {
        QStringLiteral("内科"),     QStringLiteral("外科"),
        QStringLiteral("儿科"),     QStringLiteral("妇产科"),
        QStringLiteral("骨科"),     QStringLiteral("眼科"),
        QStringLiteral("耳鼻喉科"), QStringLiteral("皮肤科"),
    };
    auto *deptGrid = new QGridLayout;
    deptGrid->setSpacing(12);

    m_deptGroup = new QButtonGroup(this);
    m_deptGroup->setExclusive(true); // 同一时间只选中一个科室
    for (int i = 0; i < depts.size(); ++i) {
        auto *deptBtn = new QPushButton(depts.at(i), card);
        deptBtn->setCursor(Qt::PointingHandCursor);
        deptBtn->setCheckable(true);
        deptBtn->setMinimumHeight(48);
        deptBtn->setStyleSheet(QStringLiteral(
            "QPushButton {"
            "    color: #1F4E79;"
            "    background-color: #FFFFFF;"
            "    border: 1px solid #C8D4E2;"
            "    border-radius: 12px;"
            "    font-size: 16px;"
            "}"
            "QPushButton:hover { background-color: #EAF2FB; }"
            "QPushButton:checked {"
            "    background-color: #2E86DE;"
            "    color: #FFFFFF;"
            "    border-color: #2E86DE;"
            "}"));
        m_deptGroup->addButton(deptBtn, i);
        deptGrid->addWidget(deptBtn, i / 2, i % 2);

        // 点击科室：打印"选择XXX科"
        connect(deptBtn, &QPushButton::clicked, this, [dept = depts.at(i)] {
            qDebug().noquote() << QStringLiteral("选择%1").arg(dept);
        });
        // 每次科室切换（toggled）都按当前条件重新筛选医生卡片
        connect(deptBtn, &QPushButton::toggled, this,
                [this](bool checked) { if (checked) flush(); });
        deptBtn->setChecked(i == 0); // 默认选中第一个科室：内科
    }
    leftLayout->addLayout(deptGrid);
    leftLayout->addStretch(1);
    regions->addLayout(leftLayout, 1);

    /* ---------- 左右分隔竖线 ---------- */
    auto *separator = new QFrame(card);
    separator->setFrameShape(QFrame::VLine);
    separator->setStyleSheet(QStringLiteral("color: #E3EAF3;"));
    separator->setFixedWidth(2);
    regions->addWidget(separator);

    /* ---------- 右侧：排班时段选择 + 医生选择 ---------- */
    auto *rightLayout = new QVBoxLayout;
    rightLayout->setSpacing(12);

    // 排班时段：上午 / 下午（互斥可选中）
    auto *sessionRow = new QHBoxLayout;
    sessionRow->setSpacing(12);

    auto *sessionLabel = new QLabel(QStringLiteral("选择时段"), card);
    sessionLabel->setStyleSheet(QStringLiteral(
        "color: #1F4E79; font-size: 16px; font-weight: bold; background: transparent;"));
    sessionRow->addWidget(sessionLabel);

    m_sessionGroup = new QButtonGroup(this);
    m_sessionGroup->setExclusive(true);

    auto *amBtn = new QPushButton(QStringLiteral("上午"), card);
    auto *pmBtn = new QPushButton(QStringLiteral("下午"), card);
    const QString sessionStyle = QStringLiteral(
        "QPushButton {"
        "    color: #1F4E79;"
        "    background-color: #FFFFFF;"
        "    border: 1px solid #C8D4E2;"
        "    border-radius: 10px;"
        "    font-size: 15px;"
        "    padding: 6px 22px;"
        "}"
        "QPushButton:hover { background-color: #EAF2FB; }"
        "QPushButton:checked {"
        "    background-color: #2E86DE;"
        "    color: #FFFFFF;"
        "    border-color: #2E86DE;"
        "}");
    amBtn->setCursor(Qt::PointingHandCursor);
    pmBtn->setCursor(Qt::PointingHandCursor);
    amBtn->setCheckable(true);
    pmBtn->setCheckable(true);
    amBtn->setStyleSheet(sessionStyle);
    pmBtn->setStyleSheet(sessionStyle);
    m_sessionGroup->addButton(amBtn);
    m_sessionGroup->addButton(pmBtn);
    amBtn->setChecked(true); // 默认选择上午
    connect(amBtn, &QPushButton::clicked, this, [] {
        qDebug().noquote() << QStringLiteral("选择上午");
    });
    connect(pmBtn, &QPushButton::clicked, this, [] {
        qDebug().noquote() << QStringLiteral("选择下午");
    });
    // 每次时段切换（toggled）都按当前条件重新筛选医生卡片
    connect(amBtn, &QPushButton::toggled, this,
            [this](bool checked) { if (checked) flush(); });
    connect(pmBtn, &QPushButton::toggled, this,
            [this](bool checked) { if (checked) flush(); });
    sessionRow->addWidget(amBtn);
    sessionRow->addWidget(pmBtn);
    sessionRow->addStretch(1);
    rightLayout->addLayout(sessionRow);

    // 医生选择标题
    auto *doctorHeader = new QLabel(QStringLiteral("选择医生"), card);
    styleSectionHeader(doctorHeader);
    rightLayout->addWidget(doctorHeader);

    // 医生卡片滚动区：内容容器 + 3 列网格；具体卡片由 flush() 依据当前条件动态生成
    m_doctorContent = new QWidget;
    m_doctorContent->setAutoFillBackground(false);

    // 医生头像暂统一为测试头像（后续可接入真实头像）
    m_avatarPath = UIStyle::resolveImagePath(QStringLiteral("icons/doctor2.jpg"));

    m_doctorGrid = new QGridLayout(m_doctorContent);
    m_doctorGrid->setContentsMargins(2, 2, 6, 2);
    m_doctorGrid->setSpacing(14);
    m_doctorGrid->setAlignment(Qt::AlignTop);
    // 三列均分宽度
    for (int col = 0; col < 3; ++col)
        m_doctorGrid->setColumnStretch(col, 1);

    auto *scroll = new QScrollArea(card);
    scroll->setObjectName(QStringLiteral("doctorScroll"));
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll->viewport()->setAutoFillBackground(false);
    scroll->setStyleSheet(QStringLiteral(
        "QScrollArea { background: transparent; border: none; }"
        "QScrollBar:vertical { background: transparent; width: 10px; margin: 0; }"
        "QScrollBar::handle:vertical { background: #C8D4E2; border-radius: 5px; min-height: 40px; }"
        "QScrollBar::handle:vertical:hover { background: #2E86DE; }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }"
        "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { background: transparent; }"));
    scroll->setWidget(m_doctorContent);
    rightLayout->addWidget(scroll, 1);

    regions->addLayout(rightLayout, 2);

    // 左右分区装入卡片主体
    cardLayout->addLayout(regions, 1);

    /* ---------- 卡片底部居中：【确认预约】白色边框圆角按钮 ---------- */
    auto *confirmRow = new QHBoxLayout;
    confirmRow->addStretch(1);
    auto *confirmBtn = UIStyle::createOutlineButton(QStringLiteral("确认预约"), card);
    confirmBtn->setFixedSize(220, 54);
    connect(confirmBtn, &QPushButton::clicked, this, &AppointmentPage::confirmRequested);
    confirmRow->addWidget(confirmBtn);
    confirmRow->addStretch(1);
    cardLayout->addLayout(confirmRow);
}
