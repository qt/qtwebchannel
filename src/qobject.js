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
    webChannel.objectMap[name] = this;

    // List of callbacks that get invoked upon signal emission
    this.__objectSignals__ = {};

    // Cache of all properties, updated when a notify signal is emitted
    this.__propertyCache__ = {};

    var object = this;

    // ----------------------------------------------------------------------

    function addSignal(signal, isPropertyNotifySignal)
    {
        object[signal] = {
            connect: function(callback) {
                if (typeof(callback) !== "function") {
                    console.error("Bad callback given to connect to signal " + signal);
                    return;
                }

                object.__objectSignals__[signal] = object.__objectSignals__[signal] || [];
                object.__objectSignals__[signal].push(callback);

                if (!isPropertyNotifySignal) {
                    // only required for "pure" signals, handled separately for properties in propertyUpdate
                    webChannel.exec({
                        type: "Qt.connectToSignal",
                        object: object.__id__,
                        signal: signal
                    });
                }
            }
            // TODO: disconnect eventually
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
        for (var propertyName in propertyMap) {
            var propertyValue = propertyMap[propertyName];
            object.__propertyCache__[propertyName] = propertyValue;
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

    function addMethod(method)
    {
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
                if ( (response !== undefined) && callback ) {
                    (callback)(response);
                }
            });
        };
    }

    function bindGetterSetter(propertyInfo)
    {
        var propertyName = propertyInfo[0];
        var notifySignal = propertyInfo[1];
        // initialize property cache with current value
        object.__propertyCache__[propertyName] = propertyInfo[2]

        if (notifySignal) {
            if (notifySignal === 1) {
                /// signal name is optimized away, reconstruct the actual name
                notifySignal = propertyName + "Changed";
            }
            addSignal(notifySignal, true);
        }

        object.__defineSetter__(propertyName, function(value) {
            if (value === undefined) {
                console.warn("Property setter for " + propertyName + " called with undefined value!");
                return;
            }
            object.__propertyCache__[propertyName] = value;
            webChannel.exec({"type": "Qt.setProperty", "object": object.__id__, "property": propertyName, "value": value });

        });
        object.__defineGetter__(propertyName, function () {
            return (function (callback) {
                var propertyValue = object.__propertyCache__[propertyName];
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
            var object = webChannel.objectMap[payload.object];
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
                var object = webChannel.objectMap[data.object];
                if (object) {
                    object.propertyUpdate(data.signals, data.propertyMap);
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
};
