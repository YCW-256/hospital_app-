#include "mainwindow.h"
#include "aiassistantpopup.h"
#include "aiconsultpage.h"
#include "appointmentpage.h"
#include "feepage.h"
#include "functionbutton.h"
#include "../core/iconfactory.h"
#include "loginwindow.h"
#include "modifyinfopage.h"
#include "paymentpage.h"
#include "personalcenterpage.h"
#include "registerwindow.h"
#include "topnavbar.h"
#include "../core/uistyle.h"

#include <QGridLayout>
#include <QLabel>
#include <QList>
#include <QPoint>
#include <QPushButton>
#include <QResizeEvent>
#include <QStackedWidget>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle(QStringLiteral("医院自助终端"));

    // 无边框窗口
    setWindowFlags(Qt::FramelessWindowHint | Qt::Window);
    // 自助终端分辨率固定为 1280x800
    setFixedSize(1280, 800);
    m_socket=nullptr;
    m_socket=new SocketLink;

    if(m_socket)
        m_socket->connectHost();
    else{
        qDebug()<<"m_socket创建异常";
    }

    initCentralUi();
    initStackedPages();
    enableLogin();
    init_data_connect();
}

MainWindow::~MainWindow() = default;

void MainWindow::enableLogin()
{
    if (m_loginEnabled)
        return;
    m_loginEnabled = true;

    // 登录/注册作为主窗口的子遮罩（不加入 QStackedWidget），覆盖整个主窗口
    m_loginWindow = new LoginWindow(this);
    m_registerWindow = new RegisterWindow(this);
    m_loginWindow->setGeometry(rect());
    m_registerWindow->setGeometry(rect());

    // 登录成功 → 隐藏登录遮罩，露出主界面
    connect(m_loginWindow, &LoginWindow::loginSucceeded, this, [this] {
        m_loginWindow->hide();
    });

    // 登录页 ↔ 注册页互切
    connect(m_loginWindow, &LoginWindow::registerRequested, this, [this] {
        m_loginWindow->hide();
        m_registerWindow->show();
        m_registerWindow->raise();
    });
    connect(m_registerWindow, &RegisterWindow::exitRequested, this, [this] {
        m_registerWindow->hide();
        m_loginWindow->show();
        m_loginWindow->raise();
    });
    connect(m_registerWindow, &RegisterWindow::registerSucceeded, this, [this] {
        m_loginWindow->setPhone(m_registerWindow->phone()); // 注册手机号带回登录页预填
        m_registerWindow->hide();
        m_loginWindow->show();
        m_loginWindow->raise();
    });

    // 启动即显示登录遮罩
    m_loginWindow->show();
    m_loginWindow->raise();
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    // 遮罩随主窗口尺寸同步覆盖（主窗口固定 1280×800，此处作健壮性处理）
    if (m_loginWindow)
        m_loginWindow->setGeometry(rect());
    if (m_registerWindow)
        m_registerWindow->setGeometry(rect());
}

void MainWindow::init_data_connect()
{
    //注册
    connect(m_registerWindow,&RegisterWindow::data_ready,m_socket,&SocketLink::send_data);
    //登录
    connect(m_loginWindow,&LoginWindow::data_ready,m_socket,&SocketLink::send_data);
    connect(m_socket,&SocketLink::login_success,this,[this](){
        this->m_loginWindow->hide();
        this->m_registerWindow->hide();
    });
    //挂号
    connect(appointmentPage,&AppointmentPage::data_ready,m_socket,&SocketLink::send_data);
    //医生信息拉取完成（CData::m_doctor_info 就绪）→ 刷新预约挂号页的医生卡片
    connect(m_socket, &SocketLink::get_doctor_info_success, appointmentPage, &AppointmentPage::flush);

    connect(aiConsultPage,&AIConsultPage::data_ready,m_socket,&SocketLink::send_data);

}

void MainWindow::initCentralUi()
{
    // ---------- 中央容器：蓝色渐变背景 ----------
    auto *central = new QWidget(this);
    central->setObjectName(QStringLiteral("centralWidget"));
    central->setAttribute(Qt::WA_StyledBackground, true);
    central->setStyleSheet(QStringLiteral(
        "#centralWidget {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "                                stop:0 #1E5799, stop:0.5 #2989D8, stop:1 #5FB4E8);"
        "}"));
    setCentralWidget(central);

    // ---------- 垂直布局：上方导航栏 + 下方页面堆栈 ----------
    auto *rootLayout = new QVBoxLayout(central);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    m_navBar = new TopNavBar(central);
    rootLayout->addWidget(m_navBar);

    m_stack = new QStackedWidget(central);
    m_stack->setObjectName(QStringLiteral("stackedWidget"));
    m_stack->setAttribute(Qt::WA_StyledBackground, true);
    m_stack->setStyleSheet(QStringLiteral(
        "#stackedWidget { background: transparent; }"));
    rootLayout->addWidget(m_stack, 1);

    // 导航栏标题随页面切换
    connectNavTitle();

    // 点击导航栏左上角 LOGO 进入个人中心（索引 6）
    connect(m_navBar, &TopNavBar::logoClicked, this, [this] {
        m_stack->setCurrentIndex(6);
    });
}

