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
import "qrc:///qtwebchannel/qwebchannel.js" as JSClient

TestCase {
    name: "WebChannel"

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
                lastMethodArg = arg;
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
        compare(myObj.myProperty, 1);

        var initialValue;
        var changedValue;

        var channel = client.createChannel(function(channel) {
            initialValue = channel.objects.myObj.myProperty;
            channel.objects.myObj.myPropertyChanged.connect(function() {
                changedValue = channel.objects.myObj.myProperty;
            });
            // now trigger a write from the client side
            channel.objects.myObj.myProperty = 3;
        });

        client.awaitInit();
        var msg = client.awaitMessage();

        compare(initialValue, 1);
        compare(myObj.myProperty, 3);

        client.awaitIdle();

        // change property, should be propagated to HTML client and a message be send there
        myObj.myProperty = 2;
        compare(myObj.myProperty, 2);
        client.awaitIdle();
        compare(changedValue, 2);
    }

    function test_method()
    {
        var channel = client.createChannel(function (channel) {
            channel.objects.myObj.myMethod("test");
        });

        client.awaitInit();

        var msg = client.awaitMessage();
        compare(msg.type, JSClient.QWebChannelMessageTypes.invokeMethod);
        compare(msg.object, "myObj");
        compare(msg.args, ["test"]);

        compare(lastMethodArg, "test")

        client.awaitIdle();
    }

    function test_signal()
    {
        var signalReceivedArg;
        var channel = client.createChannel(function(channel) {
            channel.objects.myObj.mySignal.connect(function(arg) {
                signalReceivedArg = arg;
            });
        });
        client.awaitInit();

        var msg = client.awaitMessage();
        compare(msg.type, JSClient.QWebChannelMessageTypes.connectToSignal);
        compare(msg.object, "myObj");

        client.awaitIdle();

        myObj.mySignal("test");

        compare(signalReceivedArg, "test");
    }

    function test_grouping()
    {
        var receivedPropertyUpdates = 0;
        var properties = 0;
        var channel = client.createChannel(function(channel) {
            var originalHandler = channel.handlePropertyUpdate;
            channel.handlePropertyUpdate = function(message) {
                originalHandler(message);
                receivedPropertyUpdates++;
                properties = [channel.objects.myObj.myProperty, channel.objects.myOtherObj.foo, channel.objects.myOtherObj.bar];
            };
        });
        client.awaitInit();
        client.awaitIdle();

        // change properties a lot, we expect this to be grouped into a single update notification
        for (var i = 0; i < 10; ++i) {
            myObj.myProperty = i;
            myOtherObj.foo = i;
            myOtherObj.bar = i;
        }

        client.awaitIdle();
        compare(receivedPropertyUpdates, 1);
        compare(properties, [myObj.myProperty, myOtherObj.foo, myOtherObj.bar]);
        verify(!client.awaitMessage());
    }

    function test_wrapper()
    {
        var signalArgs;
        var testObjBeforeDeletion;
        var testObjAfterDeletion;
        var testObjId;
        var channel = client.createChannel(function(channel) {
            channel.objects.myFactory.create("testObj", function(obj) {
                testObjId = obj.__id__;
                compare(channel.objects[testObjId], obj);
                obj.mySignal.connect(function() {
                    signalArgs = arguments;
                    testObjBeforeDeletion = obj;
                    obj.deleteLater();
                    testObjAfterDeletion = obj;
                });
                obj.myProperty = 42;
                obj.myMethod("foobar");
            });
        });
        client.awaitInit();

        var msg = client.awaitMessage();
        compare(msg.type, JSClient.QWebChannelMessageTypes.invokeMethod);
        compare(msg.object, "myFactory");
        verify(myFactory.lastObj);
        compare(myFactory.lastObj.objectName, "testObj");
        compare(channel.objects[testObjId].objectName, "testObj");

        // deleteLater signal connection
        msg = client.awaitMessage();
        compare(msg.type, JSClient.QWebChannelMessageTypes.connectToSignal);
        compare(msg.object, testObjId);

        // mySignal connection
        msg = client.awaitMessage();
        compare(msg.type, JSClient.QWebChannelMessageTypes.connectToSignal);
        compare(msg.object, testObjId);

        msg = client.awaitMessage();
        compare(msg.type, JSClient.QWebChannelMessageTypes.setProperty);
        compare(msg.object, testObjId);
        compare(myFactory.lastObj.myProperty, 42);

        msg = client.awaitMessage();
        compare(msg.type, JSClient.QWebChannelMessageTypes.invokeMethod);
        compare(msg.object, testObjId);
        compare(msg.args, ["foobar"]);
        compare(lastMethodArg, "foobar");

        myFactory.lastObj.mySignal("foobar", 42);

        // deleteLater call
        msg = client.awaitMessage();
        compare(msg.type, JSClient.QWebChannelMessageTypes.invokeMethod);
        compare(msg.object, testObjId);

        compare(signalArgs, {"0": "foobar", "1": 42});

        client.awaitIdle();

        compare(JSON.stringify(testObjBeforeDeletion), JSON.stringify({}));
        compare(JSON.stringify(testObjAfterDeletion), JSON.stringify({}));
        compare(typeof channel.objects[testObjId], "undefined");
    }

    // test if returned QObjects get inserted into list of
    // objects even if no callback function is set
    function test_wrapper_wrapEveryQObject()
    {
        var channel = client.createChannel(function(channel) {
            channel.objects.myFactory.create("testObj");
        });
        client.awaitInit();

        // ignore first message (call to myFactory.create())
        client.awaitMessage();

        // second message connects to destroyed signal and contains the new objects ID
        var msg = client.awaitMessage();
        verify(msg.object);

        var testObjId = msg.object;
        compare(msg.type, JSClient.QWebChannelMessageTypes.connectToSignal);
        compare(typeof channel.objects[testObjId], "object");

        channel.objects[testObjId].deleteLater();
        msg = client.awaitMessage();

        // after receiving the destroyed signal the client deletes
        // local objects and sends back a idle message
        client.awaitIdle();

        compare(myFactory.lastObj, null);
        compare(typeof channel.objects[testObjId], "undefined");
    }

    function test_disconnect()
    {
        var signalArg;
        var channel = client.createChannel(function(channel) {
            channel.objects.myObj.mySignal.connect(function(arg) {
                signalArg = arg;
                channel.objects.myObj.mySignal.disconnect(this);
            });
        });
        client.awaitInit();

        var msg = client.awaitMessage();
        compare(msg.type, JSClient.QWebChannelMessageTypes.connectToSignal);
        compare(msg.object, "myObj");

        client.awaitIdle();

        myObj.mySignal(42);
        compare(signalArg, 42);

        msg = client.awaitMessage();
        compare(msg.type, JSClient.QWebChannelMessageTypes.disconnectFromSignal);
        compare(msg.object, "myObj");

        myObj.mySignal(0);
        compare(signalArg, 42);
    }
}
