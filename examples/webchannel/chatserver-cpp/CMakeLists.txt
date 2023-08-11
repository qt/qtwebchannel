# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(chatserver LANGUAGES CXX)

if (ANDROID)
    message(FATAL_ERROR "This project cannot be built on Android.")
endif()

set(CMAKE_AUTOMOC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/webchannel/chatserver-cpp")

find_package(Qt6 REQUIRED COMPONENTS Core WebChannel WebSockets)

qt_add_executable(chatserver
    ../shared/websocketclientwrapper.cpp ../shared/websocketclientwrapper.h
    ../shared/websockettransport.cpp ../shared/websockettransport.h
    chatserver.cpp chatserver.h
    main.cpp
)

set_target_properties(chatserver PROPERTIES
    WIN32_EXECUTABLE FALSE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(chatserver PUBLIC
    Qt::Core
    Qt::WebChannel
    Qt::WebSockets
)

install(TARGETS chatserver
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