void MainWindow::initStackedPages()
{
    // 第 0 页：首页（2 行 3 列功能按钮）
    m_stack->addWidget(createHomePage());

    // 第 1 页：待缴费用清单
    auto *paymentPage = new PaymentPage(this);
    connect(paymentPage, &PaymentPage::backRequested, this,
            [this] { m_stack->setCurrentIndex(0); });
    m_stack->addWidget(paymentPage);

    // 第 2 页：预约挂号
    appointmentPage = new AppointmentPage(this);
    connect(appointmentPage, &AppointmentPage::backRequested, this,
            [this] {

                    m_stack->setCurrentIndex(0);

    });

    m_stack->addWidget(appointmentPage);

    // 第 3 页：费用查询
    auto *feePage = new FeeQueryPage(this);
    connect(feePage, &FeeQueryPage::backRequested, this,
            [this] { m_stack->setCurrentIndex(0); });
    m_stack->addWidget(feePage);

    // 第 4 页：AI 快速问诊（原"药费查询"占位位，首页【药费查询】按钮进入）
    aiConsultPage = new AIConsultPage(this);
    connect(aiConsultPage, &AIConsultPage::backRequested, this,
            [this] { m_stack->setCurrentIndex(0); });
    m_stack->addWidget(aiConsultPage);

    // 第 5 页：尚未开发的功能占位页（门诊充值）
    m_stack->addWidget(createPlaceholderPage(QStringLiteral("门诊充值")));

    // 第 6 页：个人中心（由首页【个人中心】按钮 / 导航栏 LOGO 进入）
    auto *personalPage = new PersonalCenterPage(this);
    connect(personalPage, &PersonalCenterPage::backRequested, this,
            [this] { m_stack->setCurrentIndex(0); });
    // 【修改信息】进入修改信息页（索引 7）
    connect(personalPage, &PersonalCenterPage::modifyRequested, this,
            [this] { m_stack->setCurrentIndex(7); });
    m_stack->addWidget(personalPage);

    // 第 7 页：修改信息
    auto *modifyPage = new ModifyInfoPage(this);
    connect(modifyPage, &ModifyInfoPage::backRequested, this,
            [this] { m_stack->setCurrentIndex(6); }); // 返回个人中心
    // 【语音助手】弹出悬浮 AI 信息助手（置顶，居中于主窗口，单实例）
    connect(modifyPage, &ModifyInfoPage::voiceAssistantRequested, this, [this] {
        if (m_aiPopup && m_aiPopup->isVisible()) {
            m_aiPopup->raise(); // 已打开则置顶
            m_aiPopup->activateWindow();
            return;
        }
        m_aiPopup = new AIAssistantPopup(
            UIStyle::resolveImagePath(QStringLiteral("icons/doctor2.jpg")), this);
        connect(m_aiPopup, &QObject::destroyed, this, [this] { m_aiPopup = nullptr; });
        m_aiPopup->move(geometry().center() - QPoint(m_aiPopup->width() / 2, m_aiPopup->height() / 2));
        m_aiPopup->show();
        m_aiPopup->raise();
    });
    m_stack->addWidget(modifyPage);

    m_stack->setCurrentIndex(0); // 默认显示首页
}

void MainWindow::connectNavTitle()
{
    // 各页面在导航栏中间显示的标题
    static const QStringList titles = {
        QStringLiteral("自助终端"),      // 0 首页
        QStringLiteral("待缴费用清单"),  // 1 门诊缴费
        QStringLiteral("预约挂号"),      // 2 预约挂号
        QStringLiteral("费用查询"),      // 3 费用查询
        QStringLiteral("AI 快速问诊"),   // 4 AI 快速问诊（原"药费查询"占位位）
        QStringLiteral("门诊充值"),      // 5 占位
        QStringLiteral("个人中心"),      // 6 个人中心
        QStringLiteral("修改信息"),      // 7 修改信息
    };
    connect(m_stack, &QStackedWidget::currentChanged, this,
            [this](int index) { m_navBar->setCenterText(titles.value(index, titles.first())); });
}

