# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(standalone LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Network WebChannel WebSockets Widgets)

qt_add_executable(standalone
    websocketclientwrapper.cpp websocketclientwrapper.h
    websockettransport.cpp websockettransport.h
    core.h
    dialog.cpp dialog.h dialog.ui
    httpserver.cpp httpserver.h
    main.cpp
)

set_target_properties(standalone PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(standalone PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Network
    Qt::WebChannel
    Qt::WebSockets
    Qt::Widgets
)

set_source_files_properties("${QT_QWEBCHANNEL_JS_PATH}" PROPERTIES QT_RESOURCE_ALIAS "qwebchannel.js")

qt_add_resources(standalone "standalone_resources"
    PREFIX "/"
    FILES
        "index.html"
        "${QT_QWEBCHANNEL_JS_PATH}"
)
