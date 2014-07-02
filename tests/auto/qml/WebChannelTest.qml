/****************************************************************************
**
** Copyright (C) 2014 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com, author Milian Wolff <milian.wolff@kdab.com>
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtWebChannel module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

import QtQuick 2.0
import QtTest 1.0

import QtWebChannel 1.0

TestCase {
    property var lastLoadStatus
    property bool useWebViewTransport: false

    // only run after the webchannel has finished initialization
    when: webSocketTransport.baseUrl != ""

    WebViewTransport {
        id: webViewTransport
        webViewExperimental: defaultView.experimental
    }
    WebSocketTransport {
        id: webSocketTransport
    }

    TestWebView {
        id: defaultView
    }

    WebChannel {
        id: webChannel
    }
    property var webChannel: webChannel

    SignalSpy {
        id: rawMessageSpy
        target: useWebViewTransport ? webViewTransport : webSocketTransport;
        signalName: "onMessageReceived"
    }
    property var rawMessageSpy: rawMessageSpy
    property var rawMessageIdx: 0;

    function urlForFile(file)
    {
        verify(useWebViewTransport || webSocketTransport.baseUrl != "", "webSocketTransport.baseUrl is empty");
        return "data/" + file + (!useWebViewTransport ? "?webChannelBaseUrl=" + webSocketTransport.baseUrl : "");
    }

    // load file in the given view or use the global one by default
    function loadUrl(file, view)
    {
        if (useWebViewTransport) {
            webChannel.disconnectFrom(webSocketTransport);
            webChannel.connectTo(webViewTransport);
        } else {
            webChannel.disconnectFrom(webViewTransport);
            webChannel.connectTo(webSocketTransport);
        }
        if (!view) {
            view = defaultView;
        }
        view.url = urlForFile(file);
        view.waitForLoaded();
    }

    function cleanup()
    {
        defaultView.clear();
        rawMessageSpy.clear();
        rawMessageIdx = 0;
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

    function awaitInit()
    {
        var msg = awaitMessage();
        verify(msg);
        verify(msg.data);
        verify(msg.data.type);
        compare(msg.data.type, qWebChannelMessageTypes.init);
    }

    function awaitIdle()
    {
        var msg = awaitMessage();
        verify(msg);
        verify(msg.data);
        compare(msg.data.type, qWebChannelMessageTypes.idle);
        verify(webChannel.test_clientIsIdle())
    }

    property var qWebChannelMessageTypes: ({
        signal: 1,
        propertyUpdate: 2,
        init: 3,
        idle: 4,
        debug: 5,
        invokeMethod: 6,
        connectToSignal: 7,
        disconnectFromSignal: 8,
        setProperty: 9,
    });
}
