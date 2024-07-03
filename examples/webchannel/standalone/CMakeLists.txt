# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(standalone LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/webchannel/standalone")

find_package(Qt6 REQUIRED COMPONENTS Core Gui WebChannel WebSockets Widgets)

qt_add_executable(standalone
    ../shared/websocketclientwrapper.cpp ../shared/websocketclientwrapper.h
    ../shared/websockettransport.cpp ../shared/websockettransport.h
    core.h
    dialog.cpp dialog.h dialog.ui
    main.cpp
)

set_target_properties(standalone PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

# pro2cmake generates invalid cmake syntax here
target_compile_definitions(standalone PUBLIC
    "BUILD_DIR=\"${CMAKE_CURRENT_BINARY_DIR}\""
)

target_link_libraries(standalone PUBLIC
    Qt::Core
    Qt::Gui
    Qt::WebChannel
    Qt::WebSockets
    Qt::Widgets
)

if (NOT CMAKE_CURRENT_BINARY_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/index.html
        COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_CURRENT_SOURCE_DIR}/index.html
                ${CMAKE_CURRENT_BINARY_DIR}/index.html
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/index.html
        VERBATIM
    )
endif()

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/qwebchannel.js
    COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_CURRENT_SOURCE_DIR}/../shared/qwebchannel.js
            ${CMAKE_CURRENT_BINARY_DIR}/qwebchannel.js
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/../shared/qwebchannel.js
    VERBATIM
)

add_custom_target(output_target ALL
    DEPENDS
    ${CMAKE_CURRENT_BINARY_DIR}/index.html
    ${CMAKE_CURRENT_BINARY_DIR}/qwebchannel.js
)

add_dependencies(standalone output_target)

install(TARGETS standalone
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/index.html ${CMAKE_CURRENT_BINARY_DIR}/qwebchannel.js
    DESTINATION "${INSTALL_EXAMPLEDIR}"
)
