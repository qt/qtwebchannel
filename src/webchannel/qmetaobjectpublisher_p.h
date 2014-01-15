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

#ifndef QMETAOBJECTPUBLISHER_P_H
#define QMETAOBJECTPUBLISHER_P_H

#include "variantargument_p.h"
#include "signalhandler_p.h"

#include <QStringList>
#include <QMetaObject>
#include <QBasicTimer>
#include <QPointer>

QT_BEGIN_NAMESPACE

class QWebChannel;

#include "qwebchannelglobal.h"

class Q_WEBCHANNEL_EXPORT QMetaObjectPublisher : public QObject
{
    Q_OBJECT

public:
    QMetaObjectPublisher(QWebChannel *webChannel);
    virtual ~QMetaObjectPublisher();

    /**
     * Register @p object nuder the given @p id.
     *
     * The properties, signals and public methods of the QObject are
     * published to the remote client, where an object with the given identifier
     * is constructed.
     *
     * TODO: This must be called, before clients are initialized.
     */
    void registerObject(const QString &id, QObject *object);

    /**
     * Handle the given WebChannel client request and potentially give a response.
     *
     * @return true if the request was handled, false otherwise.
     */
    bool handleRequest(const QJsonObject &message);

    /**
     * Serialize the QMetaObject of @p object and return it in JSON form.
     */
    QJsonObject classInfoForObject(const QObject *object) const;

    /**
     * Set the client to idle or busy, based on the value of @p isIdle.
     *
     * When the value changed, start/stop the property update timer accordingly.
     */
    void setClientIsIdle(bool isIdle);

    /**
     * Initialize clients by sending them the class information of the registered objects.
     *
     * Furthermore, if that was not done already, connect to their property notify signals.
     */
    void initializeClients();

    /**
     * Go through all properties of the given object and connect to their notify signal.
     *
     * When receiving a notify signal, it will store the information in pendingPropertyUpdates which
     * gets send via a Qt.propertyUpdate message to the server when the grouping timer timeouts.
     */
    void initializePropertyUpdates(const QObject *const object, const QJsonObject &objectInfo);

    /**
     * Send the clients the new property values since the last time this function was invoked.
     *
     * This is a grouped batch of all properties for which their notify signal was emitted.
     * The list of signals as well as the arguments they contained, are also transmitted to
     * the remote clients.
     *
     * @sa timer, initializePropertyUpdates
     */
    void sendPendingPropertyUpdates();

    /**
     * Invoke the method of index @p methodIndex on @p object with the arguments @p args.
     *
     * The return value of the method invocation is then transmitted to the calling client
     * via a webchannel response to the message identified by @p id.
     */
    bool invokeMethod(QObject *const object, const int methodIndex, const QJsonArray &args, const QJsonValue &id);

    /**
     * Callback of the signalHandler which forwards the signal invocation to the webchannel clients.
     */
    void signalEmitted(const QObject *object, const int signalIndex, const QVariantList &arguments);

    /**
     * Callback for registered or wrapped objects which erases all data related to @p object.
     *
     * @sa signalEmitted
     */
    void objectDestroyed(const QObject *object);

    /**
     * Given a QVariant containing a QObject*, wrap the object and register for property updates
     * return the objects class information.
     *
     * All other input types are returned as-is.
     *
     * TODO: support wrapping of initially-registered objects
     */
    QJsonValue wrapResult(const QVariant &result);

    /**
     * Invoke delete later on @p object.
     */
    void deleteWrappedObject(QObject *object) const;

    /**
     * When updates are blocked, no property updates are transmitted to remote clients.
     */
    void setBlockUpdates(bool block);

public slots:
    /**
     * Helper slot which you can connect directly to WebChannel's rawMessageReceived signal.
     *
     * This slot then tries to parse the message as JSON and if it succeeds, calls handleRequest
     * with the obtained JSON object.
     */
    void handleRawMessage(const QString &message);

signals:
    void blockUpdatesChanged(bool block);

protected:
    void timerEvent(QTimerEvent *) Q_DECL_OVERRIDE;

private:
    friend class QmlWebChannel;
    friend class QWebChannel;
    friend class TestWebChannel;

    QWebChannel *webChannel;
    SignalHandler<QMetaObjectPublisher> signalHandler;

    // true when the client is idle, false otherwise
    bool clientIsIdle;

    // true when no property updates should be sent, false otherwise
    bool blockUpdates;

    // true when at least one client needs to be initialized,
    // i.e. when a Qt.init came in which was not handled yet.
    bool pendingInit;

    // true when at least one client was initialized and thus
    // the property updates have been initialized and the
    // object info map set.
    bool propertyUpdatesInitialized;

    // Map of registered objects indexed by their id.
    QHash<QString, QObject *> registeredObjects;

    // Map the registered objects to their id.
    QHash<const QObject *, QString> registeredObjectIds;

    // Map of objects to maps of signal indices to a set of all their property indices.
    // The last value is a set as a signal can be the notify signal of multiple properties.
    typedef QHash<int, QSet<int> > SignalToPropertyNameMap;
    QHash<const QObject *, SignalToPropertyNameMap> signalToPropertyMap;

    // Objects that changed their properties and are waiting for idle client.
    // map of object name to map of signal index to arguments
    typedef QHash<int, QVariantList> SignalToArgumentsMap;
    typedef QHash<const QObject *, SignalToArgumentsMap> PendingPropertyUpdates;
    PendingPropertyUpdates pendingPropertyUpdates;

    // Maps wrapped object to class info
    QHash<const QObject *, QJsonObject> wrappedObjects;

    // Aggregate property updates since we get multiple Qt.idle message when we have multiple
    // clients. They all share the same QWebProcess though so we must take special care to
    // prevent message flooding.
    QBasicTimer timer;
};

QT_END_NAMESPACE

#endif // QMETAOBJECTPUBLISHER_P_H
