#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <QWidget>
#include <QPushButton>
#include "../Tool/myutils.h"
#include "../MyTcp/protecol.h"
class QLabel;
class QLineEdit;

/**
 * @brief 登录界面（作为 MainWindow 的子遮罩，不加入 QStackedWidget）
 *
 * 蓝色渐变背景，白色圆角卡片内输入手机号码 + 密码；
 * 卡片底部提供【立即注册】入口。校验通过发 loginSucceeded()，
 * 由 MainWindow 隐藏本遮罩露出主界面；登录成功之前主界面不可见。
 */
class LoginWindow : public QWidget
{
    Q_OBJECT

public:
    explicit LoginWindow(QWidget *parent = nullptr);

    /** 预填手机号码（注册成功后带回登录页） */
    void setPhone(const QString &phone);

signals:
    void loginSucceeded();    // 登录成功 → 显示主界面
    void registerRequested(); // 点击【立即注册】→ 跳转注册界面
    void data_ready(QByteArray data,int size); // 发送数据到主界面

private:
    void initUi(); // 构建界面
    void onLogin(); // 校验并登录

    QLineEdit *m_phoneEdit  = nullptr; // 手机号码输入框
    QLineEdit *m_passEdit   = nullptr; // 密码输入框
    QLabel    *m_errorLabel = nullptr; // 登录错误提示（默认隐藏）

    QPushButton *loginBtn ;
};

#endif // LOGINWINDOW_H
