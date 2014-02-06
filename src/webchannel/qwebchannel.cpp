/****************************************************************************
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
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

#include "qwebchannel.h"
#include "qwebchannel_p.h"
#include "qmetaobjectpublisher_p.h"
#include "qwebchanneltransportinterface.h"

#include <QJsonDocument>
#include <QJsonObject>

QT_BEGIN_NAMESPACE

void QWebChannelPrivate::sendJSONMessage(const QJsonValue &id, const QJsonValue &data, bool response) const
{
    if (transports.isEmpty()) {
        qWarning("QWebChannel is not connected to any transports, cannot send messages.");
        return;
    }

    QJsonObject obj;
    if (response) {
        obj[QStringLiteral("response")] = true;
    }
    obj[QStringLiteral("id")] = id;
    if (!data.isNull()) {
        obj[QStringLiteral("data")] = data;
    }
    QJsonDocument doc(obj);
    const QByteArray &message = doc.toJson(QJsonDocument::Compact);

    foreach (QWebChannelTransportInterface *transport, transports) {
        transport->sendMessage(message);
    }
}

QWebChannel::QWebChannel(QObject *parent)
: QObject(parent)
, d(new QWebChannelPrivate)
{
    d->publisher = new QMetaObjectPublisher(this);
    connect(d->publisher, SIGNAL(blockUpdatesChanged(bool)),
            SIGNAL(blockUpdatesChanged(bool)));
}

QWebChannel::~QWebChannel()
{
    foreach (QWebChannelTransportInterface *transport, d->transports) {
        transport->setMessageHandler(Q_NULLPTR);
    }
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

void QWebChannel::deregisterObject(QObject *object)
{
    // handling of deregistration is analogously to handling of a destroyed signal
    d->publisher->signalEmitted(object, s_destroyedSignalIndex, QVariantList() << QVariant::fromValue(object));
}

bool QWebChannel::blockUpdates() const
{
    return d->publisher->blockUpdates;
}

void QWebChannel::setBlockUpdates(bool block)
{
    d->publisher->setBlockUpdates(block);
}

void QWebChannel::connectTo(QWebChannelTransportInterface *transport)
{
    Q_ASSERT(transport);
    if (!d->transports.contains(transport)) {
        d->transports << transport;
        transport->setMessageHandler(d->publisher);
    }
}

void QWebChannel::disconnectFrom(QWebChannelTransportInterface *transport)
{
    const int idx = d->transports.indexOf(transport);
    if (idx != -1) {
        transport->setMessageHandler(Q_NULLPTR);
        d->transports.remove(idx);
    }
}

void QWebChannel::respond(const QJsonValue &messageId, const QJsonValue &data) const
{
    d->sendJSONMessage(messageId, data, true);
}

void QWebChannel::sendMessage(const QJsonValue &id, const QJsonValue &data) const
{
    d->sendJSONMessage(id, data, false);
}

QT_END_NAMESPACE
