QT += gui webchannel widgets websockets

CONFIG += warn_on

SOURCES += \
    main.cpp \
    websockettransport.cpp \
    websocketclientwrapper.cpp

HEADERS += \
    websockettransport.h \
    websocketclientwrapper.h

FORMS += \
    dialog.ui

DEFINES += "BUILD_DIR=\"\\\""$$OUT_PWD"\\\"\""

exampleassets.files += \
    index.html
exampleassets.path = $$[QT_INSTALL_EXAMPLES]/qwebchannel/standalone
include(../exampleassets.pri)
