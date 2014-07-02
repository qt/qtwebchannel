/****************************************************************************
**
** Copyright (C) 2014 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com, author Milian Wolff <milian.wolff@kdab.com>
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtWebChannel module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

import QtQuick 2.0

import QtWebChannel 1.0

WebChannelTest {
    name: "MultiClient"
    id: test

    QtObject {
        id: foo
        property int bar: 0

        signal ping()

        function pong()
        {
            return ++bar;
        }

        WebChannel.id: "foo"
    }

    TestWebView {
        id: client1
    }
    TestWebView {
        id: client2
    }

    function initTestCase()
    {
        webChannel.registeredObjects = [foo];
    }

    function test_multiclient()
    {
        loadUrl("multiclient.html", client1);
        loadUrl("multiclient.html", client2);
        // init, connect & idle messages for two clients
        for (var i = 0; i < 3 * 2; ++i) {
            awaitMessage();
        }

        foo.ping();

        // invoke of pong method
        awaitMessage();
        awaitMessage();

        var msg = awaitMessage();
        compare(msg.data.pongAnswer, 1);
        msg = awaitMessage();
        compare(msg.data.pongAnswer, 2);

        awaitIdle();
        awaitIdle();
    }
}
