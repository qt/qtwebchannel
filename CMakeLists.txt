cmake_minimum_required(VERSION 3.15.0)

project(QtWebChannel
    VERSION 6.0.0
    DESCRIPTION "Qt WebChannel Libraries"
    HOMEPAGE_URL "https://qt.io/"
    LANGUAGES CXX C
)

find_package(Qt6 ${PROJECT_VERSION} CONFIG REQUIRED COMPONENTS BuildInternals Core)
find_package(Qt6 ${PROJECT_VERSION} CONFIG OPTIONAL_COMPONENTS Quick Test QuickTest WebSockets)

qt_build_repo()
