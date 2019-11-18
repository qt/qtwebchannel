# Generated from standalone.pro.

cmake_minimum_required(VERSION 3.14)
project(standalone LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(INSTALL_EXAMPLEDIR "examples/webchannel/standalone")

find_package(Qt6 COMPONENTS Core)
find_package(Qt6 COMPONENTS Gui)
find_package(Qt6 COMPONENTS WebChannel)
find_package(Qt6 COMPONENTS Widgets)
find_package(Qt6 COMPONENTS WebSockets)

add_qt_gui_executable(standalone
    ../shared/websocketclientwrapper.cpp ../shared/websocketclientwrapper.h
    ../shared/websockettransport.cpp ../shared/websockettransport.h
    core.h
    dialog.cpp dialog.h dialog.ui
    main.cpp
)
# special case begin
# pro2cmake generates invalid cmake syntax here
target_compile_definitions(standalone PUBLIC
    "BUILD_DIR=\"${CMAKE_CURRENT_BINARY_DIR}\""
)
# special case end
target_link_libraries(standalone PUBLIC
    Qt::Core
    Qt::Gui
    Qt::WebChannel
    Qt::WebSockets
    Qt::Widgets
)

install(TARGETS standalone
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
