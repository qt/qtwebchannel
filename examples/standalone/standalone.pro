QT += gui webchannel widgets websockets

CONFIG += warn_on

SOURCES += \
    main.cpp

FORMS += \
    dialog.ui

DEFINES += "SOURCE_DIR=\"\\\""$$PWD"\\\"\""
