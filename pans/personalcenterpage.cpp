#include "personalcenterpage.h"
#include "../core/iconfactory.h"
#include "../core/uistyle.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QList>
#include <QPushButton>
#include <QVBoxLayout>

PersonalCenterPage::PersonalCenterPage(QWidget *parent)
    : QWidget(parent)
{
    initLayout();
}

void PersonalCenterPage::initLayout()
{
    UIStyle::styleTransparentPage(this); // 透出主窗口蓝色渐变
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(64, 36, 64, 24);
    root->setSpacing(20);

    root->addStretch(1);

    /* ---------- 居中白色大圆角卡片 ---------- */
    auto *card = new QWidget(this);
    UIStyle::styleCard(card);
    card->setFixedSize(680, 440);
    root->addWidget(card, 0, Qt::AlignHCenter);

    auto *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(52, 32, 52, 26);
    cardLayout->setSpacing(14);

    // 卡片标题
    auto *title = new QLabel(QStringLiteral("个人中心"), card);
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet(QStringLiteral(
        "color: #1F4E79; font-size: 28px; font-weight: bold; background: transparent;"));
    cardLayout->addWidget(title);

    // 分隔线
    auto *separator = new QFrame(card);
    separator->setFrameShape(QFrame::HLine);
    separator->setStyleSheet(QStringLiteral("color: #E3EAF3;"));
    separator->setFixedHeight(2);
    cardLayout->addWidget(separator);

    /* ---------- 4 行个人信息 ---------- */
    struct InfoRow {
        QString name;   // 字段名（含冒号）
        QString value;  // 字段值
        bool    check;  // 是否在右侧显示绿色对勾
    };
    const QList<InfoRow> rows = {
        { QStringLiteral("姓名："),     QStringLiteral("张先生"),          false },
        { QStringLiteral("手机号码："), QStringLiteral("138*****5678"),    false },
        { QStringLiteral("医保卡号："), QStringLiteral("************1234"), true },
        { QStringLiteral("认证状态："), QStringLiteral("已认证"),          false },
    };
    for (const InfoRow &row : rows) {
        auto *rowLayout = new QHBoxLayout;
        rowLayout->setSpacing(10);

        auto *nameLbl = new QLabel(row.name, card);
        nameLbl->setStyleSheet(QStringLiteral(
            "color: #7A93B0; font-size: 20px; background: transparent;"));
        rowLayout->addWidget(nameLbl);

        auto *valueLbl = new QLabel(row.value, card);
        valueLbl->setStyleSheet(QStringLiteral(
            "color: #1F4E79; font-size: 22px; font-weight: bold; background: transparent;"));
        rowLayout->addWidget(valueLbl);

        // 认证对勾图标（医保卡号右侧）
        if (row.check) {
            auto *checkLbl = new QLabel(card);
            checkLbl->setPixmap(IconFactory::renderSvg(IconFactory::certifiedSvg(), QSize(24, 24)));
            rowLayout->addWidget(checkLbl, 0, Qt::AlignVCenter);
        }

        rowLayout->addStretch(1);
        cardLayout->addLayout(rowLayout);
    }

    cardLayout->addStretch(1);

    /* ---------- 卡片右下角：【修改信息】 ---------- */
    auto *modifyRow = new QHBoxLayout;
    modifyRow->addStretch(1);
    auto *modifyBtn = UIStyle::createPrimaryButton(QStringLiteral("修改信息"), card);
    modifyBtn->setFixedSize(180, 52);
    connect(modifyBtn, &QPushButton::clicked, this, &PersonalCenterPage::modifyRequested);
    modifyRow->addWidget(modifyBtn);
    cardLayout->addLayout(modifyRow);

    root->addStretch(2);

    /* ---------- 页面底部：左下提示 + 右下返回首页 ---------- */
    auto *bottomRow = new QHBoxLayout;
    bottomRow->setSpacing(16);

    auto *hint = new QLabel(QStringLiteral("公共终端请注意保护个人信息"), this);
    hint->setStyleSheet(QStringLiteral(
        "color: #FFFFFF; font-size: 15px; background: transparent;"));
    bottomRow->addWidget(hint);

    bottomRow->addStretch(1);

    auto *backBtn = UIStyle::createPrimaryButton(QStringLiteral("返回首页"), this);
    backBtn->setFixedSize(180, 54);
    connect(backBtn, &QPushButton::clicked, this, &PersonalCenterPage::backRequested);
    bottomRow->addWidget(backBtn);
    root->addLayout(bottomRow);
}
