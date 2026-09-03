#include "registerwindow.h"
#include "aiassistantpopup.h"
#include "../core/iconfactory.h"
#include "../core/uistyle.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QLineEdit>
#include <QPoint>
#include <QPushButton>
#include <QSize>
#include <QVBoxLayout>

RegisterWindow::RegisterWindow(QWidget *parent)
    : QWidget(parent)
{
    // 作为 MainWindow 的子遮罩：不设窗口标志与固定尺寸，由父窗口统一覆盖
    setObjectName(QStringLiteral("registerWindow"));
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet(QStringLiteral(
        "#registerWindow {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "                                stop:0 #1E5799, stop:0.5 #2989D8, stop:1 #5FB4E8);"
        "}"));
    initUi();

}

QString RegisterWindow::phone() const
{
    return m_phoneEdit ? m_phoneEdit->text().trimmed() : QString();
}

void RegisterWindow::initUi()
{
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(64, 40, 64, 32);
    root->setSpacing(22);

    // ---------- 页面大标题：注册账号 ----------
    auto *title = new QLabel(QStringLiteral("注册账号"), this);
    UIStyle::stylePageTitle(title);
    root->addWidget(title);

    // ---------- 白色圆角卡片（布局与修改信息页一致） ----------
    auto *card = new QWidget(this);
    UIStyle::styleCard(card);
    root->addWidget(card, 1);

    auto *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(56, 36, 56, 32);
    cardLayout->setSpacing(18);

    // 卡片头部：标题 + 右侧【语音助手】按钮（与修改信息页一致）
    auto *headRow = new QHBoxLayout;
    headRow->setSpacing(16);

    auto *cardTitle = new QLabel(QStringLiteral("注册信息"), card);
    cardTitle->setStyleSheet(QStringLiteral(
        "color: #1F4E79; font-size: 26px; font-weight: bold; background: transparent;"));
    headRow->addWidget(cardTitle);
    headRow->addStretch(1);

    auto *assistantBtn = new QPushButton(QStringLiteral("  语音助手"), card);
    assistantBtn->setCursor(Qt::PointingHandCursor);
    assistantBtn->setFixedHeight(52);
    assistantBtn->setIcon(QIcon(IconFactory::renderSvg(IconFactory::micSvg(), QSize(26, 26))));
    assistantBtn->setIconSize(QSize(26, 26));
    assistantBtn->setStyleSheet(QStringLiteral(
        "QPushButton {"
        "    color: #FFFFFF;"
        "    background-color: #2E86DE;"
        "    border: none;"
        "    border-radius: 26px;"
        "    font-size: 18px;"
        "    font-weight: bold;"
        "    padding: 0 24px;"
        "}"
        "QPushButton:hover   { background-color: #4A9CE8; }"
        "QPushButton:pressed { background-color: #1F6FB8; }"));
    connect(assistantBtn, &QPushButton::clicked, this, &RegisterWindow::onVoiceAssistant);
    headRow->addWidget(assistantBtn);
    cardLayout->addLayout(headRow);

    // 分隔线
    auto *separator = new QFrame(card);
    separator->setFrameShape(QFrame::HLine);
    separator->setStyleSheet(QStringLiteral("color: #E3EAF3;"));
    separator->setFixedHeight(2);
    cardLayout->addWidget(separator);

    // 字段行构造器：左侧字段名 + 右侧输入框
    auto makeFieldRow = [card](const QString &name, QLineEdit *edit) -> QHBoxLayout * {
        auto *row = new QHBoxLayout;
        row->setSpacing(12);

        auto *lbl = new QLabel(name, card);
        lbl->setFixedWidth(120);
        lbl->setStyleSheet(QStringLiteral(
            "color: #7A93B0; font-size: 18px; background: transparent;"));
        row->addWidget(lbl);

        edit->setMinimumHeight(48);
        edit->setStyleSheet(QStringLiteral(
            "QLineEdit {"
            "    color: #1F4E79;"
            "    background-color: #F4F8FC;"
            "    border: 1px solid #C8D4E2;"
            "    border-radius: 12px;"
            "    padding: 0 16px;"
            "    font-size: 18px;"
            "}"));
        row->addWidget(edit, 1);
        return row;
    };

    // 姓名
    m_nameEdit = new QLineEdit(card);
    m_nameEdit->setPlaceholderText(QStringLiteral("请输入姓名"));
    cardLayout->addLayout(makeFieldRow(QStringLiteral("姓名"), m_nameEdit));

    // 手机号码
    m_phoneEdit = new QLineEdit(card);
    m_phoneEdit->setPlaceholderText(QStringLiteral("请输入11位手机号码"));
    m_phoneEdit->setMaxLength(11);
    cardLayout->addLayout(makeFieldRow(QStringLiteral("手机号码"), m_phoneEdit));

    // 密码（新增字段，掩码显示）
    m_passEdit = new QLineEdit(card);
    m_passEdit->setPlaceholderText(QStringLiteral("请输入密码"));
    m_passEdit->setEchoMode(QLineEdit::Password);
    cardLayout->addLayout(makeFieldRow(QStringLiteral("密码"), m_passEdit));

    // 医保卡号
    m_medcardEdit = new QLineEdit(card);
    m_medcardEdit->setPlaceholderText(QStringLiteral("请输入医保卡号"));
    cardLayout->addLayout(makeFieldRow(QStringLiteral("医保卡号"), m_medcardEdit));

    // 认证状态（只读）
    auto *statusRow = new QHBoxLayout;
    statusRow->setSpacing(12);
    auto *statusLbl = new QLabel(QStringLiteral("认证状态"), card);
    statusLbl->setFixedWidth(120);
    statusLbl->setStyleSheet(QStringLiteral(
        "color: #7A93B0; font-size: 18px; background: transparent;"));
    statusRow->addWidget(statusLbl);

    auto *statusValue = new QLabel(QStringLiteral("未认证"), card);
    statusValue->setStyleSheet(QStringLiteral(
        "color: #7A93B0; font-size: 18px; background: transparent;"));
    statusRow->addWidget(statusValue);
    statusRow->addStretch(1);
    cardLayout->addLayout(statusRow);

    // 错误提示
    m_errorLabel = new QLabel(card);
    m_errorLabel->setStyleSheet(QStringLiteral(
        "color: #E74C3C; font-size: 15px; background: transparent;"));
    m_errorLabel->hide();
    cardLayout->addWidget(m_errorLabel);

    cardLayout->addStretch(1);

    // ---------- 底部按钮：退出 / 确定注册 ----------
    auto *bottomRow = new QHBoxLayout;
    bottomRow->addStretch(1);
    auto *exitBtn = UIStyle::createOutlineButton(QStringLiteral("退出"), card);
    exitBtn->setFixedSize(180, 56);
    connect(exitBtn, &QPushButton::clicked, this, &RegisterWindow::exitRequested);
    bottomRow->addWidget(exitBtn);

    bottomRow->addSpacing(24);

    auto *confirmBtn = UIStyle::createPrimaryButton(QStringLiteral("确定注册"), card);
    confirmBtn->setFixedSize(220, 56);
    connect(confirmBtn, &QPushButton::clicked, this, &RegisterWindow::onConfirm);
    bottomRow->addWidget(confirmBtn);
    bottomRow->addStretch(1);
    cardLayout->addLayout(bottomRow);
}

