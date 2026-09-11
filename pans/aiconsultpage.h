#ifndef AICONSULTPAGE_H
#define AICONSULTPAGE_H

#include <QByteArray>
#include <QImage>
#include <QWidget>
#include "../MyTcp/protecol.h"
#include "../MyTcp/cdata.h"
#include <QDateTime>

class CameraSerial;
class DeviceCamera;
class QLabel;
class QLineEdit;
class QPushButton;
class QProgressBar;          // ★ 新增：进度条前置声明
class QScrollArea;
class QThread;
class QVBoxLayout;

class AIConsultPage : public QWidget
{
    Q_OBJECT

public:
    explicit AIConsultPage(QWidget *parent = nullptr);
    ~AIConsultPage() override;

signals:
    void backRequested();
    void consultRequested(const QString &symptom);
    void data_ready(QByteArray data, int size);

private:
    void initLayout();
    QWidget *createLeftArea();
    QWidget *createRightArea();
    QWidget *createBottomBar();

    void addMessage(const QString &text, bool isUser);
    void sendToAgent(const QString &userText);
    void submitText(const QString &text);
    void onSend();
    void onVoicePressed();
    void onVoiceReleased();
    void onVoiceError(const QString &message);

    void initCamera();
    void refreshVideoLabel();

private slots:
    void onCameraFrame(const QImage &frame);
    void onToggleCamera(bool checked);
    void onDetectTongue();
    void onResumeVideo();
    void onUploadImage();
    void onTongueDetected(int classId, float confidence);

private:
    QLabel       *m_videoLabel   = nullptr;
    QPushButton  *m_toggleBtn    = nullptr;
    QPushButton  *m_detectBtn    = nullptr;
    QPushButton  *m_resumeBtn    = nullptr;
    QPushButton  *m_uploadBtn    = nullptr;
    QProgressBar *m_progressBar  = nullptr;   // ★ 新增：浮层进度条（不属于任何布局）
    QPushButton  *m_voiceBtn     = nullptr;
    QLineEdit    *m_inputEdit    = nullptr;
    bool          m_voiceBound   = false;
    QScrollArea  *m_scroll       = nullptr;
    QWidget      *m_chatContent  = nullptr;
    QVBoxLayout  *m_chatLayout   = nullptr;
    QString       m_doctorAvatar;
    QString       m_userAvatar;

    DeviceCamera *m_camera       = nullptr;
    QThread      *m_cameraThread = nullptr;
    CameraSerial *m_serialCtl    = nullptr;
    QThread      *m_serialThread = nullptr;
    QImage        m_latestFrame;
    bool          m_cameraOn     = false;
    QByteArray    m_uploadImageData;

    void deal_img();
};

#endif // AICONSULTPAGE_H