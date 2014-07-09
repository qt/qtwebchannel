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
import QtWebChannel.Tests 1.0
import "qrc:///qwebchannel/qwebchannel.js" as Client

Item {
    TestTransport {
        id: serverTransport
    }
    readonly property var serverTransport: serverTransport

    property var clientMessages: []

    property bool debug: false

    QtObject {
        id: clientTransport

        property var send;
        property var onmessage;

        function postMessage(message)
        {
            if (debug) {
                console.log("client posts message: ", message);
            }
            clientMessages.push(message);
            serverTransport.receiveMessage(message);
        }

        Component.onCompleted: {
            serverTransport.sendMessageRequested.connect(function(message) {
                if (debug) {
                    console.log("client received message: ", message);
                }
                if (onmessage) {
                    onmessage({data:message});
                }
            });
        }
    }
    readonly property var clientTransport: clientTransport

    Timer {
        id: timer
        running: false
        repeat: false

        property var callback

        onTriggered: {
            callback();
        }
    }

    function setTimeout(callback, delay)
    {
        if (timer.running) {
            console.error("nested calls to setTimeout are not supported!", JSON.stringify(callback), JSON.stringify(timer.callback));
            return;
        }
        timer.callback = callback;
        // note: an interval of 0 is directly triggered, so add a little padding
        timer.interval = delay + 1;
        timer.running = true;
    }

    function createChannel(callback, raw)
    {
        return new Client.QWebChannel(clientTransport, callback, raw);
    }

    function cleanup()
    {
        clientMessages = [];
        timer.running = false;
    }

    function awaitRawMessage()
    {
        for (var i = 0; i < 10 && !clientMessages.length; ++i) {
            wait(10);
        }
        return clientMessages.shift();
    }

    function awaitMessage()
    {
        var msg = awaitRawMessage()
        if (debug) {
          console.log("handling message: ", msg);
        }
        if (!msg) {
            return false;
        }
        return JSON.parse(msg);
    }

    function awaitInit()
    {
        var msg = awaitMessage();
        verify(msg);
        verify(msg.type);
        compare(msg.type, Client.QWebChannelMessageTypes.init);
    }

    function awaitIdle()
    {
        var msg = awaitMessage();
        verify(msg);
        compare(msg.type, Client.QWebChannelMessageTypes.idle);
        verify(webChannel.clientIsIdle())
    }

    function awaitMessageSkipIdle()
    {
        var msg;
        do {
            msg = awaitMessage();
            verify(msg);
        } while (msg.type === Client.QWebChannelMessageTypes.idle);
        return msg;
    }

}
