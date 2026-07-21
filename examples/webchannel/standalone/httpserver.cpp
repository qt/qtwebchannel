// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "httpserver.h"

#include <QHostAddress>
#include <QList>
#include <QTcpServer>
#include <QTcpSocket>

#include <optional>

namespace {

constexpr int maxRequestSize = 8 * 1024;

QByteArray makeHttpResponse(int statusCode, const char *reasonPhrase,
                            const QByteArray &mimeType, const QByteArray &body)
{
    return QByteArray("HTTP/1.1 ") + QByteArray::number(statusCode) + ' ' + reasonPhrase + "\r\n"
           + "Content-Type: " + mimeType + "\r\n"
           + "Content-Length: " + QByteArray::number(body.size()) + "\r\n"
           + "Connection: close\r\n"
           + "\r\n" + body;
}

// Returns the request target (the path, without any query string) once the HTTP
// request line has fully arrived, or nullopt while it is still incomplete.
std::optional<QByteArray> tryParseRequestTarget(const QByteArray &request)
{
    const int endOfLine = request.indexOf("\r\n");
    if (endOfLine < 0)
        return std::nullopt;

    const QList<QByteArray> parts = request.left(endOfLine).split(' ');
    if (parts.size() < 2)
        return QByteArray();

    QByteArray target = parts.at(1);
    const int queryStart = target.indexOf('?');
    if (queryStart >= 0)
        target.truncate(queryStart);
    return target;
}

} // namespace

HttpServer::HttpServer(QHash<QByteArray, HttpResource> routes, QObject *parent)
    : QObject(parent)
    , m_routes(std::move(routes))
    , m_server(new QTcpServer(this))
{
    connect(m_server, &QTcpServer::newConnection, this, [this]() {
        while (QTcpSocket *socket = m_server->nextPendingConnection())
            serveConnection(socket);
    });
}

bool HttpServer::listen()
{
    if (!m_server->listen(QHostAddress::LocalHost)) {
        qWarning("Failed to start HTTP server: %s", qPrintable(m_server->errorString()));
        return false;
    }
    return true;
}

quint16 HttpServer::port() const
{
    return m_server->serverPort();
}

void HttpServer::serveConnection(QTcpSocket *socket)
{
    connect(socket, &QTcpSocket::disconnected, socket, &QObject::deleteLater);
    connect(socket, &QTcpSocket::readyRead, socket,
            [this, socket, request = QByteArray()]() mutable {
        if (socket->state() != QAbstractSocket::ConnectedState)
            return;

        request += socket->readAll();

        const std::optional<QByteArray> target = tryParseRequestTarget(request);
        if (!target) {
            if (request.size() > maxRequestSize) {
                qWarning("HTTP server: request line exceeded %d bytes; dropping connection",
                         maxRequestSize);
                socket->disconnectFromHost();
            }
            return;
        }

        const auto route = m_routes.constFind(*target);
        const auto response = route != m_routes.constEnd()
            ? makeHttpResponse(200, "OK", route->mimeType, route->body)
            : makeHttpResponse(404, "Not Found", "text/plain", "Not Found\n");
        socket->write(response);
        socket->disconnectFromHost();
    });
}
