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
#include "qwebchannel_p.h"
#include "qmetaobjectpublisher_p.h"
#include "qwebchannelsocket_p.h"

#include <QJsonDocument>
#include <QJsonObject>

void QWebChannelPrivate::sendJSONMessage(const QJsonValue &id, const QJsonValue &data, bool response) const
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
    socket->sendMessage(doc.toJson(QJsonDocument::Compact));
}

QWebChannel::QWebChannel(QObject *parent)
: QObject(parent)
, d(new QWebChannelPrivate)
{
    d->socket = new QWebChannelSocket(this);

    connect(d->socket, SIGNAL(textDataReceived(QString)),
            SIGNAL(rawMessageReceived(QString)));
    connect(d->socket, SIGNAL(failed(QString)),
            SIGNAL(failed(QString)));
    connect(d->socket, SIGNAL(initialized()),
            SLOT(onInitialized()));
    connect(d->socket, SIGNAL(pongReceived()),
            SIGNAL(pongReceived()));

    d->socket->initLater();

    d->publisher = new QMetaObjectPublisher(this);
    connect(d->publisher, SIGNAL(blockUpdatesChanged(bool)),
            SIGNAL(blockUpdatesChanged(bool)));
    connect(d->socket, SIGNAL(textDataReceived(QString)),
            d->publisher, SLOT(handleRawMessage(QString)));
}

QWebChannel::~QWebChannel()
{
}

QString QWebChannel::baseUrl() const
{
    return d->socket->m_baseUrl;
}

void QWebChannel::setUseSecret(bool s)
{
    if (d->socket->m_useSecret == s)
        return;
    d->socket->m_useSecret = s;
    d->socket->initLater();
}

bool QWebChannel::useSecret() const
{
    return d->socket->m_useSecret;
}

void QWebChannel::registerObjects(const QHash< QString, QObject * > &objects)
{
    const QHash<QString, QObject *>::const_iterator end = objects.constEnd();
    for (QHash<QString, QObject *>::const_iterator it = objects.constBegin(); it != end; ++it) {
        d->publisher->registerObject(it.key(), it.value());
    }
}

void QWebChannel::registerObject(const QString &id, QObject *object)
{
    d->publisher->registerObject(id, object);
}

bool QWebChannel::blockUpdates() const
{
    return d->publisher->blockUpdates;
}

void QWebChannel::setBlockUpdates(bool block)
{
    d->publisher->setBlockUpdates(block);
}

void QWebChannel::onInitialized()
{
    emit initialized();
    emit baseUrlChanged(d->socket->m_baseUrl);
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
    d->socket->sendMessage(message.toUtf8());
}

void QWebChannel::ping() const
{
    d->socket->ping();
}

#include "qwebchannel.moc"
