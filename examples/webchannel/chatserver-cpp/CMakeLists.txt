# Generated from chatserver-cpp.pro.

cmake_minimum_required(VERSION 3.16)
project(chatserver LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
  set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/webchannel/chatserver-cpp")

find_package(Qt6 COMPONENTS Core)
find_package(Qt6 COMPONENTS WebSockets)
find_package(Qt6 COMPONENTS WebChannel)

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
