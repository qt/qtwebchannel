/****************************************************************************
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
** Copyright (C) 2014 basysKom GmbH, author Bernd Lamecker <bernd.lamecker@basyskom.com>
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
'use strict';

var WebSocket = require('faye-websocket');

var socket = new WebSocket.Client('ws://127.0.0.1:1337');
var transport = {};
socket.on('open', function(event) {
    transport.postMessage = function(data) {
        socket.send(data);
    }

    var messageBuffer = '';

    new require('../../src/webchannel/qwebchannel.js').QWebChannel(transport, function(channel) {
        console.log("Client connected");

        process.stdin.setRawMode(true); //required to get input by each single keystroke

        channel.objects.dialog.sendText.connect(function(message) {
            process.stdout.clearLine();
            process.stdout.cursorTo(0);
            console.log('Received message: ' + message);
            getInput();
        });

        function getInput() {
            process.stdout.write('>' + messageBuffer);
        }

        process.stdin.on('data', function (message) {
            if (message[0] === 3) {
                process.exit();
            } else if (message[0] === 13) {
                process.stdout.clearLine();
                process.stdout.cursorTo(0);
                process.stdout.write('Sent message: ' + messageBuffer);
                channel.objects.dialog.receiveText(messageBuffer);
                messageBuffer = '';
                getInput();
            } else {
                process.stdout.write(message.toString());
                messageBuffer += message.toString();
            }
        });

        getInput();
    });
});

socket.on('message', function(event) {
    transport.onmessage(event);
});

socket.on('error', function (error) {
    console.log('connection error: ' + error);
    process.exit(1);
});

socket.on('close', function () {
    console.log('connection closed');
    process.exit(1);
});
