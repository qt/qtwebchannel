QT = core quick webchannel-private

INCLUDEPATH += ../../webchannel
VPATH += ../../webchannel

SOURCES += \
    plugin.cpp \
    qmlwebchannel.cpp \
    qmlwebchannelattached.cpp \
    qmlwebviewtransport.cpp

HEADERS += \
    qmlwebchannel.h \
    qmlwebchannelattached.h \
    qmlwebviewtransport.h

load(qml_plugin)
