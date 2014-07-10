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
var webchannel;

var assert = require('assert');
var WebSocket = require('faye-websocket');


describe('QWebChannel API', function() {
    //some tests really need some time for communication
    this.timeout(10000);

    before(function(done) {
        var socket = new WebSocket.Client('ws://127.0.0.1:1337');
        var transport = {};
        socket.on('open', function(event) {
            console.log("Socket connected");
            transport.postMessage = function(data) {
                socket.send(data);
            }

            new require('../../src/webchannel/qwebchannel.js').QWebChannel(transport, function(channel) {
                console.log("Webchannel ready");
                webchannel = channel;
                done();
            });
        });

        socket.on('message', function(event) {
            transport.onmessage(event);
        });

        socket.on('error', function (error) {

        });

        socket.on('close', function () {

        });

    });

    beforeEach(function(done) {
        // prepare client
        done();
    });

    afterEach(function() {
        // cleanup client
    });

    it('send "NodeJS Test" as a message to the server', function(done) {
        webchannel.objects.dialog.receiveText("NodeJS Test");
        done();
    });
});
