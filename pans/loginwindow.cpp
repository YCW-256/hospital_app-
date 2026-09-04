#include "loginwindow.h"
#include "../core/uistyle.h"
#include "../MyTcp/cdata.h"
#include "topnavbar.h" // CircularLogo

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

LoginWindow::LoginWindow(QWidget *parent)
    : QWidget(parent)
{
    // 作为 MainWindow 的子遮罩：不设窗口标志与固定尺寸，由父窗口统一覆盖
    setObjectName(QStringLiteral("loginWindow"));
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet(QStringLiteral(
        "#loginWindow {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "                                stop:0 #1E5799, stop:0.5 #2989D8, stop:1 #5FB4E8);"
        "}"));
    initUi();
}

void LoginWindow::setPhone(const QString &phone)
{
    if (m_phoneEdit)
        m_phoneEdit->setText(phone);
}

void LoginWindow::initUi()
{
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    // ---------- 顶部右侧：关闭按钮（无边框窗口的退出入口） ----------
    auto *topBar = new QHBoxLayout;
    topBar->setContentsMargins(0, 16, 20, 0);
    topBar->addStretch(1);

    auto *closeBtn = new QPushButton(QStringLiteral("×"), this);
    closeBtn->setCursor(Qt::PointingHandCursor);
    closeBtn->setToolTip(QStringLiteral("关闭"));
    closeBtn->setFixedSize(40, 40);
    closeBtn->setStyleSheet(QStringLiteral(
        "QPushButton {"
        "    color: #FFFFFF;"
        "    background-color: transparent;"
        "    border: none;"
        "    border-radius: 20px;"
        "    font-size: 26px;"
        "}"
        "QPushButton:hover   { background-color: #E81123; color: #FFFFFF; }"
        "QPushButton:pressed { background-color: #B3071B; color: #FFFFFF; }"));
    // 登录为子遮罩，close() 只会隐藏自身；此处关闭顶层主窗口以退出程序
    connect(closeBtn, &QPushButton::clicked, this, [this] { window()->close(); });
    topBar->addWidget(closeBtn);
    root->addLayout(topBar);

    root->addStretch(2);

    // ---------- 白色圆角登录卡片 ----------
    auto *card = new QWidget(this);
    UIStyle::styleCard(card);
    card->setFixedSize(560, 560);
    root->addWidget(card, 0, Qt::AlignHCenter);

    auto *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(48, 32, 48, 32);
    cardLayout->setSpacing(12);

    // 顶部 LOGO + 名称 + 副标题
    auto *logo = new CircularLogo(card);
    logo->setFixedSize(64, 64);
    cardLayout->addWidget(logo, 0, Qt::AlignHCenter);

    auto *appName = new QLabel(QStringLiteral("医院自助终端"), card);
    appName->setAlignment(Qt::AlignCenter);
    appName->setStyleSheet(QStringLiteral(
        "color: #1F4E79; font-size: 28px; font-weight: bold; background: transparent;"));
    cardLayout->addWidget(appName);

    auto *subTitle = new QLabel(QStringLiteral("用户登录"), card);
    subTitle->setAlignment(Qt::AlignCenter);
    subTitle->setStyleSheet(QStringLiteral(
        "color: #7A93B0; font-size: 15px; background: transparent;"));
    cardLayout->addWidget(subTitle);

    cardLayout->addSpacing(8);

    // 字段行构造器：左侧字段名 + 右侧输入框
    auto makeFieldRow = [card](const QString &name, QLineEdit *edit,
                               bool isPassword) -> QHBoxLayout * {
        auto *row = new QHBoxLayout;
        row->setSpacing(12);

        auto *lbl = new QLabel(name, card);
        lbl->setFixedWidth(100);
        lbl->setStyleSheet(QStringLiteral(
            "color: #7A93B0; font-size: 18px; background: transparent;"));
        row->addWidget(lbl);

        edit->setMinimumHeight(52);
        if (isPassword)
            edit->setEchoMode(QLineEdit::Password);
        edit->setStyleSheet(QStringLiteral(
            "QLineEdit {"
            "    color: #1F4E79;"
            "    background-color: #F4F8FC;"
            "    border: 1px solid #C8D4E2;"
            "    border-radius: 12px;"
            "    padding: 0 16px;"
            "    font-size: 18px;"
            "}"
            "QLineEdit:focus { border: 2px solid #2E86DE; }"));
        row->addWidget(edit, 1);
        return row;
    };

    m_phoneEdit = new QLineEdit(card);
    m_phoneEdit->setPlaceholderText(QStringLiteral("请输入11位手机号码"));
    m_phoneEdit->setMaxLength(11);
    cardLayout->addLayout(makeFieldRow(QStringLiteral("手机号码"), m_phoneEdit, false));

    m_passEdit = new QLineEdit(card);
    m_passEdit->setPlaceholderText(QStringLiteral("请输入密码"));
    cardLayout->addLayout(makeFieldRow(QStringLiteral("密码"), m_passEdit, true));

    // 登录错误提示
    m_errorLabel = new QLabel(card);
    m_errorLabel->setStyleSheet(QStringLiteral(
        "color: #E74C3C; font-size: 15px; background: transparent;"));
    m_errorLabel->hide();
    cardLayout->addWidget(m_errorLabel);

    cardLayout->addSpacing(4);

    // 登录按钮
    loginBtn = UIStyle::createPrimaryButton(QStringLiteral("登  录"), card);
    loginBtn->setFixedHeight(56);
    connect(loginBtn, &QPushButton::clicked, this, &LoginWindow::onLogin);
    connect(m_passEdit, &QLineEdit::returnPressed, this, &LoginWindow::onLogin);
    cardLayout->addWidget(loginBtn);

    // 分隔线 + 注册入口
    auto *separator = new QFrame(card);
    separator->setFrameShape(QFrame::HLine);
    separator->setStyleSheet(QStringLiteral("color: #E3EAF3;"));
    cardLayout->addWidget(separator);

    auto *bottomRow = new QHBoxLayout;
    bottomRow->addStretch(1);
    auto *hint = new QLabel(QStringLiteral("还没有账号？"), card);
    hint->setStyleSheet(QStringLiteral(
        "color: #7A93B0; font-size: 16px; background: transparent;"));
    bottomRow->addWidget(hint);
    auto *regBtn = new QPushButton(QStringLiteral("立即注册"), card);
    regBtn->setCursor(Qt::PointingHandCursor);
    regBtn->setStyleSheet(QStringLiteral(
        "QPushButton { color: #2E86DE; background: transparent; border: none;"
        "              font-size: 16px; font-weight: bold; }"
        "QPushButton:hover { color: #1F6FB8; }"));
    connect(regBtn, &QPushButton::clicked, this, &LoginWindow::registerRequested);
    bottomRow->addWidget(regBtn);
    bottomRow->addStretch(1);
    cardLayout->addLayout(bottomRow);

    root->addStretch(3);
}

void LoginWindow::onLogin()
{
    const QString phone = m_phoneEdit->text();
    const QString pwd = m_passEdit->text();

    if (phone.length() != 11) {
        m_errorLabel->setText(QStringLiteral("请输入正确的11位手机号码"));
        m_errorLabel->show();
        return;
    }
    if (pwd.isEmpty()) {
        m_errorLabel->setText(QStringLiteral("请输入密码"));
        m_errorLabel->show();
        return;
    }
    m_errorLabel->hide();
    // 记录本次登录手机号到 CData，供个人中心页展示当前登录用户
    CData::m_phone = phone;
    PATIENT_LOGIN_REQ req;
    req.type=1;
    MyUtils::qstringToCharArray(req.account,sizeof(req.account),phone);
    MyUtils::qstringToCharArray(req.pwd,sizeof(req.pwd),pwd);
    HEAD head;
    head.len=sizeof(req);
    head.type=SERVICE_TYPE::PATIENT_LOGIN;
    QByteArray data;
    data.resize(1024);
    memcpy(data.data(),&head,sizeof(head));
    memcpy(data.data()+sizeof(head),&req,sizeof(req));

    emit data_ready(data,sizeof(head)+sizeof(req));


    //emit loginSucceeded();
}
