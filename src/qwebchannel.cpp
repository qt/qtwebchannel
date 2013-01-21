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

#include <QFile>
#include <QPointer>
#include <QStringList>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTimer>
#include <QUuid>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

QWebChannelResponder::QWebChannelResponder(QTcpSocket* s)
    : QObject(s)
    , socket(s)
    , autoDeleteTimer(new QTimer(this))
{
    connect(socket.data(), SIGNAL(disconnected()), socket.data(), SLOT(deleteLater()));
    connect(autoDeleteTimer, SIGNAL(timeout()), this, SLOT(noop()));
    autoDeleteTimer->setSingleShot(true);
    autoDeleteTimer->start(0);
}

void QWebChannelResponder::retain()
{
    autoDeleteTimer->stop();
}

void QWebChannelResponder::noop()
{
    open(0);
    close();
}

void QWebChannelResponder::send(const QString& stringData)
{
    const QByteArray data = stringData.toUtf8();
    open(data.length());
    write(data);
    close();
}

void QWebChannelResponder::open(uint contentLength)
{
    if (!socket || !socket->isOpen()) {
        qWarning() << "cannot open response - socket is not open anymore";
        return;
    }

    retain();

    socket->write("HTTP/1.1 200 OK\r\n"
                  "Content-Type: text/json\r\n"
                  "Content-Length: " + QByteArray::number(contentLength) + "\r\n"
                  "\r\n");
}

void QWebChannelResponder::write(const QByteArray& data)
{
    if (!socket || !socket->isOpen()) {
        qWarning() << "cannot write response - socket is not open anymore";
        return;
    }
    socket->write(data);
}

void QWebChannelResponder::close()
{
    if (!socket) {
        qWarning() << "cannot close - socket is not available anymore";
        return;
    }
    deleteLater();
    socket->close();
}

struct HttpRequestData {
    enum State { BeginState, AfterFirstLineState, AfterHeadersState, DoneState };

    State state;
    QString firstLine;
    QMap<QString, QString> headers;
    int contentLength;
    QString content;
    HttpRequestData()
        : state(BeginState),
          contentLength(0) { }
};

class QWebChannelPrivate : public QObject {
    Q_OBJECT
public:
    bool useSecret;
    bool autoRetain;
    int port;
    int minPort;
    int maxPort;
    QUrl baseUrl;
    QTcpServer* server;
    QString secret;
    typedef QPair<QString, QString> Broadcast;
    typedef QVector< Broadcast > BroadcastList;
    BroadcastList pendingBroadcasts;
    QPointer<QTcpSocket> pollSocket;
    bool starting;
    QMap<QTcpSocket*, HttpRequestData> pendingData;

    QWebChannelPrivate(QObject* parent)
        : QObject(parent)
        , useSecret(true)
        , autoRetain(false)
        , port(-1)
        , minPort(49158)
        , maxPort(65535)
        , server(new QTcpServer(this))
        , secret("42")
        , starting(false)
    {
        connect(server, SIGNAL(newConnection()), this, SLOT(service()));
        connect(server, SIGNAL(acceptError(QAbstractSocket::SocketError)), SLOT(acceptError(QAbstractSocket::SocketError)));
    }
    void initLater()
    {
        if (starting)
            return;
        metaObject()->invokeMethod(this, "init", Qt::QueuedConnection   );
        starting = true;
    }

    void handleHttpRequest(QTcpSocket* socket, const HttpRequestData& data);

    void submitBroadcasts(QTcpSocket* socket);

public slots:
    void init();
    void broadcast(const QString&, const QString&);
    void service();
    void handleSocketData();
    void handleSocketData(QTcpSocket*);
    void acceptError(QAbstractSocket::SocketError);
    void socketError(QAbstractSocket::SocketError);

signals:
    void execute(const QString&, QObject*);
    void initialized();
    void noPortAvailable();
};

void QWebChannelPrivate::acceptError(QAbstractSocket::SocketError error)
{
    qWarning() << "SERVER ERROR" << server->errorString() << error;
}

void QWebChannelPrivate::socketError(QAbstractSocket::SocketError error)
{
    qWarning() << "SOCKET ERROR" << qobject_cast<QTcpSocket*>(sender())->errorString() << error;
}

void QWebChannelPrivate::submitBroadcasts(QTcpSocket* socket)
{
    socket->write("HTTP/1.1 200 OK\r\n"
                  "Content-Type: text/json\r\n");
    QJsonArray array;
    foreach(const Broadcast& broadcast, pendingBroadcasts) {
        QJsonObject obj;
        obj.insert(QStringLiteral("id"), broadcast.first);
        obj.insert(QStringLiteral("data"), broadcast.second);
        array.append(obj);
    }
    pendingBroadcasts.clear();
    QJsonDocument doc;
    doc.setArray(array);
    const QByteArray jsonData = doc.toJson();
    socket->write("Content-Length: " + QByteArray::number(jsonData.length()) + "\r\n"
                  "\r\n");
    socket->write(doc.toJson());
    socket->close();
}

