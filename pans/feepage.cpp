#include "feepage.h"
#include "../core/uistyle.h"

#include <QButtonGroup>
#include <QComboBox>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QStringList>
#include <QTableWidget>
#include <QVBoxLayout>

namespace {
/* 侧边栏条目按钮样式 */
QPushButton *createSideItem(const QString &text, QWidget *parent)
{
    auto *btn = new QPushButton(text, parent);
    btn->setCursor(Qt::PointingHandCursor);
    btn->setMinimumHeight(44);
    btn->setStyleSheet(QStringLiteral(
        "QPushButton {"
        "    color: #1F4E79;"
        "    background-color: #FFFFFF;"
        "    border: 1px solid #E3EAF3;"
        "    border-radius: 10px;"
        "    font-size: 15px;"
        "    text-align: left;"
        "    padding-left: 16px;"
        "}"
        "QPushButton:hover { background-color: #EAF2FB; border-color: #2E86DE; }"
        "QPushButton:checked {"
        "    background-color: #2E86DE;"
        "    color: #FFFFFF;"
        "    border-color: #2E86DE;"
        "}"));
    return btn;
}

/* 下拉框样式 */
void styleCombo(QComboBox *combo)
{
    combo->setMinimumHeight(44);
    combo->setStyleSheet(QStringLiteral(
        "QComboBox {"
        "    color: #1F4E79;"
        "    background-color: #FFFFFF;"
        "    border: 1px solid #C8D4E2;"
        "    border-radius: 10px;"
        "    padding: 0 14px;"
        "    font-size: 15px;"
        "}"
        "QComboBox::drop-down { border: none; width: 28px; }"
        "QComboBox QAbstractItemView {"
        "    color: #1F4E79;"
        "    background-color: #FFFFFF;"
        "    selection-background-color: #EAF2FB;"
        "    selection-color: #1F4E79;"
        "    border: 1px solid #C8D4E2;"
        "}"));
}
} // namespace

FeeQueryPage::FeeQueryPage(QWidget *parent)
    : QWidget(parent)
{
    initLayout();
}