/* ==================== 第 0 页：首页 ==================== */

QWidget *MainWindow::createHomePage()
{
    auto *home = new QWidget(this);
    home->setObjectName(QStringLiteral("homePage"));
    home->setAttribute(Qt::WA_StyledBackground, true);
    home->setStyleSheet(QStringLiteral(
        "#homePage { background: transparent; }"));

    // 2 行 3 列网格布局
    auto *grid = new QGridLayout(home);
    grid->setContentsMargins(64, 52, 64, 56);
    grid->setHorizontalSpacing(40);
    grid->setVerticalSpacing(40);

    // 六个功能按钮：名称 / 图标 / 背景色
    const QStringList names = {
        QStringLiteral("预约挂号"),
        QStringLiteral("门诊缴费"),
        QStringLiteral("费用查询"),
        QStringLiteral("个人中心"),  // 原"自助发卡"改为个人中心
        QStringLiteral("药费查询"),
        QStringLiteral("门诊充值"),
    };
    const QStringList colors = {
        QStringLiteral("#D9772B"), // 棕橙
        QStringLiteral("#2E86DE"), // 蓝
        QStringLiteral("#9B59B6"), // 紫
        QStringLiteral("#5E2E91"), // 深紫
        QStringLiteral("#1FA99A"), // 青绿
        QStringLiteral("#7C6BD4"), // 蓝紫
    };
    const QStringList svgs = {
        IconFactory::appointmentSvg(),
        IconFactory::paymentSvg(),
        IconFactory::feeQuerySvg(),
        IconFactory::personSvg(),  // 个人中心
        IconFactory::medicineQuerySvg(),
        IconFactory::rechargeSvg(),
    };

    // 六个按钮对应的 stack 页面索引：
    // 预约挂号→2、门诊缴费→1、费用查询→3、个人中心→6、
    // 药费查询→4（AI 快速问诊页）、门诊充值→5
    const QList<int> targetIndexes = { 2, 1, 3, 6, 4, 5 };

    for (int i = 0; i < names.size(); ++i) {
        auto *btn = new RoundedFunctionButton(names.at(i), svgs.at(i), colors.at(i), home);
        grid->addWidget(btn, i / 3, i % 3);

        // 点击后切换到对应的功能子页面
        if(i==0){
            connect(btn, &QPushButton::clicked, this, [this, i, targetIndexes] {
                m_stack->setCurrentIndex(targetIndexes.at(i));
                this->appointmentPage->getDoctorInfo();
            });
        }
        else{
        connect(btn, &QPushButton::clicked, this, [this, i, targetIndexes] {
            m_stack->setCurrentIndex(targetIndexes.at(i));
        });

        }
    }

    return home;
}

/* ==================== 功能占位页 ==================== */

QWidget *MainWindow::createPlaceholderPage(const QString &title)
{
    auto *page = new QWidget(this);
    page->setObjectName(QStringLiteral("placeholderPage"));
    page->setAttribute(Qt::WA_StyledBackground, true);
    page->setStyleSheet(QStringLiteral(
        "#placeholderPage { background: transparent; }"));

    auto *layout = new QVBoxLayout(page);
    layout->setContentsMargins(40, 26, 40, 40);

    // 返回首页按钮（半透明白色胶囊样式）
    auto *backBtn = new QPushButton(QStringLiteral("← 返回首页"), page);
    backBtn->setCursor(Qt::PointingHandCursor);
    backBtn->setFixedSize(140, 44);
    backBtn->setStyleSheet(QStringLiteral(
        "QPushButton {"
        "    color: #FFFFFF;"
        "    background-color: rgba(255,255,255,0.18);"
        "    border: 1px solid rgba(255,255,255,0.55);"
        "    border-radius: 22px;"
        "    font-size: 16px;"
        "}"
        "QPushButton:hover   { background-color: rgba(255,255,255,0.32); }"
        "QPushButton:pressed { background-color: rgba(255,255,255,0.45); }"));
    connect(backBtn, &QPushButton::clicked, this, [this] {
        m_stack->setCurrentIndex(0); // 回到首页
    });
    layout->addWidget(backBtn, 0, Qt::AlignLeft);

    layout->addStretch(1);

    // 占位提示文字
    auto *label = new QLabel(QStringLiteral("%1\n\n页面建设中，敬请期待…").arg(title), page);
    label->setAlignment(Qt::AlignCenter);
    label->setStyleSheet(QStringLiteral(
        "color: #FFFFFF; font-size: 30px; font-weight: bold; background: transparent;"));
    layout->addWidget(label, 0, Qt::AlignCenter);

    layout->addStretch(2);

    return page;
}
