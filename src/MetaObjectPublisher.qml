/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** Copyright (C) 2013 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com, author Milian Wolff <milian.wolff@kdab.com>
** Contact: http://www.qt-project.org/legal
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

import QtQuick 2.0
import Qt.labs.WebChannel 1.0

MetaObjectPublisherImpl
{
    id: publisher

    // The web channel this publisher works on.
    property var webChannel

    /**
     * This map contains the registered objects indexed by their name.
     */
    property variant registeredObjects
    property var subscriberCountMap

    // Map of object names to maps of signal names to an array of all their properties.
    // The last value is an array as a signal can be the notify signal of multiple properties.
    property var signalToPropertyMap

    // Objects that changed their properties and are waiting for idle client.
    // map of object name to map of signal name to arguments
    property var pendingPropertyUpdates

    // true when the client is idle, false otherwise
    property bool clientIsIdle: false

    // true when no property updates should be sent, false otherwise
    property bool blockUpdates: false

    // true when at least one client needs to be initialized,
    // i.e. when a Qt.init came in which was not handled yet.
    property bool pendingInit: false

    // true when at least one client was initialized and thus
    // the property updates have been initialized and the
    // object info map set.
    property bool propertyUpdatesInitialized: false

    /**
     *    Wrap a result value if it's a Qt QObject
     *
     *    @return object info for wrapped Qt Object,
     *     or the same value if no wrapping needed
     *
     */
    function wrapResult(result)
    {
        if (typeof(result) === "object"
            && result["objectName"] !== undefined)
        {
            var ret = wrapObject(result);
            initializePropertyUpdates(ret.id, ret.data, result, webChannel);
            return ret;
        }
        return result;
    }

    function convertQMLArgsToJSArgs(qmlArgs)
    {
        // NOTE: QML arguments is a map not an array it seems...
        // so do the conversion manually
        var args = [];
        for (var i = 0; i < qmlArgs.length; ++i) {
            args.push(qmlArgs[i]);
        }
        return args;
    }

    /**
     * Handle the given WebChannel client request and potentially give a response.
     *
     * @return true if the request was handled, false otherwise.
     */
    function handleRequest(data)
    {
        var message = typeof(data) === "string" ? JSON.parse(data) : data;
        if (!message.data) {
            return false;
        }
        var payload = message.data;
        if (!payload.type) {
            return false;
        }

        if (payload.object) {
            var isWrapped = false;
            var object = registeredObjects[payload.object];
            if (!object) {
                object = unwrapObject(payload.object);
                if (object)
                    isWrapped = true;
                else
                    return false
            }

            if (payload.type === "Qt.invokeMethod") {
                var method = object[payload.method];
                if (method !== undefined) {
                    webChannel.respond(message.id,
                        wrapResult(method.apply(method, payload.args)));
                    return true;
                }
                if (isWrapped && payload.method === "deleteLater") {
                    // invoke `deleteLater` on wrapped QObject indirectly
                    deleteWrappedObject(object);
                    return true;
                }
                return false;
            }
            if (payload.type === "Qt.connectToSignal") {
                if (object.hasOwnProperty(payload.signal)) {
                    subscriberCountMap =  subscriberCountMap || {};
                    subscriberCountMap[payload.object] = subscriberCountMap[payload.object] || {};

                    // if no one is connected, connect.
                    if (!subscriberCountMap[payload.object].hasOwnProperty(payload.signal)) {
                         object[payload.signal].connect(function() {
                            var args = convertQMLArgsToJSArgs(arguments);
                            webChannel.sendMessage("Qt.signal", {
                                object: payload.object,
                                signal: payload.signal,
                                args: args
                            });
                        });
                        subscriberCountMap[payload.object][payload.signal] = true;
                    }
                    return true;
                }
                // connecting to `destroyed` signal of wrapped QObject
                if (isWrapped && payload.signal === "destroyed") {
                    // is a no-op on this side
                    return true;
                }
                return false;
            }
            if (payload.type === "Qt.setProperty") {
                object[payload.property] = payload.value;
                return true;
            }
        }
        if (payload.type === "Qt.idle") {
            clientIsIdle = true;
            return true;
        }
        if (payload.type === "Qt.init") {
            if (!blockUpdates) {
                initializeClients();
            } else {
                pendingInit = true;
            }
            return true;
        }
        if (payload.type === "Qt.Debug") {
            console.log("DEBUG: ", payload.message);
            return true;
        }
        return false;
    }

    function registerObjects(objects)
    {
        if (propertyUpdatesInitialized) {
            console.error("Registered new object after initialization. This does not work!");
        }
        // joining a JS map and a QML one is not as easy as one would assume...
        // NOTE: the extra indirection via "merged" is required, using registeredObjects directly
        //       does not work! this looks like a QML/v8 bug to me, but I could not find a
        //       standalone testcase which reproduces this behavior :(
        var merged = registeredObjects;
        for (var name in objects) {
            if (!merged.hasOwnProperty(name)) {
                merged[name] = objects[name];
            }
        }
        registeredObjects = merged;
    }

    function initializeClients()
    {
        var objectInfos = classInfoForObjects(registeredObjects);
        webChannel.sendMessage("Qt.init", objectInfos);
        if (!propertyUpdatesInitialized) {
            for (var objectName in objectInfos) {
                var objectInfo = objectInfos[objectName];
                var object = registeredObjects[objectName];
                initializePropertyUpdates(objectName, objectInfo, object);
            }
            propertyUpdatesInitialized = true;
        }
        pendingInit = false;
    }

    // This function goes through all properties of all objects and connects against
    // their notify signal.
    // When receiving a notify signal, it will send a Qt.propertyUpdate message to the
    // server.
    function initializePropertyUpdates(objectName, objectInfo, object)
    {
        for (var propertyIndex in objectInfo.properties) {
            var propertyInfo = objectInfo.properties[propertyIndex];
            var propertyName = propertyInfo[0];
            var signalName = propertyInfo[1];

            if (!signalName) // Property without NOTIFY signal
                continue;

            if (signalName === 1) {
                /// signal name is optimized away, reconstruct the actual name
                signalName = propertyName + "Changed";
            }

            signalToPropertyMap[objectName] = signalToPropertyMap[objectName] || {};
            signalToPropertyMap[objectName][signalName] = signalToPropertyMap[objectName][signalName] || [];
            var connectedProperties = signalToPropertyMap[objectName][signalName];
            var numConnectedProperties = connectedProperties === undefined ? 0 : connectedProperties.length;

            // Only connect for a property update once
            if (numConnectedProperties === 0) {
                (function(signalName) {
                    object[signalName].connect(function() {
                        pendingPropertyUpdates[objectName] = pendingPropertyUpdates[objectName] || {};
                        pendingPropertyUpdates[objectName][signalName] = arguments;
                    });
                })(signalName);
            }

            if (connectedProperties.indexOf(propertyName) === -1) {
                /// TODO: this ensures that a given property is only once in
                ///       the list of connected properties.
                ///       This happens when multiple clients are connected to
                ///       a single webchannel. A better place for the initialization
                ///       should be found.
                connectedProperties.push(propertyName);
            }
        }
    }

    function sendPendingPropertyUpdates()
    {
        if (blockUpdates || !clientIsIdle) {
            return;
        }

        var data = [];
        for (var objectName in pendingPropertyUpdates) {
            var object = registeredObjects[objectName];
            if (!object) {
                object = unwrapObject(objectName);
                if (!object) {
                    console.error("Got property update for unknown object " + objectName);
                    continue;
                }
            }
            var signals = pendingPropertyUpdates[objectName];
            var propertyMap = {};
            for (var signalName in signals) {
                var propertyList = signalToPropertyMap[objectName][signalName];
                for (var propertyIndex in propertyList) {
                    var propertyName = propertyList[propertyIndex];
                    var propertyValue = object[propertyName];
                    propertyMap[propertyName] = propertyValue;
                }
                signals[signalName] = convertQMLArgsToJSArgs(signals[signalName]);
            }
            data.push({
                object: objectName,
                signals: signals,
                propertyMap: propertyMap
            });
        }
        pendingPropertyUpdates = {};
        if (data.length > 0) {
            webChannel.sendMessage("Qt.propertyUpdate", data);
            clientIsIdle = false;
        }
    }

    Component.onCompleted: {
        // Initializing this in the property declaration is not possible and yields to "undefined"
        signalToPropertyMap = {}
        pendingPropertyUpdates = {}
        registeredObjects = {}
    }

    onBlockUpdatesChanged: {
        if (blockUpdates) {
            return;
        }

        if (pendingInit) {
            initializeClients();
        } else {
            sendPendingPropertyUpdates();
        }
    }

    onWrappedObjectDestroyed: { // (const QString& id)
        // act as if object had sent `destroyed` signal
        webChannel.sendMessage("Qt.signal", {
            object: id,
            signal: "destroyed",
            args: []
        });
        delete subscriberCountMap[id];
        delete pendingPropertyUpdates[id];
        delete signalToPropertyMap[id]
    }

    /**
     * Aggregate property updates since we get multiple Qt.idle message when we have multiple
     * clients. They all share the same QWebProcess though so we must take special care to
     * prevent message flooding.
     */
    Timer {
        id: propertyUpdateTimer
        /// TODO: what is the proper value here?
        interval: 50;
        running: !publisher.blockUpdates && publisher.clientIsIdle;
        repeat: true;
        onTriggered: publisher.sendPendingPropertyUpdates()
    }
}
