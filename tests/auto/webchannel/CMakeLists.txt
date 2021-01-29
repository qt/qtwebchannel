# Generated from webchannel.pro.

#####################################################################
## tst_webchannel Test:
#####################################################################

qt_add_test(tst_webchannel
    SOURCES
        tst_webchannel.cpp tst_webchannel.h
    INCLUDE_DIRECTORIES
        ../../../src/webchannel
    LIBRARIES
        Qt::CorePrivate
        Qt::WebChannelPrivate
    PUBLIC_LIBRARIES
        Qt::WebChannel
)

## Scopes:
#####################################################################

qt_extend_target(tst_webchannel CONDITION TARGET Qt::Qml
    DEFINES
        WEBCHANNEL_TESTS_CAN_USE_JS_ENGINE
    PUBLIC_LIBRARIES
        Qt::Qml
)
