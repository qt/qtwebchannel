include(src.pri)

TEMPLATE = lib
TARGET = qwebchannel
TARGETPATH = Qt/labs/WebChannel
QT += qml quick
CONFIG += qt plugin

TARGET = $$qtLibraryTarget($$TARGET)

# Input
SOURCES += $$PWD/qwebchannel_plugin.cpp
HEADERS += $$PWD/qwebchannel_plugin.h

RESOURCES += \
    resources.qrc

OTHER_FILES = qmldir \
    webchannel.js \
    qobject.js \
    MetaObjectPublisher.qml

target.path = $$[QT_INSTALL_QML]/$$TARGETPATH

# extra files that need to be deployed to $$TARGETPATH
DEPLOY_FILES = \
    qmldir \
    MetaObjectPublisher.qml

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
