/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Copyright (C) 2013 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com, author Milian Wolff <milian.wolff@kdab.com>
** Contact: http://www.qt-project.org/legal
*
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

#include "qwebchannel.h"

#include <QUuid>
#include <QStringList>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>

#include "qwebsocketserver_p.h"

class QWebChannelPrivate : public QWebSocketServer
{
    Q_OBJECT
public:
    QByteArray m_secret;
    bool m_useSecret;

    QString m_baseUrl;
    bool m_starting;

    QWebChannelPrivate(QObject* parent)
    : QWebSocketServer(parent)
    , m_useSecret(true)
    , m_starting(false)
    {
        connect(this, SIGNAL(error(QAbstractSocket::SocketError)),
                SLOT(socketError()));
    }

    void initLater()
    {
        if (m_starting)
            return;
        metaObject()->invokeMethod(this, "init", Qt::QueuedConnection);
        m_starting = true;
    }

    void sendJSONMessage(const QJsonValue& id, const QJsonValue& data, bool response) const;

signals:
    void failed(const QString& reason);
    void initialized();

protected:
    bool isValid(const HeaderData& connection) Q_DECL_OVERRIDE;

private slots:
    void init();
    void socketError();
};

bool QWebChannelPrivate::isValid(const HeaderData& connection)
{
    if (!QWebSocketServer::isValid(connection)) {
        return false;
    }
    return connection.protocol == QByteArrayLiteral("QWebChannel")
            && connection.path == m_secret;
}

void QWebChannelPrivate::init()
{
    close();

    m_starting = false;
    if (m_useSecret) {
        m_secret = QUuid::createUuid().toByteArray();
        // replace { by /
        m_secret[0] = '/';
        // chop of trailing }
        m_secret.chop(1);
    }

    if (!listen(QHostAddress::LocalHost)) {
        emit failed(errorString());
        return;
    }

    m_baseUrl = QStringLiteral("127.0.0.1:%1%2").arg(port()).arg(QString::fromLatin1(m_secret));
    emit initialized();
}

void QWebChannelPrivate::socketError()
{
    emit failed(errorString());
}

void QWebChannelPrivate::sendJSONMessage(const QJsonValue& id, const QJsonValue& data, bool response) const
{
    QJsonObject obj;
    if (response) {
        obj[QStringLiteral("response")] = true;
    }
    obj[QStringLiteral("id")] = id;
    if (!data.isNull()) {
        obj[QStringLiteral("data")] = data;
    }
    QJsonDocument doc(obj);
    sendMessage(doc.toJson(QJsonDocument::Compact));
}

QWebChannel::QWebChannel(QObject *parent)
: QObject(parent)
, d(new QWebChannelPrivate(this))
{
    connect(d, SIGNAL(textDataReceived(QString)),
            SIGNAL(rawMessageReceived(QString)));
    connect(d, SIGNAL(failed(QString)),
            SIGNAL(failed(QString)));
    connect(d, SIGNAL(initialized()),
            SLOT(onInitialized()));
    connect(d, SIGNAL(pongReceived()),
            SIGNAL(pongReceived()));
    d->initLater();
}

QWebChannel::~QWebChannel()
{
}

QString QWebChannel::baseUrl() const
{
    return d->m_baseUrl;
}

void QWebChannel::setUseSecret(bool s)
{
    if (d->m_useSecret == s)
        return;
    d->m_useSecret = s;
    d->initLater();
}

bool QWebChannel::useSecret() const
{
    return d->m_useSecret;
}

void QWebChannel::onInitialized()
{
    emit initialized();
    emit baseUrlChanged(d->m_baseUrl);
}

void QWebChannel::respond(const QJsonValue& messageId, const QJsonValue& data) const
{
    d->sendJSONMessage(messageId, data, true);
}

void QWebChannel::sendMessage(const QJsonValue& id, const QJsonValue& data) const
{
    d->sendJSONMessage(id, data, false);
}

void QWebChannel::sendRawMessage(const QString& message) const
{
    d->sendMessage(message.toUtf8());
}

void QWebChannel::ping() const
{
    d->ping();
}

#include "qwebchannel.moc"
