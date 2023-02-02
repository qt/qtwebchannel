# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_add_qml_module(WebChannelQuick
    URI "QtWebChannel"
    VERSION "${PROJECT_VERSION}"
    PAST_MAJOR_VERSIONS 1
    GENERATE_CPP_EXPORTS
    SOURCES
        qqmlwebchannel.h qqmlwebchannel.cpp
        qqmlwebchannelattached_p.h qqmlwebchannelattached.cpp
        qwebchannelquickglobal.h
    LIBRARIES
        Qt::WebChannelPrivate
)
