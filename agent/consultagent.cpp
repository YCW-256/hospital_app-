#include "consultagent.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

ConsultAgent *ConsultAgent::m_instance = nullptr;

ConsultAgent::ConsultAgent()
    : DeepSeekAgent(QStringLiteral(
        "你是医院自助终端的AI快速问诊医生，帮助用户根据症状（如发热咳嗽、肠胃不适、"
        "皮肤问题、舌苔健康等）进行初步问诊分析，给出生活建议与就医科室建议。"
        "请用中文简洁、友好地回复，不要下确诊结论，仅提供初步健康建议。"))
{
}

QString ConsultAgent::ask(const QString &question)
{
    // 基类只负责获取原始数据，此处解析 JSON 并整理成回复文本
    const QByteArray raw = DeepSeekAgent::ask(question);
    QString reply;
    if (raw.isEmpty()) {
        reply = QStringLiteral("网络连接失败，请稍后再试。");
    } else {
        const QJsonDocument doc = QJsonDocument::fromJson(raw);
        const QJsonArray choices = doc.object().value("choices").toArray();
        if (!choices.isEmpty()) {
            reply = choices.first().toObject()
                        .value("message").toObject().value("content").toString();
        }
        if (reply.isEmpty())
            reply = QStringLiteral("抱歉，我没有理解您的意思，请换个说法再试一次。");
    }
    // 解析出的回复记入对话记忆，作为下一轮 AI 的上下文
    recordAssistantReply(reply);
    return reply;
}

ConsultAgent *ConsultAgent::instance()
{
    if (!m_instance)
        m_instance = new ConsultAgent;
    return m_instance;
}
