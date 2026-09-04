#include "speechrecognizer.h"

#include "audiorecorder.h"
#include "sensevoiceengine.h"

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QMetaObject>
#include <QMetaType>
#include <QStringList>
#include <QThread>
#include <QTimer>

// std::vector<float> 需注册元类型才能在跨线程（QueuedConnection）信号中传递
Q_DECLARE_METATYPE(std::vector<float>)

SpeechRecognizer *SpeechRecognizer::m_instance = nullptr;

SpeechRecognizer *SpeechRecognizer::instance()
{
    if (!m_instance)
        m_instance = new SpeechRecognizer; // 主线程创建，无父对象（进程结束时随进程释放）
    return m_instance;
}

QString SpeechRecognizer::modelDir()
{
#ifdef VOICE_MODEL_DIR
    const QString macroPath = QString::fromUtf8(VOICE_MODEL_DIR);
    if (QFile::exists(macroPath + QStringLiteral("/model.int8.onnx")))
        return macroPath;
#endif
    // 运行期相对路径兜底：可执行文件旁的 voice/model、上级目录 voice/model 等
    const QStringList bases = {
        QDir::currentPath(),
        QCoreApplication::applicationDirPath(),
        QDir::cleanPath(QCoreApplication::applicationDirPath() + QStringLiteral("/..")),
    };
    for (const QString &base : bases) {
        const QString candidate = QDir(base).filePath(QStringLiteral("voice/model"));
        if (QFile::exists(candidate + QStringLiteral("/model.int8.onnx")))
            return candidate;
    }
#ifdef VOICE_MODEL_DIR
    return macroPath; // 都找不到则回退宏路径，让引擎报缺失文件名
#else
    return QDir::currentPath() + QStringLiteral("/voice/model");
#endif
}

SpeechRecognizer::SpeechRecognizer(QObject *parent)
    : QObject(parent)
{
    qRegisterMetaType<std::vector<float>>();

    // 录音器在主线程（麦克风采集是轻量 IO）
    m_recorder = new AudioRecorder(this);

    // 识别引擎放入独立工作线程：模型加载与推理都耗时，避免卡住界面
    m_engine = new SenseVoiceEngine;
    m_thread = new QThread(this);
    m_engine->moveToThread(m_thread);

    // 主线程录音结束 → 送工作线程执行识别（跨线程 QueuedConnection）
    connect(this, &SpeechRecognizer::recognizeRequested,
            m_engine, &SenseVoiceEngine::recognizeAudio, Qt::QueuedConnection);
    // 引擎线程回主线程的信号（加载结果 / 错误 / 识别完成）
    connect(m_engine, &SenseVoiceEngine::modelLoadFinished,
            this, &SpeechRecognizer::onModelLoaded);
    connect(m_engine, &SenseVoiceEngine::errorOccurred,
            this, &SpeechRecognizer::onEngineError);
    connect(m_engine, &SenseVoiceEngine::transcriptionFinished,
            this, &SpeechRecognizer::onEngineResult);

    m_thread->start();

    // 程序退出前收尾：停录音、退出并回收引擎线程
    connect(qApp, &QCoreApplication::aboutToQuit, this, [this] {
        if (m_recording) {
            m_recorder->stopRecording();
            m_recording = false;
        }
        m_requester = nullptr;
        m_busy = false;
        if (m_thread && m_thread->isRunning()) {
            m_thread->quit();
            m_thread->wait();
        }
        if (m_engine) {
            delete m_engine;
            m_engine = nullptr;
        }
        if (m_thread) {
            delete m_thread;
            m_thread = nullptr;
        }
    });
}

void SpeechRecognizer::startListening(QObject *requester)
{
    if (!requester)
        return;
    if (m_requester || m_recording || m_busy)
        return; // 已有一次进行中，忽略重复按下

    // 模型未就绪时把"加载"排到引擎线程（首次约 1~3s）；因识别请求也在同一线程
    // 排队且 FIFO，加载一定先于本次识别执行，因此录音可立即开始、无需等待加载。
    if (!m_modelReady && !m_modelLoading) {
        m_modelLoading = true;
        QMetaObject::invokeMethod(m_engine, [this] {
            m_engine->loadModel(modelDir());
        }, Qt::QueuedConnection);
    }

    if (!AudioRecorder::hasAudioInput()) {
        emit errorOccurred(requester, QStringLiteral("未检测到麦克风"));
        return;
    }

    m_requester = requester;
    if (m_recorder->startRecording()) {
        m_recording = true;
        emit recordingChanged(true);
    } else {
        m_requester = nullptr;
        emit errorOccurred(requester, QStringLiteral("无法打开麦克风，请检查录音设备"));
    }
}

void SpeechRecognizer::stopListening()
{
    if (!m_recording || !m_requester)
        return;

    m_recorder->stopRecording();
    m_recording = false;
    emit recordingChanged(false);

    QObject *requester = m_requester; // m_requester 保留到识别完成，用于结果路由
    std::vector<float> data = m_recorder->getFloat32AudioData();
    if (data.empty()) {
        m_requester = nullptr;
        emit errorOccurred(requester, QStringLiteral("没有录到声音，请按住重新说一次"));
        return;
    }

    m_busy = true;
    emit busyChanged(true);
    emit recognizeRequested(std::move(data)); // 排队到工作线程识别
}

void SpeechRecognizer::cancelFor(QObject *requester)
{
    if (requester != m_requester)
        return;
    m_requester = nullptr;
    if (m_recording) {
        m_recorder->stopRecording();
        m_recording = false;
        emit recordingChanged(false);
    }
    if (m_busy) {
        m_busy = false;
        emit busyChanged(false);
    }
}

void SpeechRecognizer::onModelLoaded(bool ok)
{
    m_modelLoading = false;
    m_modelReady = ok;
    if (!ok)
        qDebug().noquote() << "[语音识别] 模型加载失败（后续识别将提示错误）";
}

void SpeechRecognizer::onEngineError(const QString &errMsg)
{
    qDebug().noquote() << "[语音识别] 引擎错误:" << errMsg;
    // 仅"识别待结果（已松开、busy=true）"阶段出错才转发给界面提示；
    // 加载期 / 录音期的引擎错误只记日志，最终结果或错误由识别阶段统一给出，
    // 避免录音途中收到加载错误而误清 requester、忘停录音。
    if (m_requester && m_busy) {
        QObject *requester = m_requester;
        m_requester = nullptr;
        m_busy = false;
        emit busyChanged(false);
        emit errorOccurred(requester, errMsg);
    }
}

void SpeechRecognizer::onEngineResult(const QString &text, qint64 elapsedMs)
{
    Q_UNUSED(elapsedMs)
    if (!m_requester)
        return; // 发起方已取消/销毁，丢弃结果

    QObject *requester = m_requester;
    m_requester = nullptr;
    m_busy = false;
    emit busyChanged(false);

    const QString trimmed = text.trimmed();
    if (trimmed.isEmpty())
        emit errorOccurred(requester, QStringLiteral("未能识别到有效语音，请重试"));
    else
        emit recognized(requester, trimmed);
}
