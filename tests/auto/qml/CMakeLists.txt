# Generated from qml.pro.

#####################################################################
## qml Test:
#####################################################################

# Collect test data
file(GLOB_RECURSE test_data_glob
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    data/*)
list(APPEND test_data ${test_data_glob})

qt_add_test(qml
    QMLTEST
    QML_IMPORTPATH
        "${CMAKE_CURRENT_BINARY_DIR}/../../../qml"
        "${CMAKE_CURRENT_SOURCE_DIR}"
    SOURCES
        qml.cpp
        testobject.cpp testobject.h
        testtransport.cpp testtransport.h
        testwebchannel.cpp testwebchannel.h
    LIBRARIES
        Qt::CorePrivate
        Qt::WebChannelPrivate
    PUBLIC_LIBRARIES
        Qt::Gui
        Qt::WebChannel
    TESTDATA ${test_data}
)

#### Keys ignored in scope 1:.:.:qml.pro:<TRUE>:
# DISTFILES = "tst_webchannelseparation.qml"
# IMPORTPATH = "$$OUT_PWD/../../../qml" "$$PWD"
# OTHER_FILES = "Client.qml" "WebChannelTest.qml" "tst_webchannel.qml" "tst_metaobjectpublisher.qml" "tst_bench.qml" "tst_multiclient.qml"
# TEMPLATE = "app"
