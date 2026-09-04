#ifndef PERSONALCENTERPAGE_H
#define PERSONALCENTERPAGE_H

#include <QString>
#include <QWidget>

class QLabel;
class QShowEvent;

/**
 * @brief 个人中心页（QStackedWidget 索引 6）
 *
 * 结构：居中白色大圆角卡片（标题"个人中心" + 4 行个人信息 + 右下角【修改信息】）
 *      + 页面底部（左下提示文字 + 右下【返回首页】）
 *
 * 展示当前登录患者信息：姓名 / 手机号码 / 医保卡号 / 认证状态。
 * 数据取自 CData（姓名 `m_name`、手机号 `m_phone` 在登录时写入），
 * 页面每次被切换到前台（showEvent）都重新刷新，跟随登录用户变化。
 * 说明：医保卡号暂无独立数据源，暂与手机号展示一致；认证状态默认"已认证"。
 */
class PersonalCenterPage : public QWidget
{
    Q_OBJECT

public:
    explicit PersonalCenterPage(QWidget *parent = nullptr);

signals:
    void backRequested();    // 【返回首页】点击，由主窗口切回首页
    void modifyRequested();  // 【修改信息】点击（预留）

protected:
    void showEvent(QShowEvent *event) override; // 每次进入本页刷新为当前登录用户数据

private:
    void initLayout();  // 构建页面布局（四行值标签保存为成员，供刷新时改文本）
    void refreshInfo(); // 从 CData 读取并刷新四行展示

    QLabel *m_nameValue  = nullptr; // 姓名 值
    QLabel *m_phoneValue = nullptr; // 手机号码 值
    QLabel *m_cardValue  = nullptr; // 医保卡号 值
    QLabel *m_certValue  = nullptr; // 认证状态 值
};

#endif // PERSONALCENTERPAGE_H
