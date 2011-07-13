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

function debug(x) {
  document.body.innerHTML = document.body.innerHTML  + "<p>" + x + "</p>";
}


(function(){

var queryVariables = {};
var requests = {};
var subscribers = {};
var baseUrl = "";
var initialized = false;
function S4() {
   return (((1+Math.random())*0x10000)|0).toString(16).substring(1);
}
function guid() {
   return (S4()+S4()+"."+S4()+"."+S4()+"."+S4()+"."+S4()+S4()+S4());
}


function sendRequest(url, onSuccess, onFailure)
{
    var req = new XMLHttpRequest();
    req.open("GET", url, true);
    req.onreadystatechange = function() {
        if (req.readyState != 4)
            return;
        if (req.status != 200 && req.status != 304) {
            onFailure();
            return;
        }
        onSuccess(JSON.parse(req.responseText));
    };
    req.send(null);
}

function poll(url, callback)
{
    setTimeout(function() {
    sendRequest(url + "/" + guid(),
        function(object) {
            poll(url, callback);
            callback(object);
        }, function() { poll(url, callback); });
    }, 0);
}

function init() {
    if (initialized)
        return;
    initialized = true;
    var search = location.search.substr(1).split("&");
    for (var i = 0; i < search.length; ++i) {
        var s = search[i].split("=");
        queryVariables[s[0]] = s[1];
    }
    baseUrl = queryVariables.webchannel_baseUrl;
}

navigator.webChannel = {
    exec: function(message, onSuccess, onFailure) {
        init();
        sendRequest(baseUrl + "/exec/"+ JSON.stringify(message), onSuccess, onFailure);
    },

    subscribe: function(id, callback) {
        init();
        poll(baseUrl + "/subscribe/" + id, callback);
    },
};
})();
