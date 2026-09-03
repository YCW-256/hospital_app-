#ifndef FUNCTIONBUTTON_H
#define FUNCTIONBUTTON_H

#include <QPushButton>

class QLabel;

/**
 * @brief 首页圆角功能按钮
 *
 * 继承 QPushButton，自带点击(clicked)信号与 hover/pressed 状态。
 * 内部采用垂直布局：上方白色 SVG 图标，下方白色文字。
 * 圆角背景色、hover 加深、按下变暗均由 Qt StyleSheet 实现。
 */
class RoundedFunctionButton : public QPushButton
{
    Q_OBJECT

public:
    /**
     * @param text      按钮下方显示的文字
     * @param iconSvg   内联 SVG 图标（渲染为白色）
     * @param baseColor 按钮背景色，如 "#D9772B"
     * @param parent    父控件
     */
    explicit RoundedFunctionButton(const QString &text,
                                   const QString &iconSvg,
                                   const QString &baseColor,
                                   QWidget *parent = nullptr);
};

#endif // FUNCTIONBUTTON_H
