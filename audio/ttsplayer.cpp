#include "ttsplayer.h"

#include <QDebug>

// 防止 windows.h 的 min/max 宏污染 Qt 模板代码
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <objbase.h>
#include <sapi.h>

TtsPlayer *TtsPlayer::m_instance = nullptr;

TtsPlayer::TtsPlayer()
{
    // 初始化 COM（若已被 Qt 或其他代码以其它模式初始化，则忽略返回值）
    m_comInitialized = SUCCEEDED(CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED));
}

TtsPlayer::~TtsPlayer()
{
    if (m_voice)
        static_cast<ISpVoice *>(m_voice)->Release();
    if (m_comInitialized)
        CoUninitialize();
}

TtsPlayer *TtsPlayer::instance()
{
    if (!m_instance)
        m_instance = new TtsPlayer;
    return m_instance;
}

void TtsPlayer::ensureVoice()
{
    if (m_voice)
        return;

    ISpVoice *voice = nullptr;
    HRESULT hr = CoCreateInstance(CLSID_SpVoice, nullptr, CLSCTX_ALL,
                                  IID_ISpVoice, reinterpret_cast<void **>(&voice));
    if (SUCCEEDED(hr) && voice) {
        voice->SetRate(2); // 语速：-10~10
        m_voice = voice;
    } else {
        qDebug() << "创建语音对象失败，语音功能不可用";
    }
}

void TtsPlayer::speak(const QString &text)
{
    if (text.isEmpty())
        return;

    ensureVoice();
    if (!m_voice)
        return;

    // SPF_ASYNC 异步播放；SPF_PURGEBEFORESPEAK 先清掉当前/待播语音，
    // 使新语句打断上一条，而不是排队等待
    static_cast<ISpVoice *>(m_voice)->Speak(text.toStdWString().c_str(),
                                            SPF_ASYNC | SPF_PURGEBEFORESPEAK, nullptr);
}

void TtsPlayer::stop()
{
    if (!m_voice)
        return;
    // 以空文本 + SPF_PURGEBEFORESPEAK 清除当前/待播语音
    static_cast<ISpVoice *>(m_voice)->Speak(nullptr, SPF_PURGEBEFORESPEAK, nullptr);
}
