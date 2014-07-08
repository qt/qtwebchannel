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

#include "qqmlwebchannel.h"

#include "qwebchannel_p.h"
#include "qmetaobjectpublisher_p.h"
#include "qwebchannelabstracttransport.h"

#include <QtQml/QQmlContext>

#include "qqmlwebchannelattached_p.h"

QT_BEGIN_NAMESPACE

class QQmlWebChannelPrivate : public QWebChannelPrivate
{
    Q_DECLARE_PUBLIC(QQmlWebChannel)
public:
    QVector<QObject*> registeredObjects;

    void _q_objectIdChanged(const QString &newId);
};

void QQmlWebChannelPrivate::_q_objectIdChanged(const QString &newId)
{
    Q_Q(QQmlWebChannel);
    const QQmlWebChannelAttached *const attached = qobject_cast<QQmlWebChannelAttached*>(q->sender());
    Q_ASSERT(attached);
    Q_ASSERT(attached->parent());
    Q_ASSERT(registeredObjects.contains(attached->parent()));

    QObject *const object = attached->parent();
    const QString &oldId = publisher->registeredObjectIds.value(object);

    if (!oldId.isEmpty()) {
        q->deregisterObject(object);
    }

    q->registerObject(newId, object);
}

QQmlWebChannel::QQmlWebChannel(QObject *parent)
    : QWebChannel(*(new QQmlWebChannelPrivate), parent)
{
}

QQmlWebChannel::~QQmlWebChannel()
{

}

void QQmlWebChannel::registerObjects(const QVariantMap &objects)
{
    Q_D(QQmlWebChannel);
    QMap<QString, QVariant>::const_iterator it = objects.constBegin();
    for (; it != objects.constEnd(); ++it) {
        QObject *object = it.value().value<QObject*>();
        if (!object) {
            qWarning("Invalid QObject given to register under name %s", qPrintable(it.key()));
            continue;
        }
        d->publisher->registerObject(it.key(), object);
    }
}

QQmlWebChannelAttached *QQmlWebChannel::qmlAttachedProperties(QObject *obj)
{
    return new QQmlWebChannelAttached(obj);
}

void QQmlWebChannel::connectTo(QObject *transport)
{
    if (QWebChannelAbstractTransport *realTransport = qobject_cast<QWebChannelAbstractTransport*>(transport)) {
        QWebChannel::connectTo(realTransport);
    } else {
        qWarning() << "Cannot connect to transport" << transport << " - it is not a QWebChannelAbstractTransport.";
    }
}

void QQmlWebChannel::disconnectFrom(QObject *transport)
{
    if (QWebChannelAbstractTransport *realTransport = qobject_cast<QWebChannelAbstractTransport*>(transport)) {
        QWebChannel::disconnectFrom(realTransport);
    } else {
        qWarning() << "Cannot disconnect from transport" << transport << " - it is not a QWebChannelAbstractTransport.";
    }
}

QQmlListProperty<QObject> QQmlWebChannel::registeredObjects()
{
    return QQmlListProperty<QObject>(this, 0,
                                     registeredObjects_append,
                                     registeredObjects_count,
                                     registeredObjects_at,
                                     registeredObjects_clear);
}

void QQmlWebChannel::registeredObjects_append(QQmlListProperty<QObject> *prop, QObject *object)
{
    const QQmlWebChannelAttached *const attached = qobject_cast<QQmlWebChannelAttached*>(
        qmlAttachedPropertiesObject<QQmlWebChannel>(object, false /* don't create */));
    if (!attached) {
        const QQmlContext *const context = qmlContext(object);
        qWarning() << "Cannot register object" << context->nameForObject(object) << '(' << object << ") without attached WebChannel.id property. Did you forget to set it?";
        return;
    }
    QQmlWebChannel *channel = static_cast<QQmlWebChannel*>(prop->object);
    if (!attached->id().isEmpty()) {
        // TODO: warning in such cases?
        channel->registerObject(attached->id(), object);
    }
    channel->d_func()->registeredObjects.append(object);
    connect(attached, SIGNAL(idChanged(QString)), channel, SLOT(_q_objectIdChanged(QString)));
}

int QQmlWebChannel::registeredObjects_count(QQmlListProperty<QObject> *prop)
{
    return static_cast<QQmlWebChannel*>(prop->object)->d_func()->registeredObjects.size();
}

QObject *QQmlWebChannel::registeredObjects_at(QQmlListProperty<QObject> *prop, int index)
{
    return static_cast<QQmlWebChannel*>(prop->object)->d_func()->registeredObjects.at(index);
}

void QQmlWebChannel::registeredObjects_clear(QQmlListProperty<QObject> *prop)
{
    QQmlWebChannel *channel = static_cast<QQmlWebChannel*>(prop->object);
    foreach (QObject *object, channel->d_func()->registeredObjects) {
        channel->deregisterObject(object);
    }
    return channel->d_func()->registeredObjects.clear();
}

QQmlListProperty<QObject> QQmlWebChannel::transports()
{
    return QQmlListProperty<QObject>(this, 0,
                                                           transports_append,
                                                           transports_count,
                                                           transports_at,
                                                           transports_clear);
}

void QQmlWebChannel::transports_append(QQmlListProperty<QObject> *prop, QObject *transport)
{
    QQmlWebChannel *channel = static_cast<QQmlWebChannel*>(prop->object);
    channel->connectTo(transport);
}

int QQmlWebChannel::transports_count(QQmlListProperty<QObject> *prop)
{
    return static_cast<QQmlWebChannel*>(prop->object)->d_func()->transports.size();
}

QObject *QQmlWebChannel::transports_at(QQmlListProperty<QObject> *prop, int index)
{
    QQmlWebChannel *channel = static_cast<QQmlWebChannel*>(prop->object);
    return channel->d_func()->transports.at(index);
}

void QQmlWebChannel::transports_clear(QQmlListProperty<QObject> *prop)
{
    QWebChannel *channel = static_cast<QWebChannel*>(prop->object);
    foreach (QWebChannelAbstractTransport *transport, channel->d_func()->transports) {
        channel->disconnectFrom(transport);
    }
    Q_ASSERT(channel->d_func()->transports.isEmpty());
}

QT_END_NAMESPACE

#include "moc_qqmlwebchannel.cpp"
