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

// Remove the calling script from the DOM.
var scriptElement = document.querySelector('script[src="' + baseUrl + '"]');
scriptElement.parentNode.removeChild(scriptElement);

function sendRequest(url, onSuccess, onFailure)
{
    var req = new XMLHttpRequest();
    req.open("GET", url, true);
    req.onreadystatechange = function() {
        if (req.readyState != 4)
            return;
        if (req.status != 200 && req.status != 304) {
            onFailure();
            req = undefined;
            return;
        }
        onSuccess(JSON.parse(req.responseText));
        req = undefined;
    };
    req.send(null);
}

function poll(url, callback)
{
    setTimeout(function() {
    sendRequest(url,
        function(object) {
            poll(url, callback);
            callback(object);
        }, function() { poll(url, callback); });
    }, 0);
}

navigator.webChannel = {
    execute: function(message, onSuccess, onFailure) {
        sendRequest(baseUrl + "/e/"+ JSON.stringify(message), onSuccess, onFailure);
    },

    subscribe: function(id, callback) {
        poll(baseUrl + "/s/" + id, callback);
    },
};

