# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

# Generated from webchannel.pro.

#####################################################################
## tst_webchannel Test:
#####################################################################

qt_internal_add_test(tst_webchannel
    SOURCES
        tst_webchannel.cpp tst_webchannel.h
    INCLUDE_DIRECTORIES
        ../../../src/webchannel
    LIBRARIES
        Qt::CorePrivate
        Qt::WebChannelPrivate
)

## Scopes:
#####################################################################

qt_internal_extend_target(tst_webchannel CONDITION TARGET Qt::Qml
    DEFINES
        WEBCHANNEL_TESTS_CAN_USE_JS_ENGINE
    LIBRARIES
        Qt::Qml
)

qt_internal_extend_target(tst_webchannel CONDITION TARGET Qt::Concurrent
    DEFINES
        WEBCHANNEL_TESTS_CAN_USE_CONCURRENT
    LIBRARIES
        Qt::Concurrent
)
