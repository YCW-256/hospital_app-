#include "uistyle.h"

#include <QAbstractItemView>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QTableView>

namespace UIStyle {

void stylePageTitle(QLabel *label)
{
    label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    label->setStyleSheet(QStringLiteral(
        "color: #FFFFFF;"
        "font-size: 34px;"
        "font-weight: bold;"
        "background: transparent;"));
}

void styleTransparentPage(QWidget *page)
{
    page->setAttribute(Qt::WA_StyledBackground, true);
    page->setStyleSheet(QStringLiteral("background: transparent;"));
}

QString resolveImagePath(const QString &relativePath)
{
    const QStringList baseDirs = {
        QDir::currentPath(),
        QCoreApplication::applicationDirPath(),
        QDir::cleanPath(QCoreApplication::applicationDirPath() + QStringLiteral("/..")),
    };
    for (const QString &dir : baseDirs) {
        const QString candidate = QDir(dir).filePath(relativePath);
        if (QFile::exists(candidate))
            return candidate;
    }
    return relativePath; // 找不到则返回原相对路径（图片不显示）
}

void styleCard(QWidget *card)
{
    card->setObjectName(QStringLiteral("whiteCard"));
    card->setAttribute(Qt::WA_StyledBackground, true);
    card->setStyleSheet(QStringLiteral(
        "#whiteCard {"
        "    background-color: #FFFFFF;"
        "    border-radius: 24px;"
        "}"));
}

QPushButton *createPrimaryButton(const QString &text, QWidget *parent)
{
    auto *btn = new QPushButton(text, parent);
    btn->setCursor(Qt::PointingHandCursor);
    btn->setMinimumSize(160, 52);
    btn->setStyleSheet(QStringLiteral(
        "QPushButton {"
        "    color: #FFFFFF;"
        "    background-color: %1;"
        "    border: none;"
        "    border-radius: 26px;"
        "    font-size: 18px;"
        "    font-weight: bold;"
        "}"
        "QPushButton:hover   { background-color: %2; }"
        "QPushButton:pressed { background-color: %3; }")
        .arg(kPrimaryBlue, kPrimaryBlueHover, kPrimaryBluePress));
    return btn;
}

QPushButton *createOutlineButton(const QString &text, QWidget *parent)
{
    auto *btn = new QPushButton(text, parent);
    btn->setCursor(Qt::PointingHandCursor);
    btn->setMinimumSize(160, 52);
    btn->setStyleSheet(QStringLiteral(
        "QPushButton {"
        "    color: %1;"
        "    background-color: #FFFFFF;"
        "    border: 2px solid %1;"
        "    border-radius: 26px;"
        "    font-size: 18px;"
        "    font-weight: bold;"
        "}"
        "QPushButton:hover   { background-color: #EAF2FB; }"
        "QPushButton:pressed { background-color: #D6E6F7; }")
        .arg(kDeepBlue));
    return btn;
}

void styleTable(QTableView *table)
{
    table->setAlternatingRowColors(true);                    // 隔行底色
    table->setSelectionMode(QAbstractItemView::NoSelection); // 禁止选中
    table->setEditTriggers(QAbstractItemView::NoEditTriggers); // 禁止编辑
    table->setShowGrid(false);
    table->verticalHeader()->setVisible(false);              // 隐藏行号

    table->setStyleSheet(QStringLiteral(
        "QTableView {"
        "    background-color: #FFFFFF;"
        "    alternate-background-color: %1;"
        "    border: 1px solid %2;"
        "    border-radius: 10px;"
        "    font-size: 15px;"
        "}"
        "QTableView::item {"
        "    padding: 6px;"
        "    border: none;"
        "}"
        "QHeaderView::section {"
        "    background-color: %3;"
        "    color: #FFFFFF;"
        "    font-size: 15px;"
        "    font-weight: bold;"
        "    padding: 10px 6px;"
        "    border: none;"
        "    border-right: 1px solid %4;"
        "}"
        "QTableCornerButton::section {"
        "    background-color: %3;"
        "    border: none;"
        "}")
        .arg(kTableAlt, kTableGrid, kDeepBlue, kHeaderSep));
}

} // namespace UIStyle
