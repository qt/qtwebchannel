# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generated from webchannel.pro.

#####################################################################
## webchannel Plugin:
#####################################################################

qt_internal_add_qml_module(webchannel
    URI "QtWebChannel"
    VERSION "${PROJECT_VERSION}"
    PAST_MAJOR_VERSIONS 1
    CLASS_NAME QWebChannelPlugin
    PLUGIN_TARGET webchannel
    NO_PLUGIN_OPTIONAL
    NO_GENERATE_PLUGIN_SOURCE
    NO_GENERATE_QMLTYPES
    INSTALL_SOURCE_QMLTYPES "plugins.qmltypes"
    SOURCES
        plugin.cpp
    INCLUDE_DIRECTORIES
        ../../webchannel
    LIBRARIES
        Qt::Core
        Qt::Quick
        Qt::WebChannelPrivate
)

#### Keys ignored in scope 1:.:.:webchannel.pro:<TRUE>:
# IMPORT_VERSION = "1.$$QT_MINOR_VERSION"
# TARGETPATH = "QtWebChannel"
