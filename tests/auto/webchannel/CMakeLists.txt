# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    cmake_minimum_required(VERSION 3.16)
    project(tst_webchannel LANGUAGES CXX)
    find_package(Qt6BuildInternals REQUIRED COMPONENTS STANDALONE_TEST)
endif()

qt_internal_add_test(tst_webchannel
    SOURCES
        tst_webchannel.cpp tst_webchannel.h
    LIBRARIES
        Qt::CorePrivate
        Qt::WebChannelPrivate
        Qt::TestPrivate
)

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
