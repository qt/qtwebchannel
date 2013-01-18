include(src.pri)

TEMPLATE = lib
TARGET = qwebchannel
TARGETPATH = Qt/labs/WebChannel
QT += qml
CONFIG += qt plugin

TARGET = $$qtLibraryTarget($$TARGET)

# Input
SOURCES += qwebchannel_plugin.cpp
HEADERS += qwebchannel_plugin.h

OTHER_FILES = qmldir \
    qtc_packaging/debian_harmattan/rules \
    qtc_packaging/debian_harmattan/README \
    qtc_packaging/debian_harmattan/copyright \
    qtc_packaging/debian_harmattan/control \
    qtc_packaging/debian_harmattan/compat \
    qtc_packaging/debian_harmattan/changelog \
    webchannel.js \
    qobject.js \
    webchannel-iframe.html \
    MetaObjectPublisher.qml

!equals(_PRO_FILE_PWD_, $$OUT_PWD) {
    copy_qmldir.target = $$OUT_PWD/qmldir
    copy_qmldir.depends = $$_PRO_FILE_PWD_/qmldir
    copy_qmldir.commands = $(COPY_FILE) \"$$replace(copy_qmldir.depends, /, $$QMAKE_DIR_SEP)\" \"$$replace(copy_qmldir.target, /, $$QMAKE_DIR_SEP)\"
    QMAKE_EXTRA_TARGETS += copy_qmldir
    PRE_TARGETDEPS += $$copy_qmldir.target
}

target.path = $$[QT_INSTALL_QML]/$$TARGETPATH

qmldir.files += $$PWD/qmldir $$PWD/MetaObjectPublisher.qml
qmldir.path +=  $$[QT_INSTALL_QML]/$$TARGETPATH

INSTALLS += target qmldir

RESOURCES += \
    resources.qrc
