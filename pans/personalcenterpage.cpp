#include "personalcenterpage.h"
#include "../MyTcp/cdata.h"
#include "../core/iconfactory.h"
#include "../core/uistyle.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QShowEvent>
#include <QVBoxLayout>

namespace {

/* 手机号中间打码：138****5678；空串返回"——"，非 11 位按位返回 */
QString maskPhone(const QString &phone)
{
    const QString t = phone.trimmed();
    if (t.isEmpty())
        return QStringLiteral("——");
    if (t.length() == 11)
        return t.left(3) + QStringLiteral("****") + t.right(4);
    if (t.length() >= 7) // 兜底打码：保留前3后2
        return t.left(3) + QStringLiteral("****") + t.right(2);
    return t;
}

} // namespace

PersonalCenterPage::PersonalCenterPage(QWidget *parent)
    : QWidget(parent)
{
    initLayout();
    refreshInfo(); // 首次按当前 CData 初始化（此后每次进入页面由 showEvent 刷新）
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

    // 4 行个人信息（值标签保存为成员，供 refreshInfo() 改文本）
    auto buildRow = [card, cardLayout](const QString &name, QLabel **valueOut,
                                       bool certifiedIcon) {
        auto *rowLayout = new QHBoxLayout;
        rowLayout->setSpacing(10);

        auto *nameLbl = new QLabel(name, card);
        nameLbl->setStyleSheet(QStringLiteral(
            "color: #7A93B0; font-size: 20px; background: transparent;"));
        rowLayout->addWidget(nameLbl);

        auto *valueLbl = new QLabel(QStringLiteral("——"), card);
        valueLbl->setStyleSheet(QStringLiteral(
            "color: #1F4E79; font-size: 22px; font-weight: bold; background: transparent;"));
        rowLayout->addWidget(valueLbl);
        if (valueOut)
            *valueOut = valueLbl;

        if (certifiedIcon) { // 认证对勾图标（医保卡号右侧）
            auto *checkLbl = new QLabel(card);
            checkLbl->setPixmap(IconFactory::renderSvg(IconFactory::certifiedSvg(), QSize(24, 24)));
            rowLayout->addWidget(checkLbl, 0, Qt::AlignVCenter);
        }

        rowLayout->addStretch(1);
        cardLayout->addLayout(rowLayout);
    };

    buildRow(QStringLiteral("姓名："),     &m_nameValue,  false);
    buildRow(QStringLiteral("手机号码："), &m_phoneValue, false);
    buildRow(QStringLiteral("医保卡号："), &m_cardValue,  true);  // 右侧带认证对勾
    buildRow(QStringLiteral("认证状态："), &m_certValue,  false);

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

void PersonalCenterPage::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    refreshInfo(); // 每次进入个人中心都按当前登录用户刷新
}

void PersonalCenterPage::refreshInfo()
{
    const QString name  = CData::m_name.trimmed();   // 登录响应返回的患者姓名
    const QString phone = CData::m_phone.trimmed();  // 登录时写入的手机号码
    const QString phoneDisplay = maskPhone(phone);   // 中间打码

    if (m_nameValue)
        m_nameValue->setText(name.isEmpty() ? QStringLiteral("——") : name);
    if (m_phoneValue)
        m_phoneValue->setText(phoneDisplay);
    if (m_cardValue)
        m_cardValue->setText(phoneDisplay); // 医保卡号暂无独立数据源，暂与手机号一致
    if (m_certValue)
        m_certValue->setText(QStringLiteral("已认证")); // 认证状态默认已认证
}
