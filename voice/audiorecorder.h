#ifndef AUDIORECORDER_H
#define AUDIORECORDER_H

#include <QByteArray>
#include <QObject>

#include <vector>

class QAudioSource; // Qt6 音频输入源（对应 Qt5 的 QAudioInput）
class QBuffer;      // 内存缓冲区 IO 设备

/**
 * @brief 语音输入录音器（voice/ 语音识别模块）
 *
 * 基于 Qt6 QAudioSource 从默认麦克风录制音频：
 * - 录音格式：16kHz / 单声道 / Int16（SenseVoice ASR 的输入要求）
 * - 录制数据缓存在内存 QByteArray 中，不落盘
 * - 提供 Int16 原始字节 → Float32 归一化样本（[-1.0, 1.0]）的转换
 *   供语音识别引擎（SenseVoiceEngine）使用
 *
 * 移植自参考工程 sense_voice_demo 的 AudioRecorder。
 */
class AudioRecorder : public QObject
{
    Q_OBJECT

public:
    explicit AudioRecorder(QObject *parent = nullptr);
    ~AudioRecorder() override;

    /** 开始录音；成功返回 true（设备可用等），失败发 errorOccurred */
    bool startRecording();

    /** 停止录音：结束写入，按录音时长发 recordingStopped(durationMs) */
    void stopRecording();

    /** 当前是否正在录音 */
    bool isRecording() const;

    /** 取录音数据的 Float32 归一化样本，范围 [-1.0, 1.0]（Int16 除以 32768） */
    std::vector<float> getFloat32AudioData() const;

    /** 静态：系统是否有可用音频输入设备（麦克风） */
    static bool hasAudioInput();

signals:
    void recordingStarted();                       // 录音已开始
    void recordingStopped(qint64 durationMs);      // 录音已结束（含时长）
    void errorOccurred(const QString &errorMsg);   // 录音错误

private:
    QAudioSource *m_audioSource = nullptr; // 麦克风采集源
    QBuffer      *m_buffer      = nullptr; // 内存缓冲（数据写入 m_audioBytes）
    QByteArray    m_audioBytes;            // 录音原始 Int16 字节
    bool          m_isRecording = false;   // 录音状态锁
    qint64        m_startTime   = 0;       // 录音开始时间戳（毫秒）

    static constexpr int SAMPLE_RATE = 16000; // 采样率（SenseVoice 输入标准）
};

#endif // AUDIORECORDER_H
