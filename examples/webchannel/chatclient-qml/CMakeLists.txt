# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(qmlchatclient LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Quick Widgets)

qt_standard_project_setup()

qt_add_executable(qmlchatclient
    main.cpp
)

set_target_properties(qmlchatclient PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(qmlchatclient PRIVATE
    Qt::Core
    Qt::Gui
    Qt::Quick
    Qt::Widgets
)

# Resources:
set(data_resource_files
    "qmlchatclient.qml"
    "LoginForm.ui.qml"
    "MainForm.ui.qml"
    "../shared/qwebchannel.js"
)

qt6_add_resources(qmlchatclient "data"
    PREFIX
        "/"
    FILES
        ${data_resource_files}
)
