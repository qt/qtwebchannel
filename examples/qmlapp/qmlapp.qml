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
import QtWebKit 3.0
import QtWebKit.experimental 1.0

Rectangle {
    width: 500
    height: 600
    WebChannel {
        id: webChannel

        onRawMessageReceived: {
            console.log(rawMessage);
            var msg = JSON.parse(rawMessage);
            editor.text += msg.data.a + "\n";
            sendMessage("b", "This is a response from QML");
        }

        onInitialized: {
            console.log(baseUrl);
        }
    }

    TextEdit {
        text: "enter data here\n"
        id: editor
        anchors.top: parent.top
        width: parent.width
        height: 400
    }

    Text {
        id: txt
        anchors.top: editor.bottom
        width: parent.width
        height: 100
        text: "Click to send message to HTML client"
        MouseArea {
            anchors.fill: parent
            onClicked: {
                webChannel.sendMessage("foobar", editor.text);
            }
        }
    }

    WebView {
        id: webView
        width: parent.width
        anchors.top: txt.bottom
        height: 100
        url: "index.html?webChannelBaseUrl=" + webChannel.baseUrl;
        experimental.preferences.developerExtrasEnabled: true
    }

}