void FeeQueryPage::initLayout()
{
    UIStyle::styleTransparentPage(this); // 透出主窗口蓝色渐变
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(64, 40, 64, 32);
    root->setSpacing(22);

    /* ---------- 页面大标题 ---------- */
    auto *title = new QLabel(QStringLiteral("费用查询"), this);
    UIStyle::stylePageTitle(title);
    root->addWidget(title);

    /* ---------- 白色圆角卡片 ---------- */
    auto *card = new QWidget(this);
    UIStyle::styleCard(card);
    root->addWidget(card, 1);

    auto *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(28, 24, 28, 24);
    cardLayout->setSpacing(16);

    /* ---------- 顶部筛选栏 ---------- */
    auto *filterRow = new QHBoxLayout;
    filterRow->setSpacing(14);

    auto *comboQuery = new QComboBox(card);
    comboQuery->addItems({ QStringLiteral("全部"), QStringLiteral("门诊费用"),
                           QStringLiteral("住院费用"), QStringLiteral("药品费用") });
    auto *comboType = new QComboBox(card);
    comboType->addItems({ QStringLiteral("全部"), QStringLiteral("自费"), QStringLiteral("医保") });
    auto *comboTime = new QComboBox(card);
    comboTime->addItems({ QStringLiteral("近一个月"), QStringLiteral("近三个月"),
                          QStringLiteral("近半年"), QStringLiteral("全部") });
    styleCombo(comboQuery);
    styleCombo(comboType);
    styleCombo(comboTime);

    filterRow->addWidget(comboQuery, 1);
    filterRow->addWidget(comboType, 1);
    filterRow->addWidget(comboTime, 1);
    filterRow->addStretch(1);

    auto *queryBtn = UIStyle::createPrimaryButton(QStringLiteral("查询"), card);
    queryBtn->setFixedSize(140, 44);
    filterRow->addWidget(queryBtn);
    cardLayout->addLayout(filterRow);

    /* ---------- 中部：左侧侧边栏 + 右侧明细表格 ---------- */
    auto *bodyRow = new QHBoxLayout;
    bodyRow->setSpacing(18);

    // 左侧固定侧边栏
    auto *sideLayout = new QVBoxLayout;
    sideLayout->setSpacing(8);
    sideLayout->setContentsMargins(0, 0, 0, 0);

    const QStringList sideItems = {
        QStringLiteral("费用查询"), QStringLiteral("费用类型"), QStringLiteral("时间范围"),
        QStringLiteral("药品费"),  QStringLiteral("检查费"),   QStringLiteral("诊疗费"),
    };
    auto *sideGroup = new QButtonGroup(this);
    sideGroup->setExclusive(true);
    for (int i = 0; i < sideItems.size(); ++i) {
        auto *item = createSideItem(sideItems.at(i), card);
        item->setCheckable(true);
        sideGroup->addButton(item, i);
        sideLayout->addWidget(item);
    }
    sideGroup->button(0)->setChecked(true); // 默认选中第一项

    // 侧边栏底部：返回 / 导出明细
    auto *backItem = createSideItem(QStringLiteral("返回"), card);
    auto *exportItem = createSideItem(QStringLiteral("导出明细"), card);
    connect(backItem, &QPushButton::clicked, this, &FeeQueryPage::backRequested);
    connect(exportItem, &QPushButton::clicked, this, &FeeQueryPage::exportRequested);
    sideLayout->addStretch(1);
    sideLayout->addWidget(backItem);
    sideLayout->addWidget(exportItem);
    bodyRow->addLayout(sideLayout);

    // 右侧明细表格
    auto *table = new QTableWidget(5, 8, card);
    UIStyle::styleTable(table);
    table->setHorizontalHeaderLabels({
        QStringLiteral("查询结果"), QStringLiteral("费用条件"), QStringLiteral("费用类型"),
        QStringLiteral("时间范围"), QStringLiteral("药品费"),   QStringLiteral("检查费"),
        QStringLiteral("诊疗费"),   QStringLiteral("导出明细")
    });
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->verticalHeader()->setDefaultSectionSize(40);

    // 预置多行示例数据
    const QList<QStringList> rows = {
        { QStringLiteral("门诊费用"), QStringLiteral("门诊"), QStringLiteral("自费"),
          QStringLiteral("2026-08"), QStringLiteral("120.00"), QStringLiteral("80.00"),
          QStringLiteral("40.00"),  QStringLiteral("明细") },
        { QStringLiteral("住院费用"), QStringLiteral("住院"), QStringLiteral("医保"),
          QStringLiteral("2026-08"), QStringLiteral("300.00"), QStringLiteral("200.00"),
          QStringLiteral("100.00"), QStringLiteral("明细") },
        { QStringLiteral("检查项目"), QStringLiteral("检查"), QStringLiteral("自费"),
          QStringLiteral("2026-07"), QStringLiteral("0.00"),   QStringLiteral("260.00"),
          QStringLiteral("0.00"),   QStringLiteral("明细") },
        { QStringLiteral("药品费用"), QStringLiteral("药品"), QStringLiteral("医保"),
          QStringLiteral("2026-07"), QStringLiteral("186.50"), QStringLiteral("0.00"),
          QStringLiteral("0.00"),   QStringLiteral("明细") },
        { QStringLiteral("诊疗费用"), QStringLiteral("诊疗"), QStringLiteral("医保"),
          QStringLiteral("2026-06"), QStringLiteral("0.00"),   QStringLiteral("0.00"),
          QStringLiteral("150.00"), QStringLiteral("明细") },
    };
    for (int r = 0; r < rows.size(); ++r)
        for (int c = 0; c < rows.at(r).size(); ++c) {
            auto *item = new QTableWidgetItem(rows.at(r).at(c));
            item->setTextAlignment(Qt::AlignCenter);
            table->setItem(r, c, item);
        }
    bodyRow->addWidget(table, 1);
    cardLayout->addLayout(bodyRow, 1);

    /* ---------- 卡片底部：【返回】【导出明细】并排 ---------- */
    auto *bottomRow = new QHBoxLayout;
    bottomRow->addStretch(1);
    auto *backBtn = UIStyle::createOutlineButton(QStringLiteral("返回"), card);
    auto *exportBtn = UIStyle::createPrimaryButton(QStringLiteral("导出明细"), card);
    connect(backBtn, &QPushButton::clicked, this, &FeeQueryPage::backRequested);
    connect(exportBtn, &QPushButton::clicked, this, &FeeQueryPage::exportRequested);
    bottomRow->addWidget(backBtn);
    bottomRow->addSpacing(24);
    bottomRow->addWidget(exportBtn);
    bottomRow->addStretch(1);
    cardLayout->addLayout(bottomRow);
}
