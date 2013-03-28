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

import QtQuick 2.0
import Qt.labs.WebChannel 1.0

MetaObjectPublisherImpl
{
    /**
     * This map contains the registered objects indexed by their name.
     */
    property variant registeredObjects

    /**
     * Handle the given WebChannel client request and potentially give a response.
     *
     * @return true if the request was handled, false otherwise.
     */
    function handleRequest(data, webChannel)
    {
        var message = typeof(data) === "string" ? JSON.parse(data) : data;
        if (!message.data) {
            return false;
        }
        var payload = message.data;
        if (!payload.type) {
            return false;
        }
        var object = payload.object ? registeredObjects[payload.object] : null;

        if (payload.type === "Qt.invokeMethod" && object) {
            var method = object[payload.method];
            webChannel.respond(message.id, method.apply(method, payload.args));
        } else if (payload.type === "Qt.connectToSignal" && object) {
            object[payload.signal].connect(function() {
                // NOTE: QML arguments is a map not an array it seems...
                // so do the conversion manually
                var args = [];
                for(var i = 0; i < arguments.length; ++i) {
                    args.push(arguments[i]);
                }
                webChannel.sendMessage("Qt.signal", {
                        object: payload.object,
                        signal: payload.signal,
                        args: args
                });
            });
        } else if (payload.type === "Qt.getProperty" && object) {
            webChannel.respond(message.id, object[payload.property]);
        } else if (payload.type === "Qt.setProperty" && object) {
            object[payload.property] = payload.value;
        } else if (payload.type === "Qt.getObjects") {
            webChannel.respond(message.id, registeredObjectInfos());
        } else if (payload.type === "Qt.Debug") {
            console.log("DEBUG: ", payload.message);
        } else {
            return false;
        }

        return true;
    }

    function registerObjects(objects)
    {
        // joining a JS map and a QML one is not as easy as one would assume...
        for(var name in registeredObjects) {
            if (!objects[name]) {
                objects[name] = registeredObjects[name];
            }
        }
        registeredObjects = objects;
    }

    function registeredObjectInfos()
    {
        var objectInfos = {};
        for(var name in registeredObjects) {
            var object = registeredObjects[name];
            if (object) {
                objectInfos[name] = classInfoForObject(object);
            }
        }
        return objectInfos;
    }
}
