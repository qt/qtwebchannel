TARGET = QtWebChannel
QT = core network
CONFIG += warn_on strict_flags

load(qt_module)

RESOURCES += \
    resources.qrc

OTHER_FILES = \
    webchannel.js \
    qobject.js

PUBLIC_HEADERS += \
    qwebchannel.h \
    qmetaobjectpublisher.h

PRIVATE_HEADERS += \
    qwebsocketserver_p.h \
    variantargument_p.h \
    signalhandler_p.h

SOURCES += \
    qwebchannel.cpp \
    qmetaobjectpublisher.cpp \
    qwebsocketserver.cpp

HEADERS += $$PUBLIC_HEADERS $$PRIVATE_HEADERS
