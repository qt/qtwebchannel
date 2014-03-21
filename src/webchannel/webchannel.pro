TARGET = QtWebChannel
QT = core network
CONFIG += warn_on strict_flags

load(qt_module)

QMAKE_DOCS = $$PWD/doc/qtwebchannel.qdocconf

RESOURCES += \
    resources.qrc

OTHER_FILES = \
    qwebchannel.js

PUBLIC_HEADERS += \
    qwebchannel.h \
    qmessagepassinginterface.h

PRIVATE_HEADERS += \
    qwebchannel_p.h \
    qmetaobjectpublisher_p.h \
    variantargument_p.h \
    signalhandler_p.h

SOURCES += \
    qwebchannel.cpp \
    qmetaobjectpublisher.cpp

qtHaveModule(qml) {
    QT += qml
    DEFINES += HAVE_QML=1
}

HEADERS += $$PUBLIC_HEADERS $$PRIVATE_HEADERS
