#include "paymentpage.h"
#include "../core/iconfactory.h"
#include "../core/uistyle.h"

#include <QHBoxLayout>
#include <QHeaderView>
#include <QIcon>
#include <QLabel>
#include <QLineEdit>
#include <QList>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>

PaymentPage::PaymentPage(QWidget *parent)
    : QWidget(parent)
{
    initLayout();
}

void PaymentPage::initLayout()
{
    UIStyle::styleTransparentPage(this); // 透出主窗口蓝色渐变
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(64, 40, 64, 28);
    root->setSpacing(22);

    /* ---------- 页面大标题 ---------- */
    auto *title = new QLabel(QStringLiteral("待缴费用清单"), this);
    UIStyle::stylePageTitle(title);
    root->addWidget(title);

    /* ---------- 白色圆角卡片 ---------- */
    auto *card = new QWidget(this);
    UIStyle::styleCard(card);
    root->addWidget(card, 1);

    auto *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(32, 18, 32, 24);
    cardLayout->setSpacing(16);

    // 卡片右上角：下载图标按钮
    auto *topRow = new QHBoxLayout;
    topRow->addStretch(1);
    auto *downloadBtn = new QPushButton(card);
    downloadBtn->setCursor(Qt::PointingHandCursor);
    downloadBtn->setToolTip(QStringLiteral("下载清单"));
    downloadBtn->setFixedSize(44, 44);
    downloadBtn->setIcon(QIcon(IconFactory::renderSvg(IconFactory::downloadSvg(), QSize(24, 24))));
    downloadBtn->setIconSize(QSize(24, 24));
    downloadBtn->setStyleSheet(QStringLiteral(
        "QPushButton { background: transparent; border: none; border-radius: 12px; }"
        "QPushButton:hover { background-color: #EAF2FB; }"
        "QPushButton:pressed { background-color: #D6E6F7; }"));
    topRow->addWidget(downloadBtn);
    cardLayout->addLayout(topRow);

    // 费用表格：项目名称 / 金额 / 数量 / 合计金额
    auto *table = new QTableWidget(3, 4, card);
    UIStyle::styleTable(table);
    table->setHorizontalHeaderLabels({
        QStringLiteral("项目名称"), QStringLiteral("金额"),
        QStringLiteral("数量"),    QStringLiteral("合计金额")
    });
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->verticalHeader()->setDefaultSectionSize(42);

    // 预置 3 行示例数据
    const QList<QStringList> rows = {
        { QStringLiteral("挂号费"), QStringLiteral("20.00"),  QStringLiteral("1"), QStringLiteral("20.00") },
        { QStringLiteral("检查费"), QStringLiteral("150.00"), QStringLiteral("2"), QStringLiteral("300.00") },
        { QStringLiteral("药品费"), QStringLiteral("85.50"),  QStringLiteral("1"), QStringLiteral("85.50") },
    };
    double total = 0.0;
    for (int r = 0; r < rows.size(); ++r) {
        for (int c = 0; c < rows.at(r).size(); ++c) {
            auto *item = new QTableWidgetItem(rows.at(r).at(c));
            item->setTextAlignment(c == 0 ? (Qt::AlignVCenter | Qt::AlignLeft)
                                          : (Qt::AlignVCenter | Qt::AlignRight));
            table->setItem(r, c, item);
        }
        total += rows.at(r).last().toDouble();
    }
    cardLayout->addWidget(table, 1);

    /* ---------- 合计金额 + 支付方式 ---------- */
    auto *payRow = new QHBoxLayout;
    payRow->setSpacing(14);

    auto *totalLabel = new QLabel(QStringLiteral("合计金额"), card);
    totalLabel->setStyleSheet(QStringLiteral(
        "color: #1F4E79; font-size: 18px; font-weight: bold; background: transparent;"));
    payRow->addWidget(totalLabel);

    auto *totalEdit = new QLineEdit(card);
    totalEdit->setText(QStringLiteral("¥ %1").arg(total, 0, 'f', 2));
    totalEdit->setReadOnly(true);
    totalEdit->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    totalEdit->setFixedWidth(220);
    totalEdit->setStyleSheet(QStringLiteral(
        "QLineEdit {"
        "    color: #1F4E79;"
        "    background-color: #F4F8FC;"
        "    border: 1px solid #C8D4E2;"
        "    border-radius: 12px;"
        "    padding: 0 14px;"
        "    font-size: 20px;"
        "    font-weight: bold;"
        "}"));
    payRow->addWidget(totalEdit);

    payRow->addStretch(1);

    auto *payPrimary = UIStyle::createPrimaryButton(QStringLiteral("支付方式"), card);
    auto *payOutline = UIStyle::createOutlineButton(QStringLiteral("支付方式"), card);
    payRow->addWidget(payPrimary);
    payRow->addWidget(payOutline);
    cardLayout->addLayout(payRow);

    // 卡片底部居中：【上一步】蓝色圆角按钮
    auto *cardBackRow = new QHBoxLayout;
    cardBackRow->addStretch(1);
    auto *cardBackBtn = UIStyle::createPrimaryButton(QStringLiteral("上一步"), card);
    cardBackBtn->setFixedSize(200, 54);
    connect(cardBackBtn, &QPushButton::clicked, this, &PaymentPage::backRequested);
    cardBackRow->addWidget(cardBackBtn);
    cardBackRow->addStretch(1);
    cardLayout->addLayout(cardBackRow);

    /* ---------- 页面底部：【上一步】【确认缴费】并排 ---------- */
    auto *bottomRow = new QHBoxLayout;
    bottomRow->addStretch(1);
    auto *backBtn = UIStyle::createOutlineButton(QStringLiteral("上一步"), this);
    auto *confirmBtn = UIStyle::createPrimaryButton(QStringLiteral("确认缴费"), this);
    connect(backBtn, &QPushButton::clicked, this, &PaymentPage::backRequested);
    connect(confirmBtn, &QPushButton::clicked, this, &PaymentPage::confirmRequested);
    bottomRow->addWidget(backBtn);
    bottomRow->addSpacing(24);
    bottomRow->addWidget(confirmBtn);
    bottomRow->addStretch(1);
    root->addLayout(bottomRow);
}
