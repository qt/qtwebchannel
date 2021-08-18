# Generated from webchannel.pro.

#####################################################################
## tst_webchannel Test:
#####################################################################

qt_internal_add_test(tst_webchannel
    SOURCES
        tst_webchannel.cpp tst_webchannel.h
    INCLUDE_DIRECTORIES
        ../../../src/webchannel
    PUBLIC_LIBRARIES
        Qt::CorePrivate
        Qt::WebChannelPrivate
)

## Scopes:
#####################################################################

qt_internal_extend_target(tst_webchannel CONDITION TARGET Qt::Qml
    DEFINES
        WEBCHANNEL_TESTS_CAN_USE_JS_ENGINE
    PUBLIC_LIBRARIES
        Qt::Qml
)

qt_internal_extend_target(tst_webchannel CONDITION TARGET Qt::Concurrent
    DEFINES
        WEBCHANNEL_TESTS_CAN_USE_CONCURRENT
    PUBLIC_LIBRARIES
        Qt::Concurrent
)
