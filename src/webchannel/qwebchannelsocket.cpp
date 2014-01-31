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

#include "qwebchannelsocket_p.h"

#include <QUuid>
#include <QDebug>

#include <QtWebSockets/QWebSocket>

QT_BEGIN_NAMESPACE

QWebChannelSocket::QWebChannelSocket(QObject *parent)
    : QWebSocketServer(QStringLiteral("QWebChannel Server"), NonSecureMode, parent)
    , m_messageHandler(Q_NULLPTR)
    , m_useSecret(true)
    , m_starting(false)
{
    connect(this, SIGNAL(acceptError(QAbstractSocket::SocketError)),
            SLOT(socketError()));
    connect(this, SIGNAL(newConnection()),
            SLOT(validateNewConnection()));
}

QWebChannelSocket::~QWebChannelSocket()
{
    close();
    qDeleteAll(m_clients);
}

void QWebChannelSocket::initLater()
{
    if (m_starting)
        return;
    metaObject()->invokeMethod(this, "init", Qt::QueuedConnection);
    m_starting = true;
}

void QWebChannelSocket::sendMessage(const QString &message)
{
    foreach (QWebSocket *client, m_clients) {
        qint64 bytesWritten = client->sendTextMessage(message);
        Q_UNUSED(bytesWritten);
    }
}

void QWebChannelSocket::validateNewConnection()
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

void QWebChannelSocket::init()
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

    if (!listen(QHostAddress::LocalHost)) {
        emit failed(errorString());
        return;
    }

    m_baseUrl = QStringLiteral("127.0.0.1:%1%2").arg(serverPort()).arg(m_secret);
    emit initialized();
    emit baseUrlChanged(m_baseUrl);
}

void QWebChannelSocket::socketError()
{
    emit failed(errorString());
}

void QWebChannelSocket::messageReceived(const QString &message)
{
    if (m_messageHandler) {
        m_messageHandler->handleMessage(message);
    }
    emit textDataReceived(message);
}

void QWebChannelSocket::clientDisconnected()
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

QT_END_NAMESPACE
