#include "audiorecorder.h"

#include <QAudioDevice>
#include <QAudioFormat>
#include <QAudioSource>
#include <QBuffer>
#include <QDateTime>
#include <QDebug>
#include <QMediaDevices>

AudioRecorder::AudioRecorder(QObject *parent)
    : QObject(parent)
{
}

AudioRecorder::~AudioRecorder()
{
    if (m_isRecording)
        stopRecording();
    delete m_audioSource; // delete nullptr 安全
    delete m_buffer;
}

bool AudioRecorder::hasAudioInput()
{
    return !QMediaDevices::defaultAudioInput().isNull();
}

bool AudioRecorder::startRecording()
{
    if (m_isRecording)
        return false; // 已在录音，拒绝重复启动

    // 1. 配置录音格式：16kHz / 单声道 / Int16（SenseVoice 输入要求）
    QAudioFormat format;
    format.setSampleRate(SAMPLE_RATE);
    format.setChannelCount(1);
    format.setSampleFormat(QAudioFormat::Int16);

    // 2. 取默认音频输入设备
    QAudioDevice deviceInfo = QMediaDevices::defaultAudioInput();
    if (deviceInfo.isNull()) {
        emit errorOccurred(QStringLiteral("未找到音频输入设备"));
        return false;
    }

    // 3. 部分设备不支持 16kHz/Int16，回退到设备首选格式（识别结果可能受影响）
    if (!deviceInfo.isFormatSupported(format)) {
        format = deviceInfo.preferredFormat();
        qWarning() << "语音识别录音回退到设备首选格式:"
                   << format.sampleRate() << "Hz";
    }

    // 4. 清理上一次的资源与数据
    delete m_audioSource;
    delete m_buffer;
    m_audioSource = nullptr;
    m_buffer = nullptr;
    m_audioBytes.clear();

    // 5. 创建音频源并绑定内存缓冲（缓冲预留给 5 秒）
    m_audioSource = new QAudioSource(deviceInfo, format, this);
    m_audioSource->setBufferSize(SAMPLE_RATE * 2 * 5);
    m_buffer = new QBuffer(&m_audioBytes, this);
    m_buffer->open(QIODevice::WriteOnly);

    // 6. 开始采集，音频自动写入内存缓冲
    m_audioSource->start(m_buffer);
    m_isRecording = true;
    m_startTime = QDateTime::currentMSecsSinceEpoch();
    qDebug().noquote() << "[语音识别] 开始录音";
    emit recordingStarted();
    return true;
}

void AudioRecorder::stopRecording()
{
    if (!m_isRecording)
        return;

    m_audioSource->stop(); // 停止采集
    m_buffer->close();     // 刷新缓冲
    m_isRecording = false;

    qint64 duration = QDateTime::currentMSecsSinceEpoch() - m_startTime;
    qDebug().noquote() << "[语音识别] 录音结束" << duration << "ms,"
                       << m_audioBytes.size() << "bytes";
    emit recordingStopped(duration);
}

bool AudioRecorder::isRecording() const
{
    return m_isRecording;
}

std::vector<float> AudioRecorder::getFloat32AudioData() const
{
    const int count = m_audioBytes.size() / static_cast<int>(sizeof(int16_t));
    std::vector<float> result(count);

    const auto *src = reinterpret_cast<const int16_t *>(m_audioBytes.constData());
    for (int i = 0; i < count; ++i)
        result[i] = static_cast<float>(src[i]) / 32768.0f; // 归一化到 [-1, 1]
    return result;
}
