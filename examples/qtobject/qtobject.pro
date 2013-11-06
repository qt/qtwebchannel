QT += qml quick

CONFIG += warn_on

SOURCES += \
    main.cpp \
    testobject.cpp

HEADERS += \
    testobject.h

DEFINES += "SOURCE_DIR=\"\\\""$$PWD"\\\"\""
