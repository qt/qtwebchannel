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

function S4() {
   return (((1+Math.random())*0x10000)|0).toString(16).substring(1);
}
function guid() {
   return (S4()+S4()+"-"+S4()+"-"+S4()+"-"+S4()+"-"+S4()+S4()+S4());
}


// Remove the calling script from the DOM.
var iframeElement = document.createElement("iframe");
iframeElement.style.display = "none";
iframeElement.src = baseUrl + "/h/" + guid();
document.body.appendChild(iframeElement);
var callbacks = {};

window.addEventListener("message", function(event) {
    if (baseUrl.indexOf(event.origin))
        return;
    var data = JSON.parse(event.data);
    var callbacksForID = callbacks[data.id] || [];

    callbacksForID.forEach(function(callback) { (callback)(JSON.parse(data.payload)); });
});

navigator.webChannel = {
    exec: function(message, callback) {
        var id = guid();
            iframeElement.contentWindow.postMessage(JSON.stringify({type: "exec", id: id, payload: message}), "*");
        callbacks[id] = [function(data) {
                             (callback)(data);
                             delete callbacks[id];
                         }];
    },

    subscribe: function(id, callback) {
        iframeElement.contentWindow.postMessage(JSON.stringify({type: "subscribe", id: id}), "*");
        callbacks[id] = callbacks[id] || [];
        callbacks[id].push(callback);
    },
};

