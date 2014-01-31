### Introduction

The `QtWebChannel` module offers Qt applications a seamless way to publish `QObjects` for interaction
from HTML/JavaScript clients. These clients can either be inside local Qt `WebView`s or any other,
potentially remote, client which supports JavaScript and WebSockets.

It uses introspection on the `QObject`s and sends this serialized data to the clients. There, with
the help of a small JavaScript library, an object is created which simulates the API of the `QObject`.
Any invokable methods, including slots, can be called as well as properties read and written.
Additionally you can connect to signals and register JavaScript callbacks as handlers.

### Dependencies

This module depends on `QtBase` and `QtWebSockets`. Optionally, an additional module for QtQuick is
build which makes it easy to use the `QWebChannel` from QML. Furthermore, you can decide to use the
native IPC mechanism of `QtWebKit` for efficient message passing to QtQuick `WebView`'s.

### Building

    qmake-qt5
    make
    make install

### Usage from C++

To use the Qt/C++ library, add the following to your `QMake` project:

    QT += webchannel

Then, in your C++ code, construct a websocket transport and webchannel, then publish your `QObject`s:

    QWebSocketTransport transport;
    QWebChannel channel;
    channel.connectTo(&transport);

    channel.registerObject(QStringLiteral("foo"), myFooObj);
    ....

On the HTML/JavaScript client side, you need to embed `src/webchannel/qwebchannel.js` and setup
the connection to the WebSocket transport. The base URL for that can be found from C++ via
`transport.baseUrl()` after the transport was initialized, and must be passed to HTML clients:

    <script type="text/javascript" src="path/to/qwebchannel.js"></script>
    <script type="text/javascript">
    // note: baseUrl must be known
    new QWebChannel(baseUrl, function(channel) {
        foo.doStuff(); // calls doStuff slot or invokable method on myFooObj on the C++ side
    });
    </script>

An example which shows all this can be found in `examples/standalone`.

### Usage from QtQuick

For QML applications, use the following import:

    import QtWebChannel 1.0

Then setup the WebChannel and register objects to it:

    WebChannel {
        registeredObjects: [foo, bar, ...]

        connections: WebViewTransport {
            webViewExperimental: yourWebView.experimental
            onMessageReceived: {
                textEdit.text += "Received message: " + message + "\n";
            }
        }
    }

The above uses a `WebViewTransport` for efficient message passing between the server QML application
and HTML clients in a `WebView`, which must be setup as follows:

    WebView {
        id: yourWebView
        experimental.preferences.navigatorQtObjectEnabled: true
    }

The HTML client finally uses a similar setup as above, but can make use of the embedded resource
for `qwebchannel.js` and the `navigator.qt` object:

    <script type="text/javascript" src="qrc:///qwebchannel/qwebchannel.js"></script>
    <script type="text/javascript">
    new QWebChannel(navigator.qt, function(channel) {
        // do stuff with published objects
    });
    </script>

To see this in action, take a look at `examples/qml` and run the `example.qml` in `qmlscene`.
