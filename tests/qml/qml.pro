QT += testlib webchannel
TEMPLATE = app
TARGET = qml

CONFIG += warn_on qmltestcase

# TODO: running tests without requiring make install
IMPORTPATH += $$OUT_PWD/../../qml $$PWD

SOURCES += \
    qml.cpp

OTHER_FILES += \
    WebChannelTest.qml \
    tst_webchannel.qml \
    tst_metaobjectpublisher.qml \
    tst_bench.qml

TESTDATA = data/*
