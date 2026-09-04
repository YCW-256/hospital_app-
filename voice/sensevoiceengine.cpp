#include "sensevoiceengine.h"

#include <QDebug>
#include <QElapsedTimer>
#include <QFile>
#include <QRegularExpression>

#include <cstdint>
#include <cstring>
#include <string>

extern "C" {
#include "c-api.h"
}

namespace {
/* 剥离 SenseVoice 输出中的语言/情感等特殊标记，如 <|zh|>、<|NEUTRAL|>、<|Speech|> */
QString stripSenseVoiceTags(const QString &text)
{
    return QString(text).remove(QRegularExpression(QStringLiteral("<\\|[^>]*\\|>"))).trimmed();
}
} // namespace

SenseVoiceEngine::SenseVoiceEngine(QObject *parent)
    : QObject(parent)
{
}

SenseVoiceEngine::~SenseVoiceEngine()
{
    if (m_recognizer) {
        DestroyOfflineRecognizer(m_recognizer);
        m_recognizer = nullptr;
    }
    m_modelReady = false;
}

bool SenseVoiceEngine::loadModel(const QString &modelFolderPath)
{
    const QString modelPath = modelFolderPath + QStringLiteral("/model.int8.onnx");
    const QString tokensPath = modelFolderPath + QStringLiteral("/tokens.txt");

    qDebug().noquote() << "[语音识别] 模型目录:" << modelFolderPath;
    qDebug() << "模型文件:" << modelPath << "存在:" << QFile::exists(modelPath)
             << "| 词表文件:" << tokensPath << "存在:" << QFile::exists(tokensPath);

    if (!QFile::exists(modelPath) || !QFile::exists(tokensPath)) {
        emit errorOccurred(QStringLiteral("语音模型缺少 model.int8.onnx / tokens.txt"));
        m_modelReady = false;
        emit modelLoadFinished(false);
        return false;
    }

    // 配置结构体必须先 memset 清零，避免未初始化字段的垃圾值
    SherpaOnnxOfflineRecognizerConfig cfg;
    memset(&cfg, 0, sizeof(cfg));

    // 用局部 std::string 缓存，生命周期覆盖整个 cfg 使用阶段
    const std::string strModelPath = modelPath.toStdString();
    const std::string strTokensPath = tokensPath.toStdString();
    const std::string strLanguage = "auto";   // 自动语言检测
    const std::string strProvider = "cpu";    // CPU 推理（无需 GPU）

    cfg.model_config.sense_voice.model = strModelPath.c_str();
    cfg.model_config.sense_voice.language = strLanguage.c_str();
    cfg.model_config.sense_voice.use_itn = 1;   // ITN：口语数字/标点自动规整
    cfg.model_config.tokens = strTokensPath.c_str();
    cfg.model_config.num_threads = 4;           // 推理线程数
    cfg.model_config.provider = strProvider.c_str();

    m_recognizer = CreateOfflineRecognizer(&cfg);
    if (!m_recognizer) {
        emit errorOccurred(QStringLiteral("SenseVoice 模型初始化失败"));
        m_modelReady = false;
        emit modelLoadFinished(false);
        return false;
    }

    m_modelReady = true;
    qDebug().noquote() << "[语音识别] 模型加载成功";
    emit modelLoadFinished(true);
    return true;
}

bool SenseVoiceEngine::isModelLoaded() const
{
    return m_modelReady;
}

void SenseVoiceEngine::recognizeAudio(std::vector<float> audioData)
{
    if (!m_modelReady || !m_recognizer) {
        emit errorOccurred(QStringLiteral("语音模型尚未加载"));
        return;
    }

    QElapsedTimer timer;
    timer.start();

    // 1. 创建单次识别流，送入波形，解码
    SherpaOnnxOfflineStream *stream = CreateOfflineStream(m_recognizer);
    AcceptWaveformOffline(stream, 16000, audioData.data(), static_cast<int32_t>(audioData.size()));
    DecodeOfflineStream(m_recognizer, stream);

    // 2. 取结果文本（UTF-8）并剥离特殊标记
    const SherpaOnnxOfflineRecognizerResult *res = GetOfflineStreamResult(stream);
    const QString resultText = stripSenseVoiceTags(QString::fromUtf8(res->text));
    const qint64 costMs = timer.elapsed();

    // 3. 释放资源：先结果后流，防止内存泄漏
    DestroyOfflineRecognizerResult(res);
    DestroyOfflineStream(stream);

    qDebug().noquote() << QStringLiteral("[语音识别] 识别完成(%1 ms): %2")
                              .arg(costMs).arg(resultText);
    emit transcriptionFinished(resultText, costMs);
}
