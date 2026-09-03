#ifndef DEEPSEEKAGENT_H
#define DEEPSEEKAGENT_H

#include <QObject>
#include <QByteArray>
#include <QString>

#include <deque>

/**
 * @brief DeepSeek 智能体基类（移植自 F:\hospital_project\chat 工程）
 *
 * 只负责调用 DeepSeek Chat API 获取大模型返回的原始数据（JSON 响应体），
 * 不负责解析；原始数据的解析交给各派生类，由派生类整理成 QString 回复文本供界面使用。
 *
 * 带对话记忆：历史轮次（"用户: xxx" / "小A: xxx"）随每次请求一起发送，
 * 让 AI 能记住之前的对话上下文；记忆过长时自动裁剪。
 *
 * 注意：ask() 与派生类同名方法互为"隐藏"关系——基类 ask() 返回原始数据，
 * 派生类 ask() 解析后返回回复文本，二者参数一致、返回类型不同，调用处需按类型区分。
 */
class DeepSeekAgent : public QObject
{
    Q_OBJECT

public:
    /**
     * 同步请求 DeepSeek，返回模型返回的原始数据（JSON 响应体）。
     * 网络失败 / 请求超时返回空 QByteArray，具体提示文案由派生类解析时决定。
     * 本方法只记录用户提问到对话记忆；助手回复由派生类解析后调用
     * recordAssistantReply() 记录。
     */
    QByteArray ask(const QString &question);

protected:
    /** @param systemPrompt 系统提示词（决定智能体角色） */
    explicit DeepSeekAgent(const QString &systemPrompt, QObject *parent = nullptr);

    /** 记录助手回复到对话记忆（派生类解析出回复文本后调用），记忆过长自动裁剪 */
    void recordAssistantReply(const QString &reply);

private:
    QString m_systemPrompt;        // 系统提示词
    std::deque<QString> m_history; // 对话记忆（"用户: xxx" / "小A: xxx"）
};

#endif // DEEPSEEKAGENT_H
