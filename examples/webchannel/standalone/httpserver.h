// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include <QByteArray>
#include <QHash>
#include <QObject>

QT_BEGIN_NAMESPACE
class QTcpServer;
class QTcpSocket;
QT_END_NAMESPACE

struct HttpResource
{
    QByteArray mimeType;
    QByteArray body;
};

// Serves a fixed table of in-memory resources over HTTP/1.1 on a loopback port,
// mapping each request path to a resource (404 otherwise).
class HttpServer : public QObject
{
    Q_OBJECT

public:
    explicit HttpServer(QHash<QByteArray, HttpResource> routes, QObject *parent = nullptr);

    // Binds an ephemeral loopback port. Returns false, after logging a warning,
    // if the port cannot be bound.
    bool listen();

    // The bound port, or 0 while not listening.
    quint16 port() const;

private:
    void serveConnection(QTcpSocket *socket);

    QHash<QByteArray, HttpResource> m_routes;
    QTcpServer *m_server;
};

#endif // HTTPSERVER_H
