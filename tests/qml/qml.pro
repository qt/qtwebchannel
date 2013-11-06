QT += testlib

TARGET = qml

CONFIG += warn_on qmltestcase

IMPORTPATH += $$OUT_PWD/../../src

SOURCES += \
    qml.cpp
