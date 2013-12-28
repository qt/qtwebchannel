TARGET = QtWebChannel
QT = core network
CONFIG += warn_on strict_flags

load(qt_module)

RESOURCES += \
    resources.qrc

OTHER_FILES = \
    qwebchannel.js

PUBLIC_HEADERS += \
    qwebchannel.h

PRIVATE_HEADERS += \
    qwebchannel_p.h \
    qmetaobjectpublisher_p.h \
    qwebsocketserver_p.h \
    qwebchannelsocket_p.h \
    variantargument_p.h \
    signalhandler_p.h

SOURCES += \
    qwebchannel.cpp \
    qmetaobjectpublisher.cpp \
    qwebsocketserver.cpp \
    qwebchannelsocket.cpp

HEADERS += $$PUBLIC_HEADERS $$PRIVATE_HEADERS
