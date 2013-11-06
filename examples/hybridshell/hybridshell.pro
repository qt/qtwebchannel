QT += qml quick

CONFIG += warn_on

SOURCES += \
    main.cpp \
    shell.cpp

HEADERS += \
    shell.h

DEFINES += "SOURCE_DIR=\"\\\""$$PWD"\\\"\""
