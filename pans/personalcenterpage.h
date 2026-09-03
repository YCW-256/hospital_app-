#ifndef PERSONALCENTERPAGE_H
#define PERSONALCENTERPAGE_H

#include <QWidget>

/**
 * @brief 个人中心页（QStackedWidget 索引 7）
 *
 * 结构：居中白色大圆角卡片（标题"个人中心" + 4 行个人信息 + 右下角【修改信息】）
 *      + 页面底部（左下提示文字 + 右下【返回首页】）
 */
class PersonalCenterPage : public QWidget
{
    Q_OBJECT

public:
    explicit PersonalCenterPage(QWidget *parent = nullptr);

signals:
    void backRequested();    // 【返回首页】点击，由主窗口切回首页
    void modifyRequested();  // 【修改信息】点击（预留）

private:
    void initLayout(); // 构建页面布局
};

#endif // PERSONALCENTERPAGE_H
