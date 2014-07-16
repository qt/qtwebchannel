### Introduction

The `QtWebChannel` module offers Qt applications a seamless way to publish `QObjects` for interaction
with HTML/JavaScript clients. These clients can either be inside local Qt `WebView`s or any other,
potentially remote, client which supports JavaScript, as long as a communication channel such
as WebSockets is available.

`QtWebChannel` uses introspection on the `QObject`s and sends this serialized data to the clients.
There, with the help of a small JavaScript library, an object is created which simulates the API of
the `QObject`. Any invokable methods, including slots, can be called as well as properties read and
written. Additionally you can connect to signals and register JavaScript callbacks as handlers.

### Dependencies

This module depends on `QtBase` only. Optionally, an additional plugin for QtQuick is build which as
makes it easy to use the `QWebChannel` from QML. Note that this module alone is not functional. It
is being used in `QtWebKit` to provide a seamless integration of QML/C++ QObjects into JavaScript
clients. You can integrate it in your projects as well, by providing an implementation of the
`QWebChannelAbstractTransport` class, see the `standalone` example for how to do this.

### Building

    qmake-qt5
    make
    make install

### Usage from C++

To use the Qt/C++ library, add the following to your `QMake` project:

    QT += webchannel

Then, in your C++ code, construct a a webchannel, then publish your `QObject`s:

    QWebChannel channel;
    channel.registerObject(QStringLiteral("foo"), myFooObj);
    ....

Additionally, you need to provide a communication channel to the HTML client. One way is to
use the QtWebSockets module. On the HTML/JavaScript client side, you need to embed
`src/webchannel/qwebchannel.js` and setup the connection to a client-side transport. An example
which shows all this in action can be found in `examples/standalone`.

### Usage from QtQuick

For QML applications, use the following import:

    import QtWebChannel 1.0

Then setup the WebChannel, register objects to it and connect to transport objects:

    WebChannel {
        registeredObjects: [foo, bar, ...]

        transports: [yourTransport]
    }

To see this in action, take a look at the test code in `tests/auto/qml`.
