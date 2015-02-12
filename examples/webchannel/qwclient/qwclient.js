#!/usr/bin/env node
/****************************************************************************
**
** Copyright (C) 2014 basysKom GmbH, author Sumedha Widyadharma <sumedha.widyadharma@basyskom.com>
** Copyright (C) 2014 basysKom GmbH, author Lutz Schönemann <lutz.schoenemann@basyskom.com>
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtWebChannel module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL21$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file. Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** As a special exception, The Qt Company gives you certain additional
** rights. These rights are described in The Qt Company LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** $QT_END_LICENSE$
**
****************************************************************************/
'use strict';
var repl = require('repl');
var WebSocket = require('faye-websocket').Client;
var QWebChannel = new require('./qwebchannel.js').QWebChannel;

var serverAddress = 'ws://localhost:12345';
var channels = [];

var autoConnect = process.argv.pop();
if (autoConnect === __filename) {
    autoConnect = false;
}

var openChannel = function (address) {
    // this should be bound to the repl
    var self = this;
    address = address ? address : serverAddress;
    if (address.indexOf('://') === -1) {
        address = 'ws://' + address;
    }

    var ws = new WebSocket(address);

    ws.on('open', function (event) {
        var transport = {
        onmessage: function (data) {},
          send: function (data) {
              ws.send(data, {binary: false});
          }
        };
        ws.on('message', function (event) {
          transport.onmessage(event);
        }); // onmessage

        var webChannel = new QWebChannel(transport, function (channel) {
            channels.push(channel);
            var channelIdx = (channels.length - 1);
            console.log('channel opened', channelIdx);
            // Create a nice alias to access this channels objects
            self.context['c' + channelIdx] = channel.objects;

            ws.on('close', function () {
                for (var i = 0; i < channels.length; ++i) {
                    if (channels[i] === channel) {
                        console.log('channel closed', i);
                        channels[i] = null;
                        return;
                    }
                }
            }); // onclose
        }); // new QWebChannel
    }); // onopen

    ws.on('error', function (error) {
        console.log('websocket error', error.message);
    });
}; // openChannel

var setupRepl = function() {
    var r = repl.start({
        prompt: "webchannel> ",
        input: process.stdin,
        output: process.stdout
    });

    r.context.serverAddress = serverAddress;
    r.context.openChannel = openChannel.bind(r);
    r.context.channels = channels;

    r.context.lsObjects = function() {
        channels.forEach(function(channel){
            console.log('Channel ' + channel);
            Object.keys(channel.objects);
        });
    }
    return r;
}

var welcome = function() {
    console.log('Welcome to the qwebchannel/websocket REPL.');
    console.log('Use openChannel(url) to connect to a service.');
    console.log('For the standalone example, just openChannel() should suffice.');
    console.log('Opened channels have their objects aliased to c<channel number>, i.e. c0');
    console.log('So for the standalone example try: c0.dialog.receiveText(\'hello world\')');
}

welcome();
var repl = setupRepl();

if (autoConnect) {
    repl.context.openChannel(autoConnect);
}
