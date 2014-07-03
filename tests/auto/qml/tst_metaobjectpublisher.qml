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
import QtTest 1.0

import QtWebChannel 1.0
import QtWebChannel.Tests 1.0
import "qrc:///qwebchannel/qwebchannel.js" as Client

TestCase {
    name: "MetaObjectPublisher"

    Client {
        id: client
    }

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

    TestWebChannel {
        id: webChannel
        transports: [client.serverTransport]
        registeredObjects: [myObj, myOtherObj, myFactory]
    }

    function init()
    {
        myObj.myProperty = 1
        client.cleanup();
    }

    function test_property()
    {
        var channel = client.createChannel(function(channel) {
            channel.exec({label: "init", value: channel.objects.myObj.myProperty});
            channel.objects.myObj.myPropertyChanged.connect(function() {
                channel.exec({label: "changed", value: channel.objects.myObj.myProperty});
            });
            channel.subscribe("setProperty", function(newValue) {
                channel.objects.myObj.myProperty = newValue;
            });
        });

        client.awaitInit();
        var msg = client.awaitMessageSkipIdle();
        compare(msg.data.label, "init");
        compare(msg.data.value, 1);
        compare(myObj.myProperty, 1);

        // change property, should be propagated to HTML client and a message be send there
        myObj.myProperty = 2;
        msg = client.awaitMessageSkipIdle();
        compare(msg.data.label, "changed");
        compare(msg.data.value, 2);
        compare(myObj.myProperty, 2);

        // now trigger a write from the client side
        webChannel.sendMessage("setProperty", 3);
        msg = client.awaitMessageSkipIdle();
        compare(myObj.myProperty, 3);

        // the above write is also propagated to the HTML client
        msg = client.awaitMessageSkipIdle();
        compare(msg.data.label, "changed");
        compare(msg.data.value, 3);

        client.awaitIdle();
    }

    function test_method()
    {
        var channel = client.createChannel(function (channel) {
            channel.subscribe("invokeMethod", function (arg) {
                channel.objects.myObj.myMethod(arg);
            });
        });

        client.awaitInit();
        client.awaitIdle();

        webChannel.sendMessage("invokeMethod", "test");

        var msg = client.awaitMessage();
        compare(msg.data.type, Client.QWebChannelMessageTypes.invokeMethod);
        compare(msg.data.object, "myObj");
        compare(msg.data.args, ["test"]);

        compare(lastMethodArg, "test")
    }

    function test_signal()
    {
        var channel = client.createChannel(function(channel) {
            channel.objects.myObj.mySignal.connect(function(arg) {
                channel.exec({label: "signalReceived", value: arg});
            });
        });
        client.awaitInit();

        var msg = client.awaitMessage();
        compare(msg.data.type, Client.QWebChannelMessageTypes.connectToSignal);
        compare(msg.data.object, "myObj");

        client.awaitIdle();

        myObj.mySignal("test");

        msg = client.awaitMessageSkipIdle();
        compare(msg.data.label, "signalReceived");
        compare(msg.data.value, "test");
    }

    function test_grouping()
    {
        var channel = client.createChannel(function(channel) {
            channel.subscribe(Client.QWebChannelMessageTypes.propertyUpdate, function() {
                channel.exec({label: "gotPropertyUpdate", values: [channel.objects.myObj.myProperty, channel.objects.myOtherObj.foo, channel.objects.myOtherObj.bar]});
            });
        });
        client.awaitInit();
        client.awaitIdle();

        // change properties a lot, we expect this to be grouped into a single update notification
        for (var i = 0; i < 10; ++i) {
            myObj.myProperty = i;
            myOtherObj.foo = i;
            myOtherObj.bar = i;
        }

        var msg = client.awaitMessage();
        verify(msg);
        compare(msg.data.label, "gotPropertyUpdate");
        compare(msg.data.values, [myObj.myProperty, myOtherObj.foo, myOtherObj.bar]);

        client.awaitIdle();
    }

    function test_wrapper()
    {
        var channel = client.createChannel(function(channel) {
            channel.objects.myFactory.create("testObj", function(obj) {
                channel.objects["testObj"] = obj;
                obj.mySignal.connect(function(arg1, arg2) {
                    channel.exec({label: "signalReceived", args: [arg1, arg2]});
                });
                obj.myProperty = 42;
                obj.myMethod("foobar");
            });
            channel.subscribe("triggerDelete", function() {
                channel.objects.testObj.deleteLater();
            });
            channel.subscribe("report", function() {
                channel.exec({label:"report", obj: channel.objects.testObj})
            });
        });
        client.awaitInit();

        var msg = client.awaitMessageSkipIdle();
        compare(msg.data.type, Client.QWebChannelMessageTypes.invokeMethod);
        compare(msg.data.object, "myFactory");
        verify(myFactory.lastObj);
        compare(myFactory.lastObj.objectName, "testObj");

        msg = client.awaitMessageSkipIdle();
        compare(msg.data.type, Client.QWebChannelMessageTypes.connectToSignal);
        verify(msg.data.object);
        var objId = msg.data.object;

        msg = client.awaitMessageSkipIdle();
        compare(msg.data.type, Client.QWebChannelMessageTypes.connectToSignal);
        compare(msg.data.object, objId);

        msg = client.awaitMessageSkipIdle();
        compare(msg.data.type, Client.QWebChannelMessageTypes.setProperty);
        compare(msg.data.object, objId);
        compare(myFactory.lastObj.myProperty, 42);

        msg = client.awaitMessageSkipIdle();
        compare(msg.data.type, Client.QWebChannelMessageTypes.invokeMethod);
        compare(msg.data.object, objId);
        compare(msg.data.args, ["foobar"]);

        msg = client.awaitMessageSkipIdle();
        compare(msg.data.label, "signalReceived");
        compare(msg.data.args, ["foobar", 42]);

        // pass QObject* on the fly and trigger deleteLater from client side
        webChannel.sendMessage("triggerDelete");

        msg = client.awaitMessageSkipIdle();
        compare(msg.data.type, Client.QWebChannelMessageTypes.invokeMethod);
        compare(msg.data.object, objId);

        client.awaitIdle();

        webChannel.sendMessage("report");

        msg = client.awaitMessageSkipIdle();
        compare(msg.data.label, "report");
        compare(msg.data.obj, {});
    }

    function test_disconnect()
    {
        var channel = client.createChannel(function(channel) {
            channel.objects.myObj.mySignal.connect(function(arg) {
                channel.exec({label: "mySignalReceived", args: [arg]});
                channel.objects.myObj.mySignal.disconnect(this);
            });
            channel.subscribe("report", function() {
                channel.exec({label: "report"});
            });
        });
        client.awaitInit();

        var msg = client.awaitMessage();
        compare(msg.data.type, Client.QWebChannelMessageTypes.connectToSignal);
        compare(msg.data.object, "myObj");

        client.awaitIdle();

        myObj.mySignal(42);

        msg = client.awaitMessage();
        compare(msg.data.label, "mySignalReceived");
        compare(msg.data.args, [42]);

        msg = client.awaitMessage();
        compare(msg.data.type, Client.QWebChannelMessageTypes.disconnectFromSignal);
        compare(msg.data.object, "myObj");

        myObj.mySignal(0);

        // apparently one cannot expect failure in QML, so trigger another message
        // and verify no mySignalReceived was triggered by the above emission
        webChannel.sendMessage("report");

        msg = client.awaitMessage();
        compare(msg.data.label, "report");
    }
}
