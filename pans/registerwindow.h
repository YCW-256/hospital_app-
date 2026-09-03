#ifndef REGISTERWINDOW_H
#define REGISTERWINDOW_H

#include <QWidget>
#include "../MyTcp/protecol.h"
#include "../Tool/myutils.h"
#include <QByteArray>
class AIAssistantPopup;
class QLabel;
class QLineEdit;

/**
 * @brief 注册账号界面（作为 MainWindow 的子遮罩，不加入 QStackedWidget）
 *
 * 布局与修改信息页一致（白色圆角卡片 + 字段行）：姓名 / 手机号码 / 密码（掩码）/
 * 医保卡号 / 认证状态。页面标题为「注册账号」，底部为【退出】与【确定注册】按钮，
 * 【退出】返回登录页，【确定注册】校验通过后返回登录页。
 * 卡片头部右侧为【语音助手】按钮，弹出悬浮 AI 信息助手（AIAssistantPopup，
 * 与修改信息页共用同一个 MyAgent 个人信息助手智能体，单实例、居中于主窗口）。
 */
class RegisterWindow : public QWidget
{
    Q_OBJECT

public:
    explicit RegisterWindow(QWidget *parent = nullptr);

    /** 注册成功的手机号码（用于返回登录页预填） */
    QString phone() const;

signals:
    void exitRequested();      // 【退出】→ 返回登录界面
    void registerSucceeded();  // 【确定注册】→ 返回登录界面

private:
    void initUi();           // 构建界面
    void onConfirm();        // 校验并完成注册
    void onVoiceAssistant(); // 【语音助手】→ 弹出 AI 信息助手
    void init_agent_connect();

    void init_connect();

    QLineEdit *m_nameEdit    = nullptr; // 姓名
    QLineEdit *m_phoneEdit   = nullptr; // 手机号码
    QLineEdit *m_passEdit    = nullptr; // 密码
    QLineEdit *m_medcardEdit = nullptr; // 医保卡号
    QLabel    *m_errorLabel  = nullptr; // 错误提示（默认隐藏）

    AIAssistantPopup *m_aiPopup = nullptr; // AI 信息助手弹窗（单实例）
signals:
    void data_ready(QByteArray data,int size);
};

#endif // REGISTERWINDOW_H
