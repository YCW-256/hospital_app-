#ifndef SENSEVOICEENGINE_H
#define SENSEVOICEENGINE_H

#include <QObject>
#include <QString>

#include <vector>

// ============================================================
// Sherpa-ONNX C API 核心结构体前向声明（具体定义在其 C 头文件 c-api.h，
// 头文件中不引入完整 C 依赖，仅在本类 .cpp 内通过 extern "C" 引入）。
// ============================================================

// 离线识别器：管理模型加载、解码配置与识别资源
typedef struct SherpaOnnxOfflineRecognizer SherpaOnnxOfflineRecognizer;

// 单条音频的识别上下文
typedef struct SherpaOnnxOfflineStream SherpaOnnxOfflineStream;

// 识别结果（转录文本等）
typedef struct SherpaOnnxOfflineRecognizerResult SherpaOnnxOfflineRecognizerResult;

/**
 * @brief 基于 Sherpa-ONNX 的 SenseVoice 离线语音识别引擎（voice/ 语音识别模块）
 *
 * - 加载 SenseVoice ONNX 模型（voice/model/，需含 model.int8.onnx + tokens.txt）
 * - 语言自动检测（language="auto"）、ITN 开启（数字/标点自动规整）
 * - 输入要求：16kHz Float32 波形（由 AudioRecorder::getFloat32AudioData() 产出）
 * - 结果文本按 UTF-8 转 QString，并剥离 <|...|> 语言/情感等特殊标记
 *
 * 移植自参考工程 sense_voice_demo 的 SenseVoiceEngine。
 *
 * 线程约定：识别是同步阻塞调用，应把本对象 moveToThread 到工作线程，
 * 通过跨线程 QueuedConnection 触发 recognizeAudio()，避免阻塞 UI。
 */
class SenseVoiceEngine : public QObject
{
    Q_OBJECT

public:
    explicit SenseVoiceEngine(QObject *parent = nullptr);
    ~SenseVoiceEngine() override;

    /**
     * @brief 加载模型（构造时不加载；首次使用语音前调用）
     * @param modelFolderPath 模型文件夹（须含 model.int8.onnx 与 tokens.txt）
     * @return true 加载成功
     *
     * 说明：属直接方法调用（非槽事件），在调用线程同步执行。
     */
    bool loadModel(const QString &modelFolderPath);

    /** 模型是否已加载就绪 */
    bool isModelLoaded() const;

    /**
     * @brief 对一段 16kHz Float32 音频执行离线识别
     * @param audioData 归一化波形样本 [-1.0, 1.0]
     *
     * 典型用法：通过跨线程信号槽（Qt::QueuedConnection）让本方法在
     * 工作线程执行，识别完成后发 transcriptionFinished 回主线程。
     */
    void recognizeAudio(std::vector<float> audioData);

signals:
    /** 识别完成（text 已剥离特殊标记；elapsedMs 为识别耗时） */
    void transcriptionFinished(const QString &text, qint64 elapsedMs);

    /** 模型加载结束：ok=true 加载成功（跨线程 QueuedConnection 通知状态） */
    void modelLoadFinished(bool ok);

    /** 出错（模型缺失 / 初始化失败 / 未加载就识别等） */
    void errorOccurred(const QString &errMsg);

private:
    SherpaOnnxOfflineRecognizer *m_recognizer = nullptr; // 识别器实例
    bool m_modelReady = false;                           // 模型就绪标志
};

#endif // SENSEVOICEENGINE_H
