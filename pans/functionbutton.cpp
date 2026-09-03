#include "functionbutton.h"
#include "../core/iconfactory.h"

#include <QColor>
#include <QLabel>
#include <QVBoxLayout>

RoundedFunctionButton::RoundedFunctionButton(const QString &text,
                                             const QString &iconSvg,
                                             const QString &baseColor,
                                             QWidget *parent)
    : QPushButton(parent)
{
    setObjectName(QStringLiteral("functionButton"));
    setCursor(Qt::PointingHandCursor);
    setMinimumSize(220, 150);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // ---------- 内部垂直布局：图标在上、文字在下 ----------
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(8, 22, 8, 18);
    layout->setSpacing(12);
    layout->setAlignment(Qt::AlignCenter);

    // 白色 SVG 图标
    auto *iconLabel = new QLabel(this);
    iconLabel->setPixmap(IconFactory::renderSvg(iconSvg, QSize(58, 58)));
    iconLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(iconLabel, 0, Qt::AlignCenter);

    // 白色文字
    auto *textLabel = new QLabel(text, this);
    textLabel->setAlignment(Qt::AlignCenter);
    textLabel->setStyleSheet(
        QStringLiteral("color: #FFFFFF;"
                       "font-size: 20px;"
                       "font-weight: bold;"
                       "background: transparent;"));
    layout->addWidget(textLabel, 0, Qt::AlignCenter);

    // ---------- 由基础色生成 hover / pressed 深浅色 ----------
    const QColor base(baseColor);
    const QColor hover   = base.lighter(118);
    const QColor pressed = base.darker(120);

    // ---------- 圆角 + 背景色样式表 ----------
    setStyleSheet(QStringLiteral(
        "QPushButton#functionButton {"
        "    background-color: %1;"
        "    border: none;"
        "    border-radius: 18px;"
        "}"
        "QPushButton#functionButton:hover {"
        "    background-color: %2;"
        "}"
        "QPushButton#functionButton:pressed {"
        "    background-color: %3;"
        "}").arg(baseColor, hover.name(), pressed.name()));
}
