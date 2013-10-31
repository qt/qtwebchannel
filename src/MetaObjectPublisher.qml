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

MetaObjectPublisherPrivate
{
    /**
     * This map contains the registered objects indexed by their name.
     */
    property variant registeredObjects

    /**
     * Handle the given WebChannel client request and write to the given response.
     *
     * @return true if the request was handled, false otherwise.
     */
    function handleRequest(payload, webChannel, response)
    {
        if (!payload.type) {
            return false;
        }
        var object = payload.object ? registeredObjects[payload.object] : null;

        var ret = undefined;
        if (payload.type === "Qt.invokeMethod" && object) {
            ret = (object[payload.method])(payload.args);
        } else if (payload.type === "Qt.connectToSignal" && object) {
            object[payload.signal].connect(
                function(a,b,c,d,e,f,g,h,i,j) {
                    webChannel.broadcast("Qt.signal", JSON.stringify({object: payload.object, signal: payload.signal, args: [a,b,c,d,e,f,g,h,i,j]}));
            });
        } else if (payload.type === "Qt.getProperty" && object) {
            ret = object[payload.property];
        } else if (payload.type === "Qt.setProperty" && object) {
            object[payload.property] = payload.value;
        } else if (payload.type === "Qt.getObjects") {
            var ret = {};
            for (var name in registeredObjects) {
                object = registeredObjects[name];
                if (object) {
                    ret[name] = classInfoForObject(object);
                }
            }
        } else if (payload.type === "Qt.Debug") {
            console.log("DEBUG: ", payload.message);
        } else {
            return false;
        }

        if (ret != undefined) {
            response.send(JSON.stringify(ret));
        }
        return true;
    }

    function registerObjects(objects)
    {
        // joining a JS map and a QML one is not as easy as one would assume...
        for (var name in registeredObjects) {
            if (!objects[name]) {
                objects[name] = registeredObjects[name];
            }
        }
        registeredObjects = objects;
    }
}
