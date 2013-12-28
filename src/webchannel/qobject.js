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

function QObject(name, data, webChannel)
{
    this.__id__ = name;
    webChannel.objectMap[name] = this;

    // List of callbacks that get invoked upon signal emission
    this.__objectSignals__ = {};

    // Cache of all properties, updated when a notify signal is emitted
    this.__propertyCache__ = {};

    var object = this;

    // ----------------------------------------------------------------------

    function unwrapQObject( response )
    {
        if (!response["__QObject*__"]
            || response["id"] === undefined
            || response["data"] === undefined) {
            return response;
        }
        var objectId = response.id;
        if (webChannel.objectMap[objectId])
            return webChannel.objectMap[objectId];

        var qObject = new QObject( objectId, response.data, webChannel );
        qObject.destroyed.connect(function() {
            if (webChannel.objectMap[objectId] === qObject) {
                delete webChannel.objectMap[objectId];
                // reset the now deleted QObject to an empty {} object
                // just assigning {} though would not have the desired effect, but the
                // below also ensures all external references will see the empty map
                for (var prop in qObject) {
                    delete qObject[prop];
                }
            }
        });
        return qObject;
    }

    function addSignal(signalData, isPropertyNotifySignal)
    {
        var signalName = signalData[0];
        var signalIndex = signalData[1];
        object[signalName] = {
            connect: function(callback) {
                if (typeof(callback) !== "function") {
                    console.error("Bad callback given to connect to signal " + signalName);
                    return;
                }

                object.__objectSignals__[signalIndex] = object.__objectSignals__[signalIndex] || [];
                object.__objectSignals__[signalIndex].push(callback);

                if (!isPropertyNotifySignal) {
                    // only required for "pure" signals, handled separately for properties in propertyUpdate
                    webChannel.exec({
                        type: "Qt.connectToSignal",
                        object: object.__id__,
                        signal: signalIndex
                    });
                }
            },
            disconnect: function(callback) {
                if (typeof(callback) !== "function") {
                    console.error("Bad callback given to disconnect from signal " + signalName);
                    return;
                }
                object.__objectSignals__[signalIndex] = object.__objectSignals__[signalIndex] || [];
                var idx = object.__objectSignals__[signalIndex].indexOf(callback);
                if (idx === -1) {
                    console.error("Cannot find connection of signal " + signalName + " to " + callback.name);
                    return;
                }
                object.__objectSignals__[signalIndex].splice(idx, 1);
                if (!isPropertyNotifySignal && object.__objectSignals__[signalIndex].length === 0) {
                    // only required for "pure" signals, handled separately for properties in propertyUpdate
                    webChannel.exec({
                        type: "Qt.disconnectFromSignal",
                        object: object.__id__,
                        signal: signalIndex
                    });
                }
            }
        };
    }

    /**
     * Invokes all callbacks for the given signalname. Also works for property notify callbacks.
     */
    function invokeSignalCallbacks(signalName, signalArgs)
    {
        var connections = object.__objectSignals__[signalName];
        if (connections) {
            connections.forEach(function(callback) {
                callback.apply(callback, signalArgs);
            });
        }
    }

    this.propertyUpdate = function(signals, propertyMap)
    {
        // update property cache
        for (var propertyIndex in propertyMap) {
            var propertyValue = propertyMap[propertyIndex];
            object.__propertyCache__[propertyIndex] = propertyValue;
        }

        for (var signalName in signals) {
            // Invoke all callbacks, as signalEmitted() does not. This ensures the
            // property cache is updated before the callbacks are invoked.
            invokeSignalCallbacks(signalName, signals[signalName]);
        }
    }

    this.signalEmitted = function(signalName, signalArgs)
    {
        invokeSignalCallbacks(signalName, signalArgs);
    }

    function addMethod(methodData)
    {
        var methodName = methodData[0];
        var methodIdx = methodData[1];
        object[methodName] = function() {
            var args = [];
            var callback;
            for (var i = 0; i < arguments.length; ++i) {
                if (typeof arguments[i] === "function")
                    callback = arguments[i];
                else
                    args.push(arguments[i]);
            }

            webChannel.exec({"type": "Qt.invokeMethod", "object": object.__id__, "method": methodIdx, "args": args}, function(response) {
                if ( (response !== undefined) && callback ) {
                    (callback)(unwrapQObject(response));
                }
            });
        };
    }

    function bindGetterSetter(propertyInfo)
    {
        var propertyIndex = propertyInfo[0];
        var propertyName = propertyInfo[1];
        var notifySignalData = propertyInfo[2];
        // initialize property cache with current value
        object.__propertyCache__[propertyIndex] = propertyInfo[3];

        if (notifySignalData) {
            if (notifySignalData[0] === 1) {
                // signal name is optimized away, reconstruct the actual name
                notifySignalData[0] = propertyName + "Changed";
            }
            addSignal(notifySignalData, true);
        }

        object.__defineSetter__(propertyName, function(value) {
            if (value === undefined) {
                console.warn("Property setter for " + propertyName + " called with undefined value!");
                return;
            }
            object.__propertyCache__[propertyIndex] = value;
            webChannel.exec({"type": "Qt.setProperty", "object": object.__id__, "property": propertyIndex, "value": value });

        });
        object.__defineGetter__(propertyName, function () {
            return (function (callback) {
                var propertyValue = object.__propertyCache__[propertyIndex];
                if (propertyValue === undefined) {
                    // This shouldn't happen
                    console.warn("Undefined value in property cache for property \"" + propertyName + "\" in object " + object.__id__);
                }

                // TODO: A callback is not required here anymore, but is kept for backwards compatibility
                if (callback !== undefined) {
                    if (typeof(callback) !== "function") {
                        console.error("Bad callback given to get property " + property);
                        return;
                    }
                    callback(propertyValue);
                } else {
                    return propertyValue;
                }
            });
        });
    }

    // ----------------------------------------------------------------------

    data.methods.forEach(addMethod);

    data.properties.forEach(bindGetterSetter);

    data.signals.forEach(function(signal) { addSignal(signal, false); });

    for (var name in data.enums) {
        object[name] = data.enums[name];
    }
}

