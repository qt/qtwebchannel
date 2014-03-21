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

#include "qmlwebchannel.h"

#include "qwebchannel_p.h"
#include "qmetaobjectpublisher_p.h"

#include <QtQml/QQmlContext>

QT_USE_NAMESPACE

/*!
    \qmltype WebChannel
    \instantiates QmlWebChannel, QmlWebChannelAttached

    \inqmlmodule Qt.WebChannel
    \ingroup webchannel-qml
    \brief QML interface to QWebChannel.

    The WebChannel provides a mechanism to transparently access QObject or QML objects from HTML
    clients. All properties, signals and public slots can be used from the HTML clients.

    \sa {QML Example}
*/

/*!
  \qmlproperty QQmlListProperty<QObject> WebChannel::connections
  A list of connections which are objects implementing the QMessagePassingInterface. The connections
  are used to talk to the remote clients. Currently, only WebView.experimental and WebSocket
  implement this interface.

  \sa connectTo, disconnectFrom
  */

/*!
  \qmlproperty QQmlListProperty<QObject> WebChannel::registeredObjects
  A list of objects which should be accessible to remote clients. The objects must have the attached
  WebChannel.id property set to an identifier, under which the object is then known on the HTML side.

  Once registered, all signals and property changes are automatically propagated to the clients.
  Public invokable methods, including slots, are also accessible to the clients.

  If one needs to register objects which are not available when the component is created, use the
  imperative registerObjects method.

  \sa registerObjects
  */

QmlWebChannel::QmlWebChannel(QObject *parent)
    : QWebChannel(parent)
{
}

QmlWebChannel::~QmlWebChannel()
{

}

/*!
  Register objects to make them accessible to HTML clients. The key of the map is used as an identifier
  for the object on the client side.

  Once registered, all signals and property changes are automatically propagated to the clients.
  Public invokable methods, including slots, are also accessible to the clients.

  This imperative API can be used to register objects on the fly. For static objects, the declarative
  registeredObjects property should be preferred.

  \sa registeredObjects
  */
