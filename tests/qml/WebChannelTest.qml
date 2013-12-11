/****************************************************************************
**
** Copyright (C) 2013 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com, author Milian Wolff <milian.wolff@kdab.com>
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtWebChannel module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** No Commercial Usage
** This file contains pre-release code and may not be distributed.
** You may use this file in accordance with the terms and conditions
** contained in the Technology Preview License Agreement accompanying
** this package.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights.  These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** If you have questions regarding the use of this file, please contact
** Nokia at qt-info@nokia.com.
**
** $QT_END_LICENSE$
**
****************************************************************************/

import QtQuick 2.0
import QtTest 1.0

import QtWebChannel 1.0
import QtWebKit 3.0
import QtWebKit.experimental 1.0

TestCase {
    // only run after the webchannel has finished initialization
    when: webChannel.baseUrl != ""

    property var lastLoadStatus

    WebView {
        id: view

        experimental.preferences.developerExtrasEnabled: true

        onLoadingChanged: {
            // NOTE: we cannot use spy.signalArguments nor save the loadRequest anywhere, as it gets
            // deleted after the slots connected to the signal have finished... i.e. it's a weak pointer,
            // not a shared pointer. As such, we have to copy out the interesting data we need later on here...
            lastLoadStatus = loadRequest.status
        }

        SignalSpy {
            id: loadingSpy
            target: view
            signalName: "onLoadingChanged"
        }
    }

    WebChannel {
        id: webChannel
    }
    property var webChannel: webChannel

    SignalSpy {
        id: rawMessageSpy
        target: webChannel
        signalName: "onRawMessageReceived"
    }
    property var rawMessageSpy: rawMessageSpy
    property var rawMessageIdx: 0;

    SignalSpy {
        id: pongSpy
        target: webChannel
        signalName: "onPongReceived"
    }
    property var pongSpy: pongSpy

    function loadUrl(url)
    {
        verify(webChannel.baseUrl != "", "webChannel.baseUrl is empty");
        view.url = "data/" + url + "?webChannelBaseUrl=" + webChannel.baseUrl;
        // now wait for page to finish loading
        do {
            loadingSpy.wait(500);
        } while (view.loading);
        compare(lastLoadStatus, WebView.LoadSucceededStatus);
    }

    function cleanup()
    {
        view.url = "";
        loadingSpy.clear();
        rawMessageSpy.clear();
        rawMessageIdx = 0;
        pongSpy.clear();
    }

    function awaitRawMessage()
    {
        rawMessageSpy.wait(500);
        if (rawMessageSpy.signalArguments.length <= rawMessageIdx) {
            // still no message received, fail
            return null;
        }
        return rawMessageSpy.signalArguments[rawMessageIdx++][0];
    }

    function awaitMessage()
    {
        var msg = awaitRawMessage()
        if (!msg) {
            return msg;
        }
        return JSON.parse(msg);
    }
}