void QWebChannelPrivate::broadcast(const QString& id, const QString& message)
{
    pendingBroadcasts << qMakePair(id, message);

    if (pollSocket) {
        submitBroadcasts(pollSocket);
        pollSocket = 0;
    }
}

void QWebChannelPrivate::handleSocketData()
{
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket)
        return;
    handleSocketData(socket);

}

void QWebChannelPrivate::handleSocketData(QTcpSocket* socket)
{
    HttpRequestData* data = &pendingData[socket];
    switch (data->state) {
    case HttpRequestData::BeginState:
        if (!socket->canReadLine())
            return;
        data->firstLine = socket->readLine().trimmed();
        data->state = HttpRequestData::AfterFirstLineState;
        handleSocketData(socket);
        return;

    case HttpRequestData::AfterFirstLineState: {
        while (socket->canReadLine()) {
            QString line = socket->readLine();
            if (line == "\r\n") {
                data->state = HttpRequestData::AfterHeadersState;
                data->contentLength = data->headers["Content-Length"].toInt();
                handleSocketData(socket);
                return;
            }

            QStringList split = line.split(": ");
            data->headers[split[0]] = split[1].trimmed();
        }
        return;
    }

    case HttpRequestData::AfterHeadersState:
        data->content += socket->readAll();
        if (data->content.size() != data->contentLength)
            return;
        data->state = HttpRequestData::DoneState;
        handleHttpRequest(socket, *data);
        pendingData.remove(socket);
        return;

    case HttpRequestData::DoneState:
        return;
    }
}

void QWebChannelPrivate::handleHttpRequest(QTcpSocket *socket, const HttpRequestData &data)
{
    QStringList firstLineValues = data.firstLine.split(' ');
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
        queryMap[q.left(idx)] = q.mid(idx + 1).trimmed();
    }

    QStringList pathElements = path.split('/');
    pathElements.removeFirst();

    if ((useSecret && pathElements[0] != secret)) {
        socket->write(
                            "HTTP/1.1 401 Wrong Path\r\n"
                            "Content-Type: text/json\r\n"
                            "\r\n"
                            "<html><body><h1>Wrong Path</h1></body></html>"
                    );
        socket->close();
        return;
    }

    QString type = pathElements[1];
    if (method == "POST") {
        if (type == "EXEC") {
            QWebChannelResponder* responder = new QWebChannelResponder(socket);
            if (autoRetain)
                responder->retain();
            emit execute(data.content, responder);
        } else if (type == "POLL") {
            ///FIXME: this should be rewritten using a proper web socket approach
            if (!pendingBroadcasts.isEmpty()) {
                submitBroadcasts(socket);
            } else {
                Q_ASSERT(!pollSocket);
                // defer transmission until broadcast comes in
                pollSocket = socket;
                connect(socket, SIGNAL(disconnected()), socket, SLOT(deleteLater()));
            }
        }
    } else if (method == "GET") {
        if (type == "webchannel.js") {
            QFile file(":/webchannel.js");
            QString initFunction = pathElements[2];
            file.open(QIODevice::ReadOnly);
            socket->write("HTTP/1.1 200 OK\r\n"
                          "Content-Type: text/javascript\r\n"
                          "\r\n");
            socket->write("(function() {\n");
            socket->write(QString("var baseUrl = '%1';\n").arg(baseUrl.toString()).toUtf8());
            socket->write(QString("var initFunction = '%1';\n").arg(initFunction).toUtf8());
            socket->write(file.readAll());
            socket->write("\n})();");
            socket->close();
            file.close();
        } else if (type == "qobject.js") {
            QFile file(":/qobject.js");
            file.open(QIODevice::ReadOnly);
            socket->write("HTTP/1.1 200 OK\r\n"
                          "Content-Type: text/javascript\r\n"
                          "\r\n");
            socket->write(file.readAll());
            socket->close();
            file.close();
        } else if (type == "iframe.html") {
            QFile file(":/webchannel-iframe.html");
            file.open(QIODevice::ReadOnly);
            socket->write("HTTP/1.1 200 OK\r\n"
                          "Content-Type: text/html\r\n"
                          "\r\n");
            socket->write(file.readAll());
            socket->close();
            file.close();
        }
    }
}

void QWebChannelPrivate::service()
{
    if (!server->hasPendingConnections())
        return;

    QTcpSocket* socket = server->nextPendingConnection();
    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), SLOT(socketError(QAbstractSocket::SocketError)));
    handleSocketData(socket);
    connect(socket, SIGNAL(readyRead()), this, SLOT(handleSocketData()));

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
bool QWebChannel::autoRetain() const
{
    return d->autoRetain;
}

void QWebChannel::setAutoRetain(bool a)
{
    d->autoRetain = a;
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
    emit initialized();
    emit baseUrlChanged(d->baseUrl);
}

void QWebChannel::broadcast(const QString& id, const QString& data)
{
    d->broadcast(id, data);
}

#include <qwebchannel.moc>
