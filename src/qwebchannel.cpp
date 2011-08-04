/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QWebChannel module on Qt labs.
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
**
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qwebchannel.h"

#include <QtDeclarative/qdeclarative.h>
#include <QFile>
#include <QPointer>
#include <QStringList>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTimer>
#include <QUuid>

QWebChannelResponder::QWebChannelResponder(QTcpSocket* s)
    : QObject(s)
    , socket(s)
{
    connect(socket.data(), SIGNAL(disconnected()), socket.data(), SLOT(deleteLater()));
}

void QWebChannelResponder::open()
{
    if (!socket || !socket->isOpen())
        return;

    socket->write("HTTP/1.1 200 OK\r\n"
                  "Content-Type: text/json\r\n"
                  "\r\n");
}

void QWebChannelResponder::write(const QString& data)
{
    if (!socket || !socket->isOpen())
        return;
    socket->write(data.toUtf8());
}

void QWebChannelResponder::close()
{
    if (!socket)
        return;
    deleteLater();
    socket->close();
}

class QWebChannelPrivate : public QObject {
    Q_OBJECT
public:
    bool useSecret;
    int port;
    int minPort;
    int maxPort;
    QUrl baseUrl;
    QTcpServer* server;
    QString secret;
    typedef QMultiMap<QString, QPointer<QTcpSocket> > SubscriberMap;
    SubscriberMap subscribers;
    bool starting;

    QWebChannelPrivate(QObject* parent)
        : QObject(0)
        , useSecret(true)
        , port(-1)
        , minPort(49158)
        , maxPort(65535)
        , server(new QTcpServer(this))
        , secret("42")
        , starting(false)
    {
        connect(server, SIGNAL(newConnection()), this, SLOT(service()));
    }
    void initLater()
    {
        if (starting)
            return;
        metaObject()->invokeMethod(this, "init", Qt::QueuedConnection   );
        starting = true;
    }

public slots:
    void init();
    void broadcast(const QString&, const QString&);
    void service();
    void onSocketDelete(QObject*);

signals:
    void execute(const QString&, QObject*);
    void initialized();
    void noPortAvailable();
};

void QWebChannelPrivate::onSocketDelete(QObject * o)
{
    for (SubscriberMap::iterator it = subscribers.begin(); it != subscribers.end(); ++it) {
        if (it.value() != o)
            continue;
        subscribers.erase(it);
        return;
    }
}

void QWebChannelPrivate::broadcast(const QString& id, const QString& message)
{
    QList<QPointer<QTcpSocket> > sockets = subscribers.values(id);

    foreach(QPointer<QTcpSocket> socket, sockets) {
        if (!socket)
            continue;
        socket->write("HTTP/1.1 200 OK\r\n"
                      "Content-Type: text/json\r\n"
                      "\r\n");
        socket->write(message.toUtf8());
        socket->close();
    }
}

