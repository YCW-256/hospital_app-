#ifndef FEEPAGE_H
#define FEEPAGE_H

#include <QWidget>

/**
 * @brief 页面3：费用查询页（QStackedWidget 索引 3）
 *
 * 白色圆角卡片：
 *  - 顶部筛选栏：三个下拉框（费用查询/费用类型/时间范围）+ 蓝色【查询】按钮
 *  - 中部：左侧固定侧边栏（费用查询/费用类型/时间范围/药品费/检查费/诊疗费/返回/导出明细）
 *          + 右侧明细表格
 *  - 卡片底部：【返回】【导出明细】并排按钮
 */
class FeeQueryPage : public QWidget
{
    Q_OBJECT

public:
    explicit FeeQueryPage(QWidget *parent = nullptr);

signals:
    void backRequested();    // 【返回】点击，返回首页
    void exportRequested();  // 【导出明细】点击

private:
    void initLayout(); // 构建页面布局
};

#endif // FEEPAGE_H
