#include "myagent.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

MyAgent *MyAgent::m_instance = nullptr;

MyAgent::MyAgent()
    : DeepSeekAgent(QStringLiteral(
          "你是医院自助终端的AI信息助手小A，帮助用户查看或修改个人信息（姓名、手机号码、医保卡号、认证状态等）。\n"
          "【强制输出规则】你**只能返回一段纯JSON字符串，禁止输出任何其他文字、注释、markdown**。\n"
          "JSON固定包含3个字段：\n"
          "1. message：字符串，语音播报给用户的回答文本，简洁规范中文。\n"
          "2. decision：整数，决策码：0=无需填写；1=填写手机号；2=填写密码；3=填写姓名；4=填写医保卡号。\n"
          "3. content：字符串，需要自动填充到表单的内容；无需填充时content为空字符串\"\"。\n"
          "根据用户对话识别用户意图：如果用户提供了手机号，则decision=1，content为手机号；用户提供姓名则decision=3；用户提供医保卡号则decision=4；用户提供密码则decision=2；其他情况decision=0，content=\"\"。\n"
          "示例：\n"
          "用户：我的手机号是13800138000\n"
          "输出：{\"message\":\"已识别到您的手机号，将自动填写\",\"decision\":1,\"content\":\"13800138000\"}\n"
          "用户：帮我看一下个人信息\n"
          "输出：{\"message\":\"好的，正在为您查询个人信息\",\"decision\":0,\"content\":\"\"}\n"))
{
}

QString MyAgent::ask(const QString &question)
{
    // 重置本轮决策与填充内容
    decision = 0;
    content = "";

    const QByteArray raw = DeepSeekAgent::ask(question);
    QString replyMsg = QStringLiteral("网络连接失败，请稍后再试。");
    if (raw.isEmpty())
    {
        return replyMsg;
    }

    QJsonParseError jsonErr;
    const QJsonDocument doc = QJsonDocument::fromJson(raw, &jsonErr);
    if (jsonErr.error != QJsonParseError::NoError)
    {
        replyMsg = QStringLiteral("抱歉，我没有理解您的意思，请换个说法再试一次。");
        return replyMsg;
    }

    const QJsonArray choices = doc.object().value("choices").toArray();
    if (!choices.isEmpty())
    {
        QJsonObject choiceObj = choices.first().toObject();
        QJsonObject aiMsgObj = choiceObj.value("message").toObject();
        QString aiRawContent = aiMsgObj.value("content").toString();

        // 解析AI输出的内层JSON（就是我们prompt要求的{message,decision,content}）
        QJsonDocument innerDoc = QJsonDocument::fromJson(aiRawContent.toUtf8(), &jsonErr);
        if (jsonErr.error == QJsonParseError::NoError)
        {
            QJsonObject innerObj = innerDoc.object();
            replyMsg = innerObj.value("message").toString();
            decision = innerObj.value("decision").toInt(0);
            content = innerObj.value("content").toString("");
        }
        else
        {
            replyMsg = QStringLiteral("抱歉，我没有理解您的意思，请换个说法再试一次。");
        }
    }

    // 存入对话上下文，保存AI原始返回文本（内层json字符串）
    recordAssistantReply(replyMsg);
   qDebug()<<"决策"<<decision<<"内容"<<content;
    return replyMsg;

}

void MyAgent::getDecisionAndContent(int &outDecision, QString &outContent)const
{
    outDecision = decision;
    outContent = content;
}

MyAgent *MyAgent::instance()
{
    if (!m_instance)
        m_instance = new MyAgent;
    return m_instance;
}
