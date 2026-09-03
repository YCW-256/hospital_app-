#include "deepseekagent.h"

#include <QEventLoop>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>

namespace {
const char kApiKey[]  = "sk-54d9a446eaa64d1091481d5d53403075"; // DeepSeek API Key（与 chat 工程一致）
const char kApiUrl[]  = "https://api.deepseek.com/chat/completions";
const char kModel[]   = "deepseek-chat";
const int  kTimeoutMs = 15000;
}

DeepSeekAgent::DeepSeekAgent(const QString &systemPrompt, QObject *parent)
    : QObject(parent)
    , m_systemPrompt(systemPrompt)
{
}

QByteArray DeepSeekAgent::ask(const QString &question)
{
    QNetworkAccessManager manager;
    QNetworkRequest request(QUrl(QString::fromLatin1(kApiUrl)));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization",
                         QString("Bearer %1").arg(QString::fromLatin1(kApiKey)).toUtf8());

    QJsonObject body;
    body["model"] = QString::fromLatin1(kModel);
    body["stream"] = false;

    QJsonArray messages;
    QJsonObject sysMsg;
    sysMsg["role"] = "system";
    QString sysContent = m_systemPrompt;
    // 追加对话记忆：把之前的对话轮次发给模型，使 AI 具备上下文记忆
    for (const QString &turn : m_history)
        sysContent += QStringLiteral("\n") + turn;
    sysMsg["content"] = sysContent;
    messages.append(sysMsg);

    QJsonObject userMsg;
    userMsg["role"] = "user";
    userMsg["content"] = question;
    messages.append(userMsg);

    body["messages"] = messages;

    QNetworkReply *reply = manager.post(request, QJsonDocument(body).toJson(QJsonDocument::Compact));

    // 同步等待结果（与 chat 工程一致）
    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);
    QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    timer.start(kTimeoutMs);
    loop.exec();

    // 只负责获取原始数据；网络失败 / 超时返回空数据，由派生类解析时给出提示文案
    QByteArray raw;
    if (timer.isActive()) {
        timer.stop();
        if (reply->error() == QNetworkReply::NoError)
            raw = reply->readAll();
    }

    // 记录本轮用户提问到记忆（助手回复由派生类解析后通过 recordAssistantReply() 记录）
    m_history.push_back(QStringLiteral("用户: ") + question);
    // 记忆过长时裁剪，只保留最近若干轮
    while (m_history.size() > 20)
        m_history.pop_front();

    reply->deleteLater();
    return raw;
}

void DeepSeekAgent::recordAssistantReply(const QString &reply)
{
    m_history.push_back(QStringLiteral("小A: ") + reply);
    // 记忆过长时裁剪，只保留最近若干轮
    while (m_history.size() > 20)
        m_history.pop_front();
}
