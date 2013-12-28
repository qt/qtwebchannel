QT = core quick webchannel-private

INCLUDEPATH += ../../webchannel
VPATH += ../../webchannel

SOURCES += \
    plugin.cpp \
    qmlwebchannel.cpp

HEADERS += \
    qmlwebchannel.h

load(qml_plugin)
