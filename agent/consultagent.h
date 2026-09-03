#ifndef CONSULTAGENT_H
#define CONSULTAGENT_H

#include <QString>

#include "deepseekagent.h"

/**
 * @brief AI 快速问诊智能体（AI 快速问诊页使用）
 *
 * 与个人信息助手智能体（MyAgent）分离：独立的单例、独立的对话记忆、
 * 独立的"医疗问诊"系统提示词，两个页面互不影响，后续可各自独立演进。
 * 网络请求复用基类 DeepSeekAgent；本类负责把基类获取的原始数据解析成
 * QString 回复文本供界面使用（ask() 隐藏基类 DeepSeekAgent::ask()）。
 */
class ConsultAgent : public DeepSeekAgent
{
public:
    static ConsultAgent *instance();

    /** 请求 AI 问诊医生，解析基类返回的原始数据后返回回复文本（网络失败/超时返回友好提示） */
    QString ask(const QString &question);

private:
    explicit ConsultAgent();
    static ConsultAgent *m_instance;
};

#endif // CONSULTAGENT_H
