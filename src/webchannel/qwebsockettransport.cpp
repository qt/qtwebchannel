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

#include "qwebsockettransport.h"
#include "qwebsockettransport_p.h"

#include <QUuid>

#include <QtWebSockets/QWebSocket>

QT_USE_NAMESPACE

//BEGIN QWebSocketTransportPrivate

QWebSocketTransportPrivate::QWebSocketTransportPrivate(QWebSocketTransport *transport, QObject *parent)
    : QWebSocketServer(QStringLiteral("QWebChannel Server"), NonSecureMode, parent)
    , m_messageHandler(Q_NULLPTR)
    , m_transport(transport)
    , m_useSecret(true)
    , m_starting(false)
    , m_localAddress(QHostAddress::LocalHost)
    , m_localPort(0)
{
    connect(this, SIGNAL(acceptError(QAbstractSocket::SocketError)),
            SLOT(socketError()));
    connect(this, SIGNAL(newConnection()),
            SLOT(validateNewConnection()));
}

QWebSocketTransportPrivate::~QWebSocketTransportPrivate()
{
    close();
    qDeleteAll(m_clients);
}

void QWebSocketTransportPrivate::initLater()
{
    if (m_starting)
        return;
    metaObject()->invokeMethod(this, "init", Qt::QueuedConnection);
    m_starting = true;
}

void QWebSocketTransportPrivate::sendMessage(const QString &message, int clientId)
{
    if (clientId == -1) {
        foreach (QWebSocket *client, m_clients) {
            client->sendTextMessage(message);
        }
    } else {
        m_clients.at(clientId)->sendTextMessage(message);
    }
}

void QWebSocketTransportPrivate::validateNewConnection()
{
    QWebSocket *client = nextPendingConnection();
    // FIXME: client->protocol() != QStringLiteral("QWebChannel")
    // protocols are not supported in QtWebSockets yet...
    if (m_useSecret && client->requestUrl().path() != m_secret)
    {
        client->close(QWebSocketProtocol::CloseCodeBadOperation);
        client->deleteLater();
    } else {
        connect(client, SIGNAL(textMessageReceived(QString)),
                SLOT(messageReceived(QString)));
        connect(client, SIGNAL(disconnected()),
                SLOT(clientDisconnected()));
        m_clients << client;
    }
}

void QWebSocketTransportPrivate::init()
{
    close();

    m_starting = false;
    if (m_useSecret) {
        m_secret = QUuid::createUuid().toString();
        // replace { by /
        m_secret[0] = QLatin1Char('/');
        // chop of trailing }
        m_secret.chop(1);
    }

    if (!listen(m_localAddress, m_localPort)) {
        emit failed(errorString());
        return;
    }

    m_baseUrl = QStringLiteral("%1:%2%3").arg(m_localAddress.toString()).arg(serverPort()).arg(m_secret);
    emit initialized();
    emit baseUrlChanged(m_baseUrl);
}

void QWebSocketTransportPrivate::socketError()
{
    emit failed(errorString());
}

void QWebSocketTransportPrivate::messageReceived(const QString &message)
{
    if (m_messageHandler) {
        QWebSocket *client = qobject_cast<QWebSocket*>(sender());
        m_messageHandler->handleMessage(message, m_transport, m_clients.indexOf(client));
    }
    emit textDataReceived(message);
}

void QWebSocketTransportPrivate::clientDisconnected()
{
    QWebSocket *client = qobject_cast<QWebSocket*>(sender());
    if (!client) {
        return;
    }
    const int idx = m_clients.indexOf(client);
    Q_ASSERT(idx != -1);
    m_clients.remove(idx);
    client->deleteLater();
}

//END QWebSocketTransportPrivate

QWebSocketTransport::QWebSocketTransport(QObject *parent)
    : QObject(parent)
    , d(new QWebSocketTransportPrivate(this))
{
    connect(d.data(), SIGNAL(textDataReceived(QString)),
            SIGNAL(messageReceived(QString)));
    connect(d.data(), SIGNAL(failed(QString)),
            SIGNAL(failed(QString)));
    connect(d.data(), SIGNAL(initialized()),
            SIGNAL(initialized()));
    connect(d.data(), SIGNAL(baseUrlChanged(QString)),
            SIGNAL(baseUrlChanged(QString)));

    d->initLater();
}

QWebSocketTransport::QWebSocketTransport(const QHostAddress &address, quint16 port, QObject *parent)
    : QObject(parent)
    , d(new QWebSocketTransportPrivate(this))
{
    connect(d.data(), SIGNAL(textDataReceived(QString)),
            SIGNAL(messageReceived(QString)));
    connect(d.data(), SIGNAL(failed(QString)),
            SIGNAL(failed(QString)));
    connect(d.data(), SIGNAL(initialized()),
            SIGNAL(initialized()));
    connect(d.data(), SIGNAL(baseUrlChanged(QString)),
            SIGNAL(baseUrlChanged(QString)));

    d->m_localAddress = address;
    d->m_localPort = port;

    d->initLater();
}

QWebSocketTransport::~QWebSocketTransport()
{

}

void QWebSocketTransport::sendMessage(const QByteArray &message, int clientId) const
{
    d->sendMessage(QString::fromUtf8(message), clientId);
}

void QWebSocketTransport::sendMessage(const QString &message, int clientId) const
{
    d->sendMessage(message, clientId);
}

void QWebSocketTransport::setMessageHandler(QWebChannelMessageHandlerInterface *handler)
{
    d->m_messageHandler = handler;
}

QString QWebSocketTransport::baseUrl() const
{
    return d->m_baseUrl;
}

void QWebSocketTransport::setUseSecret(bool s)
{
    if (d->m_useSecret == s)
        return;
    d->m_useSecret = s;
    d->initLater();
}

bool QWebSocketTransport::useSecret() const
{
    return d->m_useSecret;
}
