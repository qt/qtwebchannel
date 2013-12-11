/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Copyright (C) 2013 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com, author Milian Wolff <milian.wolff@kdab.com>
** Contact: http://www.qt-project.org/legal
*
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

"use strict";

var QWebChannel = function(baseUrl, initCallback)
{
    var channel = this;
    // support multiple channels listening to the same socket
    // the responses to channel.exec must be distinguishable
    // see: http://stackoverflow.com/a/2117523/35250
    this.id = 'xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx'.replace(/[xy]/g, function(c) {
        var r = Math.random()*16|0, v = c == 'x' ? r : (r&0x3|0x8);
        return v.toString(16);
    });
    ///TODO: use ssl?
    var socketUrl = "ws://" + baseUrl;
    this.socket = new WebSocket(socketUrl, "QWebChannel");
    this.send = function(data)
    {
        if (typeof(data) !== "string") {
            data = JSON.stringify(data);
        }
        channel.socket.send(data);
    };

    this.socket.onopen = function()
    {
        initCallback(channel);
    };
    this.socket.onclose = function()
    {
        console.error("web channel closed");
    };
    this.socket.onerror = function(error)
    {
        console.error("web channel error: " + error);
    };
    this.socket.onmessage = function(message)
    {
        var jsonData = JSON.parse(message.data);
        if (jsonData.id === undefined) {
            console.error("invalid message received:", message.data);
            return;
        }
        if (jsonData.data === undefined) {
            jsonData.data = {};
        }
        if (jsonData.response) {
            if (jsonData.id[0] === channel.id) {
                channel.execCallbacks[jsonData.id[1]](jsonData.data);
                delete channel.execCallbacks[jsonData.id];
            }
        } else if (channel.subscriptions[jsonData.id]) {
            channel.subscriptions[jsonData.id].forEach(function(callback) {
                (callback)(jsonData.data); }
            );
        }
    };

    this.subscriptions = {};
    this.subscribe = function(id, callback)
    {
        if (channel.subscriptions[id]) {
            channel.subscriptions[id].push(callback);
        } else {
            channel.subscriptions[id] = [callback];
        }
    };

    this.execCallbacks = {};
    this.execId = 0;
    this.exec = function(data, callback)
    {
        if (!callback) {
            // if no callback is given, send directly
            channel.send({data: data});
            return;
        }
        if (channel.execId === Number.MAX_VALUE) {
            // wrap
            channel.execId = Number.MIN_VALUE;
        }
        var id = channel.execId++;
        channel.execCallbacks[id] = callback;
        channel.send({"id": [channel.id, id], "data": data});
    };

    this.objectMap = {};
};
