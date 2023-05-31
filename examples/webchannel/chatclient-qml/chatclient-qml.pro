TEMPLATE = app
TARGET = qmlchatclient

QT += quick widgets

SOURCES += main.cpp

RESOURCES += \
    ../shared/shared.qrc \
    LoginForm.ui.qml \
    MainForm.ui.qml \
    qmlchatclient.qml

QML_IMPORT_PATH = $$PWD/../shared

target.path = $$[QT_INSTALL_EXAMPLES]/webchannel/chatclient-qml
INSTALLS += target