void RegisterWindow::onConfirm()
{
    const QString name = m_nameEdit->text();
    const QString phone = m_phoneEdit->text().trimmed();
    const QString pwd = m_passEdit->text();
    const QString card=m_medcardEdit->text();


    if (name.isEmpty()) {
        m_errorLabel->setText(QStringLiteral("请输入姓名"));
        m_errorLabel->show();
        return;
    }
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
    //数据处理
    m_errorLabel->hide();
    QByteArray send_data;
    send_data.resize(1024);
    char *p=send_data.data();
    PATIENT_RESIGN_REQ req;
    MyUtils::qstringToCharArray(req.name, sizeof(req.name), name);
    MyUtils::qstringToCharArray(req.card, sizeof(req.card), card);
    MyUtils::qstringToCharArray(req.pwd, sizeof(req.pwd), pwd);
    MyUtils::qstringToCharArray(req.phone, sizeof(req.phone), phone);
    HEAD head;
    head.len=sizeof(req);
    head.type=SERVICE_TYPE::PATIENT_RESIGN;
    memcpy(p,&head,sizeof(head));
    memcpy(p+sizeof(HEAD),&req,sizeof(req));


    emit data_ready(send_data,sizeof(head)+sizeof(req));
    emit registerSucceeded();
}

void RegisterWindow::onVoiceAssistant()
{
    // 与修改信息页共用同一个 AI 信息助手（内部走 MyAgent 个人信息助手智能体），
    // 无边框、置顶、居中于本窗口，单实例复用
    if (m_aiPopup && m_aiPopup->isVisible()) {
        m_aiPopup->raise(); // 已打开则置顶
        m_aiPopup->activateWindow();
        return;
    }
    m_aiPopup = new AIAssistantPopup(
        UIStyle::resolveImagePath(QStringLiteral("icons/doctor2.jpg")), this);

    init_agent_connect();



    // 本窗口是 MainWindow 的子遮罩，geometry() 为相对父窗口坐标；
    // 弹窗是顶层窗口，需用顶层窗口的全局几何中心定位
    m_aiPopup->move(window()->geometry().center()
                    - QPoint(m_aiPopup->width() / 2, m_aiPopup->height() / 2));
    m_aiPopup->show();
    m_aiPopup->raise();
}

void RegisterWindow::init_agent_connect()
{
    qDebug()<<"init_agent_connect";
    connect(m_aiPopup, &QObject::destroyed, this, [this] { m_aiPopup = nullptr; });
    connect(m_aiPopup, &AIAssistantPopup::full_text, this, [this](int decision, QString content) {
        qDebug()<<"填";
        // 根据AI助手的决策码与内容，自动填充对应字段
        switch (decision) {
            case 1: // 填写手机号
                m_phoneEdit->setText(content);
                break;
            case 2: // 填写密码
                m_passEdit->setText(content);
                break;
            case 3: // 填写姓名
                m_nameEdit->setText(content);
                break;
            case 4: // 填写医保卡号
                m_medcardEdit->setText(content);
                break;
            default:
                break; // 无需填写
        }
    });
}
