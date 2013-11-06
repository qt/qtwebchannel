include(src.pri)

TEMPLATE = lib
TARGET = qwebchannel
TARGETPATH = Qt/labs/WebChannel
QT += qml quick
CONFIG += qt plugin

TARGET = $$qtLibraryTarget($$TARGET)

# Input
SOURCES += qwebchannel_plugin.cpp
HEADERS += qwebchannel_plugin.h

RESOURCES += \
    resources.qrc

OTHER_FILES = qmldir \
    qtc_packaging/debian_harmattan/rules \
    qtc_packaging/debian_harmattan/README \
    qtc_packaging/debian_harmattan/copyright \
    qtc_packaging/debian_harmattan/control \
    qtc_packaging/debian_harmattan/compat \
    qtc_packaging/debian_harmattan/changelog \
    webchannel.js \
    qobject.js \
    MetaObjectPublisher.qml \
    WebChannel.qml

target.path = $$[QT_INSTALL_QML]/$$TARGETPATH

# extra files that need to be deployed to $$TARGETPATH
DEPLOY_FILES = \
    qmldir \
    MetaObjectPublisher.qml \
    WebChannel.qml

for(FILE, DEPLOY_FILES): qmldir.files += $$PWD/$$FILE
qmldir.path +=  $$[QT_INSTALL_QML]/$$TARGETPATH

INSTALLS += target qmldir

# ensure that plugin is put into the correct folder structure
DESTDIR = $$TARGETPATH

# copy files in order to run tests without calling make install first
for(FILE, DEPLOY_FILES) {
    PRE_TARGETDEPS += $$PWD/$$FILE
    QMAKE_POST_LINK += $$quote($$QMAKE_COPY \"$$PWD/$$FILE\" $$OUT_PWD/$$TARGETPATH$$escape_expand(\n\t))
}
