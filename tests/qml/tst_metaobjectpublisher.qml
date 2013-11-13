/****************************************************************************
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
** $QT_END_LICENSE$
**
****************************************************************************/

import QtQuick 2.0

import Qt.labs.WebChannel 1.0

WebChannelTest {
    name: "MetaObjectPublisher"
    id: test

    QtObject {
        id: myObj
        property int myProperty: 1
    }

    MetaObjectPublisher {
        id: publisher
        webChannel: test.webChannel

        Connections {
            target: webChannel
            onRawMessageReceived: {
                var message = JSON.parse(rawMessage);
                verify(message);
                publisher.handleRequest(message);
            }
        }
    }

    function initTestCase()
    {
        publisher.registerObjects({
            "myObj": myObj
        });
    }

    function awaitInit()
    {
        var msg = awaitMessage();
        verify(msg);
        verify(msg.data);
        verify(msg.data.type);
        compare(msg.data.type, "Qt.init");
    }

    function awaitIdle()
    {
        var msg = awaitMessage();
        verify(msg);
        verify(msg.data);
        compare(msg.data.type, "Qt.idle");
        verify(publisher.clientIsIdle)
    }

    function test_property()
    {
        loadUrl("property.html");
        awaitInit();
        var msg = awaitMessage();
        compare(msg.data.label, "init");
        compare(msg.data.value, 1);
        compare(myObj.myProperty, 1);

        awaitIdle();

        // change property, should be propagated to HTML client and a message be send there
        myObj.myProperty = 2;
        msg = awaitMessage();
        compare(msg.data.label, "changed");
        compare(msg.data.value, 2);
        compare(myObj.myProperty, 2);

        awaitIdle();

        // now trigger a write from the client side
        webChannel.sendMessage("setProperty", 3);
        msg = awaitMessage();
        compare(myObj.myProperty, 3);
    }
}
