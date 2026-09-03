#ifndef PAYMENTPAGE_H
#define PAYMENTPAGE_H

#include <QWidget>

/**
 * @brief 页面1：待缴费用清单页（QStackedWidget 索引 1）
 *
 * 结构：大标题 + 白色圆角卡片（费用表格 / 合计与支付方式 / 卡片内上一步）
 *      + 页面底部【上一步】【确认缴费】并排按钮
 */
class PaymentPage : public QWidget
{
    Q_OBJECT

public:
    explicit PaymentPage(QWidget *parent = nullptr);

signals:
    void backRequested();    // 【上一步/返回】点击，由主窗口切回首页
    void confirmRequested(); // 【确认缴费】点击

private:
    void initLayout(); // 构建页面布局
};

#endif // PAYMENTPAGE_H
