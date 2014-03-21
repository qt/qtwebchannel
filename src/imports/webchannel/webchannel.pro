QT = core quick webchannel-private

INCLUDEPATH += ../../webchannel
VPATH += ../../webchannel

SOURCES += \
    plugin.cpp \
    qmlwebchannel.cpp \
    qmlwebchannelattached.cpp

HEADERS += \
    qmlwebchannel.h \
    qmlwebchannelattached.h

load(qml_plugin)
