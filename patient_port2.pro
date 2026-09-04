QT += core gui widgets svg network serialport multimedia

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

TARGET = patient_port2

# 入口留在根目录；界面文件统一放在 pans/（含子控件 childs/），
# 非界面文件按类别放在同级目录：agent/ 智能体、audio/ 语音、core/ 基础工具、device/ 硬件设备
SOURCES += \
    MyTcp/cdata.cpp \
    MyTcp/socketlink.cpp \
    Task/businesstask.cpp \
    Task/getdoctorinfotask.cpp \
    Tool/myutils.cpp \
    Tool/readutil.cpp \
    main.cpp \
    pans/loginwindow.cpp \
    pans/registerwindow.cpp \
    pans/mainwindow.cpp \
    pans/topnavbar.cpp \
    pans/functionbutton.cpp \
    pans/paymentpage.cpp \
    pans/appointmentpage.cpp \
    pans/feepage.cpp \
    pans/aiconsultpage.cpp \
    pans/personalcenterpage.cpp \
    pans/modifyinfopage.cpp \
    pans/aiassistantpopup.cpp \
    pans/childs/appointmentconfirmpopup.cpp \
    pans/childs/circularavatar.cpp \
    pans/childs/doctorcard.cpp \
    pans/childs/chatbubble.cpp \
    agent/deepseekagent.cpp \
    agent/myagent.cpp \
    agent/consultagent.cpp \
    audio/ttsplayer.cpp \
    core/iconfactory.cpp \
    core/uistyle.cpp \
    device/cameraserial.cpp \
    device/devicecamera.cpp \
    voice/audiorecorder.cpp \
    voice/sensevoiceengine.cpp \
    voice/speechrecognizer.cpp

HEADERS += \
    MyTcp/cdata.h \
    MyTcp/protecol.h \
    MyTcp/socketlink.h \
    Task/businesstask.h \
    Task/getdoctorinfotask.h \
    Tool/myutils.h \
    Tool/readutil.h \
    pans/loginwindow.h \
    pans/registerwindow.h \
    pans/mainwindow.h \
    pans/topnavbar.h \
    pans/functionbutton.h \
    pans/paymentpage.h \
    pans/appointmentpage.h \
    pans/feepage.h \
    pans/aiconsultpage.h \
    pans/personalcenterpage.h \
    pans/modifyinfopage.h \
    pans/aiassistantpopup.h \
    pans/childs/appointmentconfirmpopup.h \
    pans/childs/circularavatar.h \
    pans/childs/doctorcard.h \
    pans/childs/chatbubble.h \
    agent/deepseekagent.h \
    agent/myagent.h \
    agent/consultagent.h \
    audio/ttsplayer.h \
    core/iconfactory.h \
    core/uistyle.h \
    device/cameraserial.h \
    device/devicecamera.h \
    voice/audiorecorder.h \
    voice/sensevoiceengine.h \
    voice/speechrecognizer.h

# 纯代码构建，不使用 .ui 文件

# Windows SAPI 语音播放（与 patient_port 原工程一致）
win32 {
    LIBS += -lole32 -lsapi -luuid
}

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DESTDIR = $$PWD/bin

RESOURCES += \
    resource.qrc

# ==================== 语音识别模块（voice/） ====================
# Sherpa-ONNX C API 头文件目录（c-api.h / cargs.h，sensevoiceengine.cpp 内 extern "C" 引入）
INCLUDEPATH += $$PWD/voice/sherpa/include

# 链接预编译库 sherpa-onnx-c-api（voice/sherpa/lib，MSVC 导入库 .lib；
# Qt 的 llvm-mingw 工具链使用 lld，可直接链接 MSVC 导入库）
LIBS += $$PWD/voice/sherpa/lib/sherpa-onnx-c-api.lib

# 把 SenseVoice 模型目录（编译期默认 voice/model）传给 C++，运行期不再手填路径
DEFINES += VOICE_MODEL_DIR=\\\"$$PWD/voice/model\\\"

win32 {
    # 语音识别运行所需 DLL（构建后逐个复制到 exe 输出目录，避免运行时报缺失 DLL）
    VOICE_DLLS = \
        $$PWD/voice/sherpa/lib/sherpa-onnx-c-api.dll \
        $$PWD/voice/sherpa/lib/onnxruntime.dll \
        $$PWD/voice/sherpa/lib/onnxruntime_providers_shared.dll \
        $$PWD/voice/sherpa/lib/cargs.dll

    for(dll, VOICE_DLLS) {
        QMAKE_POST_LINK += $$escape_expand(\n\t)$$QMAKE_COPY $$shell_path($$dll) $$shell_path($$DESTDIR)
    }
}
