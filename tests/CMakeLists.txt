# Generated from tests.pro.

# special case begin
if(NOT TARGET Qt::Test)
    cmake_minimum_required(VERSION 3.15.0)
    project(QtWebChannelTests VERSION 6.0.0 LANGUAGES C CXX)
    find_package(Qt6 ${PROJECT_VERSION} REQUIRED COMPONENTS BuildInternals Core Test WebChannel)
    find_package(Qt6 ${PROJECT_VERSION} OPTIONAL_COMPONENTS Quick QuickTest)
    qt_set_up_standalone_tests_build()
endif()
# special case end

qt_build_tests()
