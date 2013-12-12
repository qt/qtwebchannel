/****************************************************************************
**
** Copyright (C) 2013 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com, author Milian Wolff <milian.wolff@kdab.com>
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtWebChannel module of the Qt Toolkit.
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

WebChannelTest {
    name: "WebChannel"

    function test_receiveRawMessage()
    {
        loadUrl("receiveRaw.html");
        compare(awaitRawMessage(), "foobar");
    }

    function test_sendMessage()
    {
        loadUrl("send.html");
        webChannel.sendMessage("myMessage", "foobar");
        compare(awaitRawMessage(), "myMessagePong:foobar");
    }

    function test_respondMessage()
    {
        loadUrl("respond.html");
        var msg = awaitMessage();
        verify(msg.id);
        compare(msg.data, "foobar");
        webChannel.respond(msg.id, "barfoo");
        compare(awaitRawMessage(), "received:barfoo");
    }

    function test_ping()
    {
        loadUrl("respond.html");
        webChannel.ping();
        pongSpy.wait(500);
        compare(pongSpy.count, 1);
    }
}
