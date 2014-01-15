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
    name: "MetaObjectPublisher"
    id: test

    property var lastMethodArg

    QtObject {
        id: myObj
        property int myProperty: 1

        signal mySignal(var arg)

        function myMethod(arg)
        {
            lastMethodArg = arg;
        }

        WebChannel.id: "myObj"
    }
    QtObject {
        id: myOtherObj
        property var foo: 1
        property var bar: 1
        WebChannel.id: "myOtherObj"
    }
    QtObject {
        id: myFactory
        property var lastObj
        function create(id)
        {
            lastObj = component.createObject(myFactory, {objectName: id});
            return lastObj;
        }
        WebChannel.id: "myFactory"
    }

    Component {
        id: component
        QtObject {
            property var myProperty : 0
            function myMethod(arg) {
                mySignal(arg, myProperty);
            }
            signal mySignal(var arg1, var arg2)
        }
    }

    function initTestCase()
    {
        webChannel.registeredObjects = [myObj, myOtherObj, myFactory];
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
        verify(webChannel.test_clientIsIdle())
    }

    function awaitMessageSkipIdle()
    {
        var msg;
        do {
            msg = awaitMessage();
            verify(msg);
            verify(msg.data);
        } while (msg.data.type === "Qt.idle");
        return msg;
    }

    function test_property()
    {
        myObj.myProperty = 1
        loadUrl("property.html");
        awaitInit();
        var msg = awaitMessageSkipIdle();
        compare(msg.data.label, "init");
        compare(msg.data.value, 1);
        compare(myObj.myProperty, 1);

        // change property, should be propagated to HTML client and a message be send there
        myObj.myProperty = 2;
        msg = awaitMessageSkipIdle();
        compare(msg.data.label, "changed");
        compare(msg.data.value, 2);
        compare(myObj.myProperty, 2);

        // now trigger a write from the client side
        webChannel.sendMessage("setProperty", 3);
        msg = awaitMessageSkipIdle();
        compare(myObj.myProperty, 3);

        // the above write is also propagated to the HTML client
        msg = awaitMessageSkipIdle();
        compare(msg.data.label, "changed");
        compare(msg.data.value, 3);

        awaitIdle();
    }

    function test_method()
    {
        loadUrl("method.html");
        awaitInit();
        awaitIdle();

        webChannel.sendMessage("invokeMethod", "test");

        var msg = awaitMessage();
        compare(msg.data.type, "Qt.invokeMethod");
        compare(msg.data.object, "myObj");
        compare(msg.data.args, ["test"]);

        compare(lastMethodArg, "test")
    }

    function test_signal()
    {
        loadUrl("signal.html");
        awaitInit();

        var msg = awaitMessage();
        compare(msg.data.type, "Qt.connectToSignal");
        compare(msg.data.object, "myObj");

        awaitIdle();

        myObj.mySignal("test");

        msg = awaitMessage();
        compare(msg.data.label, "signalReceived");
        compare(msg.data.value, "test");
    }

    function test_grouping()
    {
        loadUrl("grouping.html");
        awaitInit();
        awaitIdle();

        // change properties a lot, we expect this to be grouped into a single update notification
        for (var i = 0; i < 10; ++i) {
            myObj.myProperty = i;
            myOtherObj.foo = i;
            myOtherObj.bar = i;
        }

        var msg = awaitMessage();
        verify(msg);
        compare(msg.data.label, "gotPropertyUpdate");
        compare(msg.data.values, [myObj.myProperty, myOtherObj.foo, myOtherObj.bar]);

        awaitIdle();
    }

    function test_wrapper()
    {
        loadUrl("wrapper.html");
        awaitInit();

        var msg = awaitMessageSkipIdle();
        compare(msg.data.type, "Qt.invokeMethod");
        compare(msg.data.object, "myFactory");
        verify(myFactory.lastObj);
        compare(myFactory.lastObj.objectName, "testObj");

        msg = awaitMessageSkipIdle();
        compare(msg.data.type, "Qt.connectToSignal");
        verify(msg.data.object);
        var objId = msg.data.object;

        msg = awaitMessageSkipIdle();
        compare(msg.data.type, "Qt.connectToSignal");
        compare(msg.data.object, objId);

        msg = awaitMessageSkipIdle();
        compare(msg.data.type, "Qt.setProperty");
        compare(msg.data.object, objId);
        compare(myFactory.lastObj.myProperty, 42);

        msg = awaitMessageSkipIdle();
        compare(msg.data.type, "Qt.invokeMethod");
        compare(msg.data.object, objId);
        compare(msg.data.args, ["foobar"]);

        msg = awaitMessageSkipIdle();
        compare(msg.data.label, "signalReceived");
        compare(msg.data.args, ["foobar", 42]);

        // pass QObject* on the fly and trigger deleteLater from client side
        webChannel.sendMessage("triggerDelete");

        msg = awaitMessageSkipIdle();
        compare(msg.data.type, "Qt.invokeMethod");
        compare(msg.data.object, objId);

        webChannel.sendMessage("report");

        msg = awaitMessageSkipIdle();
        compare(msg.data.label, "report");
        compare(msg.data.obj, {});
    }

    function test_disconnect()
    {
        loadUrl("disconnect.html");
        awaitInit();

        var msg = awaitMessage();
        compare(msg.data.type, "Qt.connectToSignal");
        compare(msg.data.object, "myObj");

        awaitIdle();

        myObj.mySignal(42);

        msg = awaitMessage();
        compare(msg.data.label, "mySignalReceived");
        compare(msg.data.args, [42]);

        msg = awaitMessage();
        compare(msg.data.type, "Qt.disconnectFromSignal");
        compare(msg.data.object, "myObj");

        myObj.mySignal(0);

        // apparently one cannot expect failure in QML, so trigger another message
        // and verify no mySignalReceived was triggered by the above emission
        webChannel.sendMessage("report");

        msg = awaitMessage();
        compare(msg.data.label, "report");
    }
}