void QmlWebChannel::registerObjects(const QVariantMap &objects)
{
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

/*!
  \internal
  */
bool QmlWebChannel::test_clientIsIdle() const
{
    return d->publisher->clientIsIdle;
}

void QmlWebChannel::objectIdChanged(const QString &newId)
{
    const QmlWebChannelAttached *const attached = qobject_cast<QmlWebChannelAttached*>(sender());
    Q_ASSERT(attached);
    Q_ASSERT(attached->parent());
    Q_ASSERT(m_registeredObjects.contains(attached->parent()));

    QObject *const object = attached->parent();
    const QString &oldId = d->publisher->registeredObjectIds.value(object);

    if (!oldId.isEmpty()) {
        deregisterObject(object);
    }

    registerObject(newId, object);
}

QmlWebChannelAttached *QmlWebChannel::qmlAttachedProperties(QObject *obj)
{
    return new QmlWebChannelAttached(obj);
}

/*!
  Connectect to the given transport. Clients can then use it to communicate with the WebChannel.
  The transport object must implement the QMessagePassingInterface interface.
  Currently, only WebView.experimental and WebSocket implement this interface.

  \sa connections, disconnectFrom
  */

void QmlWebChannel::connectTo(QObject *transport)
{
    if (QMessagePassingInterface *iface = qobject_cast<QMessagePassingInterface*>(transport)) {
        m_connectedObjects.insert(transport, iface);
        QWebChannel::connectTo(iface);
        connect(transport, SIGNAL(destroyed(QObject*)), SLOT(transportDestroyed(QObject*)), Qt::UniqueConnection);
    } else {
        qWarning() << "Cannot connect to transport" << transport << " - it does not implement the QMessagePassingInterface.";
    }
}

/*!
  Disconnect the given transport. Clients which use it will not be able to communicate to the
  WebChannel anymore.
  */
void QmlWebChannel::disconnectFrom(QObject *transport)
{
    if (QMessagePassingInterface *iface = qobject_cast<QMessagePassingInterface*>(transport)) {
        QWebChannel::disconnectFrom(iface);
        disconnect(transport, SIGNAL(destroyed(QObject*)), this, SLOT(transportDestroyed(QObject*)));
        m_connectedObjects.remove(transport);
    } else {
        qWarning() << "Cannot disconnect from transport" << transport << " - it does not implement the QMessagePassingInterface.";
    }
}

void QmlWebChannel::transportDestroyed(QObject *transport)
{
    QMessagePassingInterface *iface = m_connectedObjects.take(transport);
    const int idx = d->transports.indexOf(iface);
    if (idx != -1) {
        d->transports.remove(idx);
    }
}

QQmlListProperty<QObject> QmlWebChannel::registeredObjects()
{
    return QQmlListProperty<QObject>(this, 0,
                                     registeredObjects_append,
                                     registeredObjects_count,
                                     registeredObjects_at,
                                     registeredObjects_clear);
}

void QmlWebChannel::registeredObjects_append(QQmlListProperty<QObject> *prop, QObject *object)
{
    const QmlWebChannelAttached *const attached = qobject_cast<QmlWebChannelAttached*>(
        qmlAttachedPropertiesObject<QmlWebChannel>(object, false /* don't create */));
    if (!attached) {
        const QQmlContext *const context = qmlContext(object);
        qWarning() << "Cannot register object" << context->nameForObject(object) << '(' << object << ") without attached WebChannel.id property. Did you forget to set it?";
        return;
    }
    QmlWebChannel *channel = static_cast<QmlWebChannel*>(prop->object);
    if (!attached->id().isEmpty()) {
        // TODO: warning in such cases?
        channel->registerObject(attached->id(), object);
    }
    channel->m_registeredObjects.append(object);
    connect(attached, SIGNAL(idChanged(QString)), channel, SLOT(objectIdChanged(QString)));
}

int QmlWebChannel::registeredObjects_count(QQmlListProperty<QObject> *prop)
{
    return static_cast<QmlWebChannel*>(prop->object)->m_registeredObjects.size();
}

QObject *QmlWebChannel::registeredObjects_at(QQmlListProperty<QObject> *prop, int index)
{
    return static_cast<QmlWebChannel*>(prop->object)->m_registeredObjects.at(index);
}

void QmlWebChannel::registeredObjects_clear(QQmlListProperty<QObject> *prop)
{
    QmlWebChannel *channel = static_cast<QmlWebChannel*>(prop->object);
    foreach (QObject *object, channel->m_registeredObjects) {
        channel->deregisterObject(object);
    }
    return channel->m_registeredObjects.clear();
}

QQmlListProperty<QObject> QmlWebChannel::transports()
{
    return QQmlListProperty<QObject>(this, 0,
                                                           transports_append,
                                                           transports_count,
                                                           transports_at,
                                                           transports_clear);
}

void QmlWebChannel::transports_append(QQmlListProperty<QObject> *prop, QObject *transport)
{
    QmlWebChannel *channel = static_cast<QmlWebChannel*>(prop->object);
    channel->connectTo(transport);
}

int QmlWebChannel::transports_count(QQmlListProperty<QObject> *prop)
{
    return static_cast<QmlWebChannel*>(prop->object)->d->transports.size();
}

QObject *QmlWebChannel::transports_at(QQmlListProperty<QObject> *prop, int index)
{
    QmlWebChannel *channel = static_cast<QmlWebChannel*>(prop->object);
    return dynamic_cast<QObject*>(channel->d->transports.at(index));
}

void QmlWebChannel::transports_clear(QQmlListProperty<QObject> *prop)
{
    QWebChannel *channel = static_cast<QWebChannel*>(prop->object);
    foreach (QMessagePassingInterface *transport, channel->d->transports) {
        channel->disconnectFrom(transport);
    }
    Q_ASSERT(channel->d->transports.isEmpty());
}
