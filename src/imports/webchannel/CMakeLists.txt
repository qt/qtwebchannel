# Generated from webchannel.pro.

#####################################################################
## webchannel Plugin:
#####################################################################

add_qml_module(declarative_webchannel # special case
    URI "QtWebChannel"
    VERSION "1.${CMAKE_PROJECT_VERSION_MINOR}"
    CLASSNAME QWebChannelPlugin
    SOURCES
        plugin.cpp
    INCLUDE_DIRECTORIES
        ../../webchannel
    LIBRARIES
        Qt::WebChannelPrivate
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::Quick
        Qt::WebChannel
)

#### Keys ignored in scope 1:.:.:webchannel.pro:<TRUE>:
# IMPORT_VERSION = "1.$$QT_MINOR_VERSION"
# TARGETPATH = "QtWebChannel"
# _LOADED = "qml_plugin"
