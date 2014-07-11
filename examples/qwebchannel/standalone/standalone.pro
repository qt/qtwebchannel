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

DEFINES += "SOURCE_DIR=\"\\\""$$PWD"\\\"\""

EXAMPLE_FILES += index.html
