// Copyright (C) 2016 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com, author Milian Wolff <milian.wolff@kdab.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "dialog.h"
#include "core.h"
#include "httpserver.h"
#include "websocketclientwrapper.h"
#include "websockettransport.h"

#include <QApplication>
#include <QByteArray>
#include <QDesktopServices>
#include <QDialog>
#include <QFile>
#include <QHostAddress>
#include <QUrl>
#include <QWebChannel>
#include <QWebSocketServer>

namespace {

QByteArray readResource(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
        qFatal("Missing embedded resource: %s", qPrintable(path));
    return file.readAll();
}

} // namespace

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    // setup the QWebSocketServer
    QWebSocketServer server(QStringLiteral("QWebChannel Standalone Example Server"), QWebSocketServer::NonSecureMode);
    if (!server.listen(QHostAddress::LocalHost, 12345)) {
        qFatal("Failed to open web socket server.");
        return 1;
    }

    // serve the index.html page and its qwebchannel.js over loopback address
    HttpServer httpServer({
        { QByteArrayLiteral("/"),
          { QByteArrayLiteral("text/html"), readResource(QStringLiteral(":/index.html")) } },
        { QByteArrayLiteral("/qwebchannel.js"),
          { QByteArrayLiteral("text/javascript"), readResource(QStringLiteral(":/qwebchannel.js")) } },
    });
    if (!httpServer.listen())
        return 1;

    // wrap WebSocket clients in QWebChannelAbstractTransport objects
    WebSocketClientWrapper clientWrapper(&server);

    // setup the channel
    QWebChannel channel;
    QObject::connect(&clientWrapper, &WebSocketClientWrapper::clientConnected,
                     &channel, &QWebChannel::connectTo);

    // setup the UI
    Dialog dialog;

    // setup the core and publish it to the QWebChannel
    Core core(&dialog);
    channel.registerObject(QStringLiteral("core"), &core);

    // open a browser window with the client HTML page
    const QUrl url(QStringLiteral("http://127.0.0.1:%1/").arg(httpServer.port()));
    QDesktopServices::openUrl(url);

    dialog.displayMessage(Dialog::tr("Initialization complete, opening browser at %1.").arg(url.toDisplayString()));
    dialog.show();

    return app.exec();
}
