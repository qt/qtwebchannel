TARGET = QtWebChannel
QT = core network websockets
CONFIG += warn_on strict_flags

load(qt_module)

RESOURCES += \
    resources.qrc

OTHER_FILES = \
    qwebchannel.js

PUBLIC_HEADERS += \
    qwebchannel.h \
    qwebchanneltransport.h \
    qwebsockettransport.h

PRIVATE_HEADERS += \
    qwebchannel_p.h \
    qmetaobjectpublisher_p.h \
    qwebsockettransport_p.h \
    variantargument_p.h \
    signalhandler_p.h

SOURCES += \
    qwebchannel.cpp \
    qmetaobjectpublisher.cpp \
    qwebsockettransport.cpp

qtHaveModule(qml) {
    QT += qml
    DEFINES += HAVE_QML=1
}

HEADERS += $$PUBLIC_HEADERS $$PRIVATE_HEADERS