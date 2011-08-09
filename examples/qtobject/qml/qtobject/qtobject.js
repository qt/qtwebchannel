/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QWebChannel module on Qt labs.
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
**
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

window.onload = function() {
    createWebChannel(function(webChannel) {
        var allSignals = {};
        webChannel.subscribe(
            "Qt.signal",
            function(payload) {
                var signalData = JSON.parse(payload);
                var object = allSignals[signalData.object];
                var conns = (object ? object[signalData.signal] : []) || [];
                var a = payload.args;
                conns.forEach(function(callback) {
                    callback.call(a);
                });
            }
        );
        webChannel.subscribe(
            "Qt.addToWindowObject",
            function(payload) {
                 var addObjectData = JSON.parse(payload);
                 var objectSignals = {};
                 var methodsAndSignals = [];
                 var object = {};
                 var objectName = addObjectData.name;
                 var data = addObjectData.data;
                 for (var i = 0; i < data.methods.length; ++i)
                     methodsAndSignals.push(data.methods[i]);
                 for (i = 0; i < data.signals.length; ++i)
                     methodsAndSignals.push(data.signals[i]);

                 methodsAndSignals.forEach(function(method) {
                     object[method] = function() {
                         var args = [];
                         var callback;
                         for (var i = 0; i < arguments.length; ++i) {
                             if (typeof arguments[i] == "function")
                                 callback = arguments[i];
                             else
                                 args.push(arguments[i]);
                         }

                         webChannel.exec(JSON.stringify({type: "Qt.invokeMethod", object: objectName, method: method, args: args}), function(response) {
                             if (response.length)
                                 (callback)(JSON.parse(response));
                         });
                     };
                 });

                 data.signals.forEach(function(signal) {
                     object[signal].connect = function(callback) {
                         objectSignals[signal] = objectSignals[signal] || [];
                         webChannel.exec(JSON.stringify({type: "Qt.connectToSignal", object: objectName, signal: signal}));
                         objectSignals[signal].push(callback);
                     };
                 });
                 allSignals[addObjectData.name] = objectSignals;

                 data.properties.forEach(function(prop) {
                     object.__defineSetter__(prop, function(value) {
                         webChannel.exec(JSON.stringify({type: "Qt.setProperty", object: objectName, property: prop, value: value }));
                     });
                     object.__defineGetter__(prop, function() {
                         return (function(callback) {
                             webChannel.exec(JSON.stringify({type: "Qt.getProperty", object: objectName, property: prop}), function(response) {
                                 callback(JSON.parse(response));
                             });
                         });
                     });
                 });

                 window[addObjectData.name] = object;
            }
        );
        webChannel.exec(JSON.stringify({type:"Qt.getObjects"}));
    });
};
