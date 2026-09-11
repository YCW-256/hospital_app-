#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "../MyTcp/protecol.h"
#include "../MyTcp/socketlink.h"
class AIAssistantPopup;
class AIConsultPage;
class AppointmentPage;
class FeeQueryPage;
class LoginWindow;
class ModifyInfoPage;
class PaymentPage;
class PersonalCenterPage;
class QResizeEvent;
class QStackedWidget;
class RegisterWindow;
class TopNavBar;
class QWidget;

/**
 * @brief 医院自助终端主窗口
 *
 * 架构：【顶部导航栏(TopNavBar) + 中部页面堆栈(QStackedWidget)】
 *  - centralWidget 采用垂直布局：上方自定义顶部导航栏，下方 QStackedWidget
 *  - QStackedWidget 第 0 页为首页（2 行 3 列六个圆角功能按钮），
 *    其余为各功能子页与占位页，点击首页按钮即可切换
 *  - 无边框窗口，蓝色渐变背景
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    /** 启用登录：登录/注册窗口作为主窗口的子遮罩（不加入 QStackedWidget），启动时覆盖显示 */
    void enableLogin();

protected:
    void resizeEvent(QResizeEvent *event) override; // 遮罩随主窗口尺寸同步

private:
    AppointmentPage*appointmentPage;


    void init_data_connect();
    //客户端连接标识
    SocketLink *m_socket;

    void initCentralUi();   // 中央容器：导航栏 + 页面堆栈
    void initStackedPages(); // 填充首页、功能页与占位页
    void connectNavTitle();  // 导航栏标题随 stack 页面联动

    QWidget *createHomePage();              // 第 0 页：首页
    QWidget *createPlaceholderPage(const QString &title); // 功能占位页

    TopNavBar         *m_navBar = nullptr; // 顶部导航栏
    QStackedWidget    *m_stack  = nullptr; // 页面堆栈
    AIAssistantPopup  *m_aiPopup = nullptr; // 悬浮 AI 助手弹窗（单实例）
    LoginWindow       *m_loginWindow = nullptr; // 登录遮罩（子控件，不入 stack）
    RegisterWindow    *m_registerWindow = nullptr; // 注册遮罩（子控件，不入 stack）
    bool               m_loginEnabled = false; // 是否启用登录流程

    AIConsultPage *aiConsultPage;
signals:
    void send_data(QByteArray data,int size);

};

#endif // MAINWINDOW_H
