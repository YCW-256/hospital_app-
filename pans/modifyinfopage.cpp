#include "modifyinfopage.h"
#include "../core/iconfactory.h"
#include "../core/uistyle.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

ModifyInfoPage::ModifyInfoPage(QWidget *parent)
    : QWidget(parent)
{
    initLayout();
}

void ModifyInfoPage::initLayout()
{
    UIStyle::styleTransparentPage(this);
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(64, 40, 64, 32);
    root->setSpacing(22);

    /* ---------- 顶部：返回按钮 + 大标题 ---------- */
    auto *titleRow = new QHBoxLayout;
    titleRow->setSpacing(18);

    auto *backBtn = UIStyle::createOutlineButton(QStringLiteral("← 返回"), this);
    backBtn->setFixedSize(150, 52);
    connect(backBtn, &QPushButton::clicked, this, &ModifyInfoPage::backRequested);
    titleRow->addWidget(backBtn);

    auto *title = new QLabel(QStringLiteral("修改信息"), this);
    UIStyle::stylePageTitle(title);
    titleRow->addWidget(title);
    titleRow->addStretch(1);
    root->addLayout(titleRow);

    /* ---------- 白色圆角卡片 ---------- */
    auto *card = new QWidget(this);
    UIStyle::styleCard(card);
    root->addWidget(card, 1);

    auto *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(56, 36, 56, 32);
    cardLayout->setSpacing(18);

    // 卡片头部：标题 + 右侧【语音助手】按钮
    auto *headRow = new QHBoxLayout;
    headRow->setSpacing(16);

    auto *cardTitle = new QLabel(QStringLiteral("个人资料"), card);
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
    connect(assistantBtn, &QPushButton::clicked, this, &ModifyInfoPage::voiceAssistantRequested);
    headRow->addWidget(assistantBtn);
    cardLayout->addLayout(headRow);

    // 分隔线
    auto *separator = new QFrame(card);
    separator->setFrameShape(QFrame::HLine);
    separator->setStyleSheet(QStringLiteral("color: #E3EAF3;"));
    separator->setFixedHeight(2);
    cardLayout->addWidget(separator);

    /* ---------- 可编辑字段 ---------- */
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

    auto *nameEdit = new QLineEdit(QStringLiteral("张三"), card);
    cardLayout->addLayout(makeFieldRow(QStringLiteral("姓名"), nameEdit));

    auto *phoneEdit = new QLineEdit(QStringLiteral("138*****5678"), card);
    cardLayout->addLayout(makeFieldRow(QStringLiteral("手机号码"), phoneEdit));

    auto *medcardEdit = new QLineEdit(QStringLiteral("************1234"), card);
    cardLayout->addLayout(makeFieldRow(QStringLiteral("医保卡号"), medcardEdit));

    // 认证状态（只读，绿色 + 对勾）
    auto *statusRow = new QHBoxLayout;
    statusRow->setSpacing(12);
    auto *statusLbl = new QLabel(QStringLiteral("认证状态"), card);
    statusLbl->setFixedWidth(120);
    statusLbl->setStyleSheet(QStringLiteral(
        "color: #7A93B0; font-size: 18px; background: transparent;"));
    statusRow->addWidget(statusLbl);

    auto *statusValue = new QLabel(QStringLiteral("已认证"), card);
    statusValue->setStyleSheet(QStringLiteral(
        "color: #2E9E6B; font-size: 18px; font-weight: bold; background: transparent;"));
    statusRow->addWidget(statusValue);

    auto *checkIcon = new QLabel(card);
    checkIcon->setPixmap(IconFactory::renderSvg(IconFactory::certifiedSvg(), QSize(22, 22)));
    statusRow->addWidget(checkIcon, 0, Qt::AlignVCenter);
    statusRow->addStretch(1);
    cardLayout->addLayout(statusRow);

    cardLayout->addStretch(1);

    /* ---------- 底部：保存修改 ---------- */
    auto *saveRow = new QHBoxLayout;
    saveRow->addStretch(1);
    auto *saveBtn = UIStyle::createPrimaryButton(QStringLiteral("保存修改"), card);
    saveBtn->setFixedSize(200, 54);
    connect(saveBtn, &QPushButton::clicked, this, [] {
        // 预留：将输入框内容提交保存
    });
    saveRow->addWidget(saveBtn);
    cardLayout->addLayout(saveRow);
}
