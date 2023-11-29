# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_add_module(WebChannel
    SOURCES
        qmetaobjectpublisher.cpp qmetaobjectpublisher_p.h
        qwebchannel.cpp qwebchannel.h qwebchannel_p.h
        qwebchannelabstracttransport.cpp qwebchannelabstracttransport.h
        signalhandler_p.h
        qwebchannelglobal.h
    DEFINES
        QT_NO_CONTEXTLESS_CONNECT
    LIBRARIES
        Qt::CorePrivate
)

set(resource_file "../../examples/webchannel/shared/qwebchannel.js")
set_source_files_properties(${resource_file} PROPERTIES
    QT_RESOURCE_ALIAS "qwebchannel.js"
)

qt_internal_add_resource(WebChannel "resources"
    PREFIX
        "/qtwebchannel/"
    FILES
        ${resource_file}
)

if(TARGET Qt::Qml)
    qt_internal_extend_target(WebChannel PUBLIC_LIBRARIES Qt::Qml)
else()
    qt_internal_extend_target(WebChannel DEFINES QT_NO_JSVALUE)
endif()

qt_internal_add_docs(WebChannel
    doc/qtwebchannel.qdocconf
)