void QWebChannelPrivate::service()
{
    if (!server->hasPendingConnections())
        return;

    QTcpSocket* socket = server->nextPendingConnection();
    if (!socket->canReadLine())
        if (!socket->waitForReadyRead(1000))
            return;


    QString firstLine = socket->readLine();
    QStringList firstLineValues = firstLine.split(' ');
    QString method = firstLineValues[0];
    QString path = firstLineValues[1];
    QString query;
    QString hash;
    int indexOfQM = path.indexOf('?');
    if (indexOfQM > 0) {
        query = path.mid(indexOfQM + 1);
        path = path.left(indexOfQM);
        int indexOfHash = query.indexOf('#');
        if (indexOfHash > 0) {
            hash = query.mid(indexOfHash + 1);
            query = query.left(indexOfHash);
        }
    } else {
        int indexOfHash = path.indexOf('#');
        if (indexOfHash > 0) {
            hash = path.mid(indexOfHash + 1);
            path = path.left(indexOfHash);
        }
    }

    QStringList queryVars = query.split('&');
    QMap<QString, QString> queryMap;
    foreach (QString q, queryVars) {
        int idx = q.indexOf("=");
        queryMap[q.left(idx)] = q.mid(idx + 1);
    }

    while (!socket->canReadLine())
        socket->waitForReadyRead();
    QString requestString = socket->readAll();
    QStringList headersAndContent = requestString.split("\r\n\r\n");
    QStringList headerList = headersAndContent[0].split("\r\n");
    QMap<QString, QString> headerMap;
    foreach (QString h, headerList) {
        int idx = h.indexOf(": ");
        headerMap[h.left(idx)] = h.mid(idx + 2);
    }

    QStringList pathElements = path.split('/');
    pathElements.removeFirst();

    if ((useSecret && pathElements[0] != secret)) {
        socket->write(
                            "HTTP/1.1 401 Wrong Path\r\n"
                            "Cache-Control: No-Cache\r\n"
                            "Content-Type: text/json\r\n"
                            "\r\n"
                            "<html><body><h1>Wrong Path</h1></body></html>"
                    );
        socket->close();
        return;
    }

    if (method == "GET") {
        QString type = pathElements.size() == 1 ? "j" : pathElements[1];
        if (type == "e") {
            QString message = QStringList(pathElements.mid(2)).join("/");
            QWebChannelResponder* responder = new QWebChannelResponder(socket);
            QString msg = QUrl::fromPercentEncoding(message.toUtf8());
            emit execute(msg, responder);
        } else if (type == "j") {
            QFile file(":/webchannel.js");
            file.open(QIODevice::ReadOnly);
            socket->write("HTTP/1.1 200 OK\r\n"
                          "Content-Type: text/javascript\r\n"
                          "Cache-Control: No-Cache\r\n"
                          "\r\n");
            socket->write("window.onload = function() {");
            socket->write(QString("baseUrl = '%1';").arg(baseUrl.toString()).toUtf8());
            socket->write(file.readAll());
            socket->write("};");
            socket->close();
            file.close();
        } else if (type == "h") {
            QFile file(":/webchannel-iframe.html");
            file.open(QIODevice::ReadOnly);
            socket->write("HTTP/1.1 200 OK\r\n"
                          "Content-Type: text/html\r\n"
                          "Cache-Control: No-Cache\r\n"
                          "\r\n");
            socket->write(file.readAll());
            socket->close();
            file.close();
        } else if (type == "s") {
            QString id = pathElements[2];
            connect(socket, SIGNAL(disconnected()), socket, SLOT(deleteLater()));
            connect(socket, SIGNAL(destroyed(QObject*)), this, SLOT(onSocketDelete(QObject*)));
            subscribers.insert(id, socket);
        }
    }
}

void QWebChannelPrivate::init()
{
    starting = false;
    if (useSecret) {
        secret = QUuid::createUuid().toString();
        secret = secret.mid(1, secret.size() - 2);
    }

    bool found = false;
    for (port = minPort; port <= maxPort; ++port) {
        if (!server->listen(QHostAddress::LocalHost, port))
            continue;
        found = true;
        break;
    }

    if (!found) {
        port = -1;
        emit noPortAvailable();
        return;
    }

    baseUrl = QString("http://localhost:%1/%2").arg(port).arg(secret);
    emit initialized();
}

QWebChannel::QWebChannel(QObject *parent):
        QObject(parent)
{
    d = new QWebChannelPrivate(this);
    connect(d, SIGNAL(execute(QString,QObject*)), this, SIGNAL(execute(QString, QObject*)));
    connect(d, SIGNAL(initialized()), this, SLOT(onInitialized()));
    connect(d, SIGNAL(noPortAvailable()), this, SIGNAL(noPortAvailable()));
    d->initLater();
}

QWebChannel::~QWebChannel()
{
}

QUrl QWebChannel::baseUrl() const
{
    return d->baseUrl;
}
void QWebChannel::setUseSecret(bool s)
{
    if (d->useSecret == s)
        return;
    d->useSecret = s;
    d->initLater();
}
bool QWebChannel::useSecret() const
{
    return d->useSecret;
}

int QWebChannel::port() const
{
    return d->port;
}

int QWebChannel::minPort() const
{
    return d->minPort;
}

int QWebChannel::maxPort() const
{
    return d->maxPort;
}

void QWebChannel::setMinPort(int p)
{
    if (d->minPort == p)
        return;
    d->minPort = p;
    d->initLater();
}

void QWebChannel::setMaxPort(int p)
{
    if (d->maxPort == p)
        return;
    d->maxPort = p;
    d->initLater();
}

void QWebChannel::onInitialized()
{
    emit baseUrlChanged(d->baseUrl);
}

void QWebChannel::broadcast(const QString& id, const QString& data)
{
    d->broadcast(id, data);
}


#include <qwebchannel.moc>
