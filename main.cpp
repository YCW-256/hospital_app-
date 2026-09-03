#include "pans/mainwindow.h"

#include <QApplication>
#include <QFont>
#include "MyTcp/cdata.h"
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    CData::init();
    app.setApplicationName(QStringLiteral("医院自助终端"));
    // 统一使用中文友好字体
    app.setFont(QFont(QStringLiteral("Microsoft YaHei"), 10));

    // 登录开关：置 1 启用登录（登录/注册窗口作为主窗口的子遮罩，登录成功后才露出主界面），
    // 置 0 直接显示主界面
    const int kEnableLogin = 0;

    MainWindow window;
    // if (kEnableLogin == 1)
    //window.enableLogin();
    window.show();

    return QApplication::exec();
}
