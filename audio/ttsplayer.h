#ifndef TTSPLAYER_H
#define TTSPLAYER_H

#include <QString>

/**
 * @brief Windows SAPI 语音播放（TTS）
 *
 * 移植自 patient_port 工程 EmergencyWidget::ttsSpeak()，
 * 通过 Windows SAPI（ISpVoice）异步朗读文本，供 AI 信息助手播报回复。
 */
class TtsPlayer
{
public:
    static TtsPlayer *instance();

    /** 异步语音播放一段文本 */
    void speak(const QString &text);

    /** 停止当前播放 */
    void stop();

private:
    TtsPlayer();
    ~TtsPlayer();

    void ensureVoice(); // 惰性创建 ISpVoice

    void  *m_voice = nullptr;        // ISpVoice*（避免在头文件引入 windows 头）
    bool   m_comInitialized = false; // 本对象是否完成了 COM 初始化
    static TtsPlayer *m_instance;
};

#endif // TTSPLAYER_H