window.setupQObjectWebChannel = function(webChannel, doneCallback)
{
    // prevent multiple initialization which might happen with multiple webchannel clients.
    var initialized = false;

    webChannel.subscribe(
        "Qt.signal",
        function(payload) {
            var object = window[payload.object] || webChannel.objectMap[payload.object];
            if (object) {
                object.signalEmitted(payload.signal, payload.args);
            } else {
                console.warn("Unhandled signal: " + payload.object + "::" + payload.signal);
            }
        }
    );

    webChannel.subscribe(
        "Qt.propertyUpdate",
        function(payload) {
            for (var i in payload) {
                var data = payload[i];
                var object = window[data.object] || webChannel.objectMap[data.object];
                if (object) {
                    object.propertyUpdate(data.signals, data.properties);
                } else {
                    console.warn("Unhandled property update: " + data.object + "::" + data.signal);
                }
            }
            setTimeout(function() { webChannel.exec({type: "Qt.idle"}); }, 0);
        }
    );

    webChannel.subscribe(
        "Qt.init",
        function(payload) {
            if (initialized) {
                return;
            }
            initialized = true;
            for (var objectName in payload) {
                var data = payload[objectName];
                var object = new QObject(objectName, data, webChannel);
                window[objectName] = object;
            }
            if (doneCallback) {
                doneCallback();
            }
            setTimeout(function() { webChannel.exec({type: "Qt.idle"}); }, 0);
        }
    );

    webChannel.exec({type:"Qt.init"});

    webChannel.debug = function(message)
    {
        webChannel.send({"data" : {"type" : "Qt.Debug", "message" : message}});
    };
};
