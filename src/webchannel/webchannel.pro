TARGET = QtWebChannel
QT = core
CONFIG += warn_on strict_flags

load(qt_module)

RESOURCES += \
    resources.qrc

OTHER_FILES = \
    qwebchannel.js

PUBLIC_HEADERS += \
    qwebchannel.h \
    qwebchannelabstracttransport.h

PRIVATE_HEADERS += \
    qwebchannel_p.h \
    qmetaobjectpublisher_p.h \
    variantargument_p.h \
    signalhandler_p.h

SOURCES += \
    qwebchannel.cpp \
    qmetaobjectpublisher.cpp \
    qwebchannelabstracttransport.cpp

qtHaveModule(qml) {
    QT += qml
    DEFINES += HAVE_QML=1

    SOURCES += \
        qmlwebchannel.cpp \
        qmlwebchannelattached.cpp

    PRIVATE_HEADERS += \
        qmlwebchannel_p.h \
        qmlwebchannelattached_p.h
}

HEADERS += $$PUBLIC_HEADERS $$PRIVATE_HEADERS
