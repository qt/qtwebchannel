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

function QObject(name, data, webChannel)
{
    this.__id__ = name;
    this.__objectSignals__ = {};

    var methodsAndSignals = [];
    for (var i in data.methods)
        methodsAndSignals.push(data.methods[i]);
    for (var i in data.signals)
        methodsAndSignals.push(data.signals[i]);

    var object = this;

    methodsAndSignals.forEach(function(method) {
        object[method] = function() {
            var args = [];
            var callback;
            for (var i = 0; i < arguments.length; ++i) {
                if (typeof arguments[i] === "function")
                    callback = arguments[i];
                else
                    args.push(arguments[i]);
            }

            webChannel.exec({"type": "Qt.invokeMethod", "object": object.__id__, "method": method, "args": args}, function(response) {
                if ((response != undefined) && callback) {
                    (callback)(response);
                }
            });
        };
    });

    function connectToSignal(signal)
    {
        object[signal].connect = function(callback) {
            if (typeof(callback) !== "function") {
                console.error("Bad callback given to connect to signal " + signal);
                return;
            }
            object.__objectSignals__[signal] = object.__objectSignals__[signal] || [];
            webChannel.exec({"type": "Qt.connectToSignal", "object": object.__id__, "signal": signal});
            object.__objectSignals__[signal].push(callback);
        };
    }
    for (var i in data.signals) {
        var signal = data.signals[i];
        connectToSignal(data.signals[i]);
    }

    function bindGetterSetter(property)
    {
        object.__defineSetter__(property, function(value) {
            webChannel.exec({"type": "Qt.setProperty", "object": object.__id__, "property": property, "value": value });
        });
        object.__defineGetter__(property, function() {
            return (function(callback) {
                webChannel.exec({"type": "Qt.getProperty", "object": object.__id__, "property": property}, function(response) {
                    if (typeof(callback) !== "function") {
                        console.error("Bad callback given to get property " + property);
                        return;
                    }
                    callback(response);
                });
            });
        });
    }
    for (i in data.properties) {
        bindGetterSetter(data.properties[i]);
    }

    for (i in data.enums) {
        object[i] = data.enums[i];
    }
}

window.setupQObjectWebChannel = function(webChannel, doneCallback)
{
    webChannel.subscribe(
        "Qt.signal",
        function(payload) {
            var object = window[payload.object];
            if (object) {
                var connections = object.__objectSignals__[payload.signal];
                if (connections) {
                    connections.forEach(function(callback) {
                        callback.apply(callback, payload.args);
                    });
                }
            }
        }
    );
    webChannel.exec({type:"Qt.getObjects"}, function(payload) {
        for(var objectName in payload) {
            var data = payload[objectName];
            var object = new QObject(objectName, data, webChannel);
            window[objectName] = object;
        }
        if (doneCallback) {
            doneCallback();
        }
    });
};
