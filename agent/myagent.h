#ifndef MYAGENT_H
#define MYAGENT_H

#include <QString>

#include "deepseekagent.h"

/**
 * @brief 个人信息助手智能体（AI 信息助手弹窗使用）
 *
 * 单例，提示词为"个人信息助手"。
 * 网络请求复用基类 DeepSeekAgent；本类负责把基类获取的原始数据解析成
 * QString 回复文本供界面使用（ask() 隐藏基类 DeepSeekAgent::ask()）。
 */
class MyAgent : public DeepSeekAgent
{
public:
    static MyAgent *instance();

    /** 请求 AI 助手，解析基类返回的原始数据后返回回复文本（网络失败/超时返回友好提示） */
    QString ask(const QString &question);

    void getDecisionAndContent(int &outDecision, QString &outContent)const;

private:
    explicit MyAgent();
    static MyAgent *m_instance;

    int decision; // 决策码：0=无需填写；1=填写手机号；2=填写密码；3=填写姓名；4=填写医保卡号

    QString content; // 需要自动填充到表单的内容；无需填充时content为空字符串""

};

#endif // MYAGENT_H
