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
#include "qmessagepassinginterface.h"

#include <QJsonDocument>
#include <QJsonObject>

QT_BEGIN_NAMESPACE

/*!
    \class QWebChannel

    \inmodule QtWebChannel
    \brief Implements a channel to expose QObjects to remote HTML clients.

    The QWebChannel fills the gap between C++ applications and HTML/JavaScript
    applications. By publishing a QObject derived object to a QWebChannel and
    using \l{qtwebchannel-javascript.html}{\c qwebchannel.js} on the HTML side, one can transparently access
    properties and public slots and methods of the QObject. No manual message
    passing and serialization of data is required, property updates and signal emission
    on the C++ side get automatically transmitted to the potentially remotely running HTML clients.
    There a JavaScript object for any published C++ QObject is available with an intuitive API
    mirrored after the C++ object's API.

    The C++ QWebChannel API makes it possible to talk to any HTML client, which could run on a local
    or even remote machine. The only limitation is that the HTML client supports the JavaScript
    features used by \l{qtwebchannel-javascript.html}{\c qwebchannel.js}. As such, one can interact with basically any modern HTML browser.

    In order to access QObjects or QML items from a HTML page running in a WebView, you can use the
    declarative WebChannel API.

    \sa {Standalone Example}
*/

/*!
    \property QWebChannel::blockUpdates

    Set to true to block updates. Then remote clients will not be notified about property changes.
    The changes are recorded and sent to the clients once setBlockUpdates(false) is called.
*/

/*!
    \internal

    Convert JSON data into a stringified message, which can be send to remote clients.
*/
QByteArray generateJSONMessage(const QJsonValue &id, const QJsonValue &data, bool response)
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
    return doc.toJson(QJsonDocument::Compact);
}

/*!
    Constructs a QWebChannel with the given \a parent.

    Note that a QWebChannel is only fully operational once you connect it to a
    QMessagePassingInterface. The HTML clients also need to be setup appropriately
    using \l{qtwebchannel-javascript.html}{\c qwebchannel.js}.

    \sa {Standalone Example}
*/
QWebChannel::QWebChannel(QObject *parent)
: QObject(parent)
, d(new QWebChannelPrivate)
{
    d->publisher = new QMetaObjectPublisher(this);
    connect(d->publisher, SIGNAL(blockUpdatesChanged(bool)),
            SIGNAL(blockUpdatesChanged(bool)));
}

/*!
    Destroys the QWebChannel.
*/
QWebChannel::~QWebChannel()
{
}

/*!
    Register a group of objects to the QWebChannel.

    The properties, signals and public methods of the objects are published to the remote clients.
    There, an object with the identifier used as key in the \a objects map is then constructed.

    \note A current limitation is that objects must be registered before any client is initialized.

    \sa QWebChannel::registerObject(), QWebChannel::deregisterObject()
*/
void QWebChannel::registerObjects(const QHash< QString, QObject * > &objects)
{
    const QHash<QString, QObject *>::const_iterator end = objects.constEnd();
    for (QHash<QString, QObject *>::const_iterator it = objects.constBegin(); it != end; ++it) {
        d->publisher->registerObject(it.key(), it.value());
    }
}

/*!
    Register a single object to the QWebChannel.

    The properties, signals and public methods of the \a object are published to the remote clients.
    There, an object with the identifier \a id is then constructed.

    \note A current limitation is that objects must be registered before any client is initialized.

    \sa QWebChannel::registerObjects(), QWebChannel::deregisterObject()
*/
void QWebChannel::registerObject(const QString &id, QObject *object)
{
    d->publisher->registerObject(id, object);
}

/*!
    Deregister the given \a object from the QWebChannel.

    Remote clients will receive a \c destroyed signal for the object.

    \sa QWebChannel::registerObjects(), QWebChannel::registerObject()
*/
void QWebChannel::deregisterObject(QObject *object)
{
    // handling of deregistration is analogously to handling of a destroyed signal
    d->publisher->signalEmitted(object, s_destroyedSignalIndex, QVariantList() << QVariant::fromValue(object));
}

/*!
    Returns true when updates are blocked and false otherwise.

    \sa QWebChannel::setBlockUpdates()
*/
bool QWebChannel::blockUpdates() const
{
    return d->publisher->blockUpdates;
}

/*!
    Set whether updates should be blocked or not.

    When updates are blocked, the remote clients will not be notified about property changes.
    The changes are recorded and sent to the clients once setBlockUpdates(false) is called.

    \sa QWebChannel::blockUpdates()
*/
void QWebChannel::setBlockUpdates(bool block)
{
    d->publisher->setBlockUpdates(block);
}

/*!
    Connects the QWebChannel to the \a transport object which implements the QMessagePassingInterface.

    The transport object then handles the communication between the C++ application and the remote
    HTML clients. Currently, only QWebSocket implements this feature for pure C++ applications.

    \sa QWebChannel::disconnectFrom(), {Standalone Example}
*/
void QWebChannel::connectTo(QMessagePassingInterface *transport)
{
    Q_ASSERT(transport);
    if (!d->transports.contains(transport)) {
        d->transports << transport;
        connect(dynamic_cast<QObject*>(transport),
                SIGNAL(textMessageReceived(QString)), d->publisher, SLOT(handleMessage(QString)));
    }
}

/*!
    Disconnects the QWebChannel from the \a transport object.

    \sa QWebChannel::connectTo()
*/
void QWebChannel::disconnectFrom(QMessagePassingInterface *transport)
{
    const int idx = d->transports.indexOf(transport);
    if (idx != -1) {
        disconnect(dynamic_cast<QObject*>(transport), 0, this, 0);
        d->transports.remove(idx);
    }
}

/*!
    Sends a message with the given \a data to all HTML clients connected to the QWebChannel.

    On the client side, the callbacks registered to the message \a id are invoked.

    You can register such a callback via
    \code
webChannel.subscribe(id, function(data) {
    // handle data
}); \endcode

    Note how on both sides JSON data is used and no manual serialization is required.
*/
void QWebChannel::sendMessage(const QJsonValue &id, const QJsonValue &data) const
{
    if (d->transports.isEmpty()) {
        qWarning("QWebChannel is not connected to any transports, cannot send messages.");
        return;
    }

    const QByteArray &message = generateJSONMessage(id, data, false);
    const QString &messageText = QString::fromUtf8(message);
    foreach (QMessagePassingInterface *transport, d->transports) {
        transport->sendTextMessage(messageText);
    }
}

QT_END_NAMESPACE
