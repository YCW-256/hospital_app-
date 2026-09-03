QT += core gui widgets svg network

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

TARGET = patient_port2

# 入口留在根目录；界面文件统一放在 pans/（含子控件 childs/），
# 非界面文件按类别放在同级目录：agent/ 智能体、audio/ 语音、core/ 基础工具
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
    pans/childs/circularavatar.cpp \
    pans/childs/doctorcard.cpp \
    pans/childs/chatbubble.cpp \
    agent/deepseekagent.cpp \
    agent/myagent.cpp \
    agent/consultagent.cpp \
    audio/ttsplayer.cpp \
    core/iconfactory.cpp \
    core/uistyle.cpp

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
    pans/childs/circularavatar.h \
    pans/childs/doctorcard.h \
    pans/childs/chatbubble.h \
    agent/deepseekagent.h \
    agent/myagent.h \
    agent/consultagent.h \
    audio/ttsplayer.h \
    core/iconfactory.h \
    core/uistyle.h

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
