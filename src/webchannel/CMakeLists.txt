# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generated from webchannel.pro.

#####################################################################
## WebChannel Module:
#####################################################################

qt_internal_add_module(WebChannel
    SOURCES
        qmetaobjectpublisher.cpp qmetaobjectpublisher_p.h
        qwebchannel.cpp qwebchannel.h qwebchannel_p.h
        qwebchannelabstracttransport.cpp qwebchannelabstracttransport.h
        signalhandler_p.h
        qwebchannelglobal.h
    LIBRARIES
        Qt::CorePrivate
    PUBLIC_LIBRARIES
        Qt::Core
    PRIVATE_MODULE_INTERFACE
        Qt::CorePrivate
    GENERATE_CPP_EXPORTS
)

# Resources:
set_source_files_properties("../../examples/webchannel/shared/qwebchannel.js"
    PROPERTIES QT_RESOURCE_ALIAS "qwebchannel.js"
)
set(resources_resource_files
    "../../examples/webchannel/shared/qwebchannel.js"
)

set_source_files_properties(${resources_resource_files} PROPERTIES QT_SKIP_QUICKCOMPILER 1)

qt_internal_add_resource(WebChannel "resources"
    PREFIX
        "/qtwebchannel/"
    FILES
        ${resources_resource_files}
)


#### Keys ignored in scope 1:.:.:webchannel.pro:<TRUE>:
# OTHER_FILES = "qwebchannel.js"

## Scopes:
#####################################################################

qt_internal_extend_target(WebChannel CONDITION TARGET Qt::Qml
    SOURCES
        qqmlwebchannel.cpp qqmlwebchannel.h
        qqmlwebchannelattached.cpp qqmlwebchannelattached_p.h
    PUBLIC_LIBRARIES
        Qt::Qml
)

qt_internal_extend_target(WebChannel CONDITION NOT TARGET Qt::Qml
    DEFINES
        QT_NO_JSVALUE
)
qt_internal_add_docs(WebChannel
    doc/qtwebchannel.qdocconf
)

