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

#include "qmetaobjectpublisher_p.h"
#include "qwebchannel.h"

#include <QEvent>
#include <QJsonDocument>
#include <QDebug>
#include <QJsonObject>
#include <QJsonArray>

#if HAVE_QML
#include <QtQml/QJSValue>
#include <QtQml/QJSEngine>
#endif

QT_BEGIN_NAMESPACE

namespace {

// NOTE: keep in sync with corresponding maps in qwebchannel.js and WebChannelTest.qml
enum Type {
    TypeInvalid = 0,

    TYPES_FIRST_VALUE = 1,

    TypeSignal = 1,
    TypePropertyUpdate = 2,
    TypeInit = 3,
    TypeIdle = 4,
    TypeDebug = 5,
    TypeInvokeMethod = 6,
    TypeConnectToSignal = 7,
    TypeDisconnectFromSignal = 8,
    TypeSetProperty = 9,

    TYPES_LAST_VALUE = 9
};

Type toType(const QJsonValue &value)
{
    int i = value.toInt(-1);
    if (i >= TYPES_FIRST_VALUE && i <= TYPES_LAST_VALUE) {
        return static_cast<Type>(i);
    } else {
        return TypeInvalid;
    }
}

const QString KEY_SIGNALS = QStringLiteral("signals");
const QString KEY_METHODS = QStringLiteral("methods");
const QString KEY_PROPERTIES = QStringLiteral("properties");
const QString KEY_ENUMS = QStringLiteral("enums");
const QString KEY_QOBJECT = QStringLiteral("__QObject*__");
const QString KEY_ID = QStringLiteral("id");
const QString KEY_DATA = QStringLiteral("data");
const QString KEY_OBJECT = QStringLiteral("object");
const QString KEY_DESTROYED = QStringLiteral("destroyed");
const QString KEY_SIGNAL = QStringLiteral("signal");
const QString KEY_TYPE = QStringLiteral("type");
const QString KEY_MESSAGE = QStringLiteral("message");
const QString KEY_METHOD = QStringLiteral("method");
const QString KEY_ARGS = QStringLiteral("args");
const QString KEY_PROPERTY = QStringLiteral("property");
const QString KEY_VALUE = QStringLiteral("value");


QString objectId(const QObject *object)
{
    return QString::number(quintptr(object), 16);
}

/// TODO: what is the proper value here?
const int PROPERTY_UPDATE_INTERVAL = 50;
}

QMetaObjectPublisher::QMetaObjectPublisher(QWebChannel *webChannel)
    : QObject(webChannel)
    , webChannel(webChannel)
    , signalHandler(this)
    , clientIsIdle(false)
    , blockUpdates(false)
    , pendingInit(false)
    , propertyUpdatesInitialized(false)
{
}

QMetaObjectPublisher::~QMetaObjectPublisher()
{

}

void QMetaObjectPublisher::registerObject(const QString &id, QObject *object)
{
    if (propertyUpdatesInitialized) {
        qWarning("Registered new object after initialization. This does not work!");
        return;
    }
    registeredObjects[id] = object;
    registeredObjectIds[object] = id;
}

QJsonObject QMetaObjectPublisher::classInfoForObject(const QObject *object) const
{
    QJsonObject data;
    if (!object) {
        qWarning("null object given to MetaObjectPublisher - bad API usage?");
        return data;
    }

    QJsonArray qtSignals;
    QJsonArray qtMethods;
    QJsonArray qtProperties;
    QJsonObject qtEnums;

    const QMetaObject *metaObject = object->metaObject();
    QSet<int> notifySignals;
    QSet<QString> identifiers;
    for (int i = 0; i < metaObject->propertyCount(); ++i) {
        const QMetaProperty &prop = metaObject->property(i);
        QJsonArray propertyInfo;
        const QString &propertyName = QString::fromLatin1(prop.name());
        propertyInfo.append(i);
        propertyInfo.append(propertyName);
        identifiers << propertyName;
        QJsonArray signalInfo;
        if (prop.hasNotifySignal()) {
            notifySignals << prop.notifySignalIndex();
            const int numParams = prop.notifySignal().parameterCount();
            if (numParams > 1) {
                qWarning("Notify signal for property '%s' has %d parameters, expected zero or one.",
                         prop.name(), numParams);
            }
            // optimize: compress the common propertyChanged notification names, just send a 1
            const QByteArray &notifySignal = prop.notifySignal().name();
            static const QByteArray changedSuffix = QByteArrayLiteral("Changed");
            if (notifySignal.length() == changedSuffix.length() + propertyName.length() &&
                notifySignal.endsWith(changedSuffix) && notifySignal.startsWith(prop.name()))
            {
                signalInfo.append(1);
            } else {
                signalInfo.append(QString::fromLatin1(notifySignal));
            }
            signalInfo.append(prop.notifySignalIndex());
        } else if (!prop.isConstant()) {
            qWarning("Property '%s'' of object '%s' has no notify signal and is not constant, "
                     "value updates in HTML will be broken!",
                     prop.name(), object->metaObject()->className());
        }
        propertyInfo.append(signalInfo);
        propertyInfo.append(QJsonValue::fromVariant(prop.read(object)));
        qtProperties.append(propertyInfo);
    }
    for (int i = 0; i < metaObject->methodCount(); ++i) {
        if (notifySignals.contains(i)) {
            continue;
        }
        const QMetaMethod &method = metaObject->method(i);
        //NOTE: this must be a string, otherwise it will be converted to '{}' in QML
        const QString &name = QString::fromLatin1(method.name());
        // optimize: skip overloaded methods/signals or property getters, on the JS side we can only
        // call one of them anyways
        // TODO: basic support for overloaded signals, methods
        if (identifiers.contains(name)) {
            continue;
        }
        identifiers << name;
        // send data as array to client with format: [name, index]
        QJsonArray data;
        data.append(name);
        data.append(i);
        if (method.methodType() == QMetaMethod::Signal) {
            qtSignals.append(data);
        } else if (method.access() == QMetaMethod::Public) {
            qtMethods.append(data);
        }
    }
    for (int i = 0; i < metaObject->enumeratorCount(); ++i) {
        QMetaEnum enumerator = metaObject->enumerator(i);
        QJsonObject values;
        for (int k = 0; k < enumerator.keyCount(); ++k) {
            values[QString::fromLatin1(enumerator.key(k))] = enumerator.value(k);
        }
        qtEnums[QString::fromLatin1(enumerator.name())] = values;
    }
    data[KEY_SIGNALS] = qtSignals;
    data[KEY_METHODS] = qtMethods;
    data[KEY_PROPERTIES] = qtProperties;
    data[KEY_ENUMS] = qtEnums;
    return data;
}

void QMetaObjectPublisher::setClientIsIdle(bool isIdle)
{
    if (clientIsIdle == isIdle) {
        return;
    }
    clientIsIdle = isIdle;
    if (!isIdle && timer.isActive()) {
        timer.stop();
    } else if (isIdle && !timer.isActive()) {
        timer.start(PROPERTY_UPDATE_INTERVAL, this);
    }
}

void QMetaObjectPublisher::initializeClients()
{
    if (!webChannel) {
        return;
    }

    QJsonObject objectInfos;
    {
        const QHash<QString, QObject *>::const_iterator end = registeredObjects.constEnd();
        for (QHash<QString, QObject *>::const_iterator it = registeredObjects.constBegin(); it != end; ++it) {
            const QJsonObject &info = classInfoForObject(it.value());
            if (!propertyUpdatesInitialized) {
                initializePropertyUpdates(it.value(), info);
            }
            objectInfos[it.key()] = info;
        }
    }
    webChannel->sendMessage(TypeInit, objectInfos);
    propertyUpdatesInitialized = true;
    pendingInit = false;
}

void QMetaObjectPublisher::initializePropertyUpdates(const QObject *const object, const QJsonObject &objectInfo)
{
    foreach (const QJsonValue &propertyInfoVar, objectInfo[KEY_PROPERTIES].toArray()) {
        const QJsonArray &propertyInfo = propertyInfoVar.toArray();
        if (propertyInfo.size() < 2) {
            qWarning() << "Invalid property info encountered:" << propertyInfoVar;
            continue;
        }
        const int propertyIndex = propertyInfo.at(0).toInt();
        const QJsonArray &signalData = propertyInfo.at(2).toArray();

        if (signalData.isEmpty()) {
            // Property without NOTIFY signal
            continue;
        }

        const int signalIndex = signalData.at(1).toInt();

        QSet<int> &connectedProperties = signalToPropertyMap[object][signalIndex];

        // Only connect for a property update once
        if (connectedProperties.isEmpty()) {
            signalHandler.connectTo(object, signalIndex);
        }

        connectedProperties.insert(propertyIndex);
    }

    // also always connect to destroyed signal
    signalHandler.connectTo(object, s_destroyedSignalIndex);
}

void QMetaObjectPublisher::sendPendingPropertyUpdates()
{
    if (blockUpdates || !clientIsIdle || pendingPropertyUpdates.isEmpty()) {
        return;
    }

    QJsonArray data;

    // convert pending property updates to JSON data
    const PendingPropertyUpdates::const_iterator end = pendingPropertyUpdates.constEnd();
    for (PendingPropertyUpdates::const_iterator it = pendingPropertyUpdates.constBegin(); it != end; ++it) {
        const QObject *object = it.key();
        const QMetaObject *const metaObject = object->metaObject();
        const SignalToPropertyNameMap &objectsSignalToPropertyMap = signalToPropertyMap.value(object);
        // maps property name to current property value
        QJsonObject properties;
        // maps signal index to list of arguments of the last emit
        QJsonObject sigs;
        const SignalToArgumentsMap::const_iterator sigEnd = it.value().constEnd();
        for (SignalToArgumentsMap::const_iterator sigIt = it.value().constBegin(); sigIt != sigEnd; ++sigIt) {
            // TODO: can we get rid of the int <-> string conversions here?
            foreach (const int propertyIndex, objectsSignalToPropertyMap.value(sigIt.key())) {
                const QMetaProperty &property = metaObject->property(propertyIndex);
                Q_ASSERT(property.isValid());
                properties[QString::number(propertyIndex)] = QJsonValue::fromVariant(property.read(object));
            }
            sigs[QString::number(sigIt.key())] = QJsonArray::fromVariantList(sigIt.value());
        }
        QJsonObject obj;
        obj[KEY_OBJECT] = registeredObjectIds.value(object);
        obj[KEY_SIGNALS] = sigs;
        obj[KEY_PROPERTIES] = properties;
        data.push_back(obj);
    }

    pendingPropertyUpdates.clear();
    webChannel->sendMessage(TypePropertyUpdate, data);
    setClientIsIdle(false);
}

bool QMetaObjectPublisher::invokeMethod(QObject *const object, const int methodIndex,
                                                const QJsonArray &args, const QJsonValue &id)
{
    const QMetaMethod &method = object->metaObject()->method(methodIndex);

    if (method.name() == QByteArrayLiteral("deleteLater")) {
        // invoke `deleteLater` on wrapped QObject indirectly
        deleteWrappedObject(object);
        return true;
    } else if (!method.isValid()) {
        qWarning() << "Cannot invoke unknown method of index" << methodIndex << "on object" << object << '.';
        return false;
    } else if (method.access() != QMetaMethod::Public) {
        qWarning() << "Cannot invoke non-public method" << method.name() << "on object" << object << '.';
        return false;
    } else if (method.methodType() != QMetaMethod::Method && method.methodType() != QMetaMethod::Slot) {
        qWarning() << "Cannot invoke non-public method" << method.name() << "on object" << object << '.';
        return false;
    } else if (args.size() > 10) {
        qWarning() << "Cannot invoke method" << method.name() << "on object" << object << "with more than 10 arguments, as that is not supported by QMetaMethod::invoke.";
        return false;
    } else if (args.size() > method.parameterCount()) {
        qWarning() << "Ignoring additional arguments while invoking method" << method.name() << "on object" << object << ':'
                   << args.size() << "arguments given, but method only takes" << method.parameterCount() << '.';
    }

    // construct converter objects of QVariant to QGenericArgument
    VariantArgument arguments[10];
    for (int i = 0; i < qMin(args.size(), method.parameterCount()); ++i) {
        QVariant arg = args.at(i).toVariant();
        if (method.parameterType(i) != QMetaType::QVariant && !arg.convert(method.parameterType(i))) {
            qWarning() << "Could not convert argument" << args.at(i) << "to target type" << method.parameterTypes().at(i) << '.';
        }
        arguments[i].value = arg;
    }

    // construct QGenericReturnArgument
    QVariant returnValue;
    if (method.returnType() != qMetaTypeId<QVariant>() && method.returnType() != qMetaTypeId<void>()) {
        // Only init variant with return type if its not a variant itself, which would
        // lead to nested variants which is not what we want.
        // Also, skip void-return types for obvious reasons (and to prevent a runtime warning inside Qt).
        returnValue = QVariant(method.returnType(), 0);
    }
    QGenericReturnArgument returnArgument(method.typeName(), returnValue.data());

    // now we can call the method
    method.invoke(object, returnArgument,
                  arguments[0], arguments[1], arguments[2], arguments[3], arguments[4],
                  arguments[5], arguments[6], arguments[7], arguments[8], arguments[9]);

    // and send the return value to the client
    webChannel->respond(id, wrapResult(returnValue));

    return true;
}

void QMetaObjectPublisher::signalEmitted(const QObject *object, const int signalIndex, const QVariantList &arguments)
{
    if (!webChannel) {
        return;
    }
    if (!signalToPropertyMap.value(object).contains(signalIndex)) {
        QJsonObject data;
        const QString &objectName = registeredObjectIds.value(object);
        Q_ASSERT(!objectName.isEmpty());
        data[KEY_OBJECT] = objectName;
        data[KEY_SIGNAL] = signalIndex;
        if (!arguments.isEmpty()) {
            // TODO: wrap (new) objects on the fly
            QJsonArray args;
#if HAVE_QML
            foreach (const QVariant &arg, arguments) {
                if (arg.canConvert<QJSValue>()) {
                    const QJSValue &jsValue = arg.value<QJSValue>();
                    args.append(qjsvalue_cast<QJsonValue>(jsValue));
                } else {
                    args.append(QJsonValue::fromVariant(arg));
                }
            }
#else
            args = QJsonArray::fromVariantList(arguments);
#endif
            data[KEY_ARGS] = args;
        }
        webChannel->sendMessage(TypeSignal, data);

        if (signalIndex == s_destroyedSignalIndex) {
            objectDestroyed(object);
        }
    } else {
        pendingPropertyUpdates[object][signalIndex] = arguments;
        if (clientIsIdle && !blockUpdates && !timer.isActive()) {
            timer.start(PROPERTY_UPDATE_INTERVAL, this);
        }
    }
}

void QMetaObjectPublisher::objectDestroyed(const QObject *object)
{
    const QString &id = registeredObjectIds.take(object);
    Q_ASSERT(!id.isEmpty());
    bool removed = registeredObjects.remove(id);
    Q_ASSERT(removed);
    Q_UNUSED(removed);

    signalToPropertyMap.remove(object);
    pendingPropertyUpdates.remove(object);
    wrappedObjects.remove(object);
}

QJsonValue QMetaObjectPublisher::wrapResult(const QVariant &result)
{
    if (QObject *object = result.value<QObject *>()) {
        QJsonObject &objectInfo = wrappedObjects[object];
        if (!objectInfo.isEmpty()) {
            // already registered, use cached information
            Q_ASSERT(registeredObjectIds.contains(object));
            return objectInfo;
        } // else the object is not yet wrapped, do it now

        const QString &id = objectId(object);
        Q_ASSERT(!registeredObjects.contains(id));
        Q_ASSERT(!registeredObjectIds.contains(object));

        objectInfo[KEY_QOBJECT] = true;
        objectInfo[KEY_ID] = id;
        objectInfo[KEY_DATA] = classInfoForObject(object);

        registeredObjectIds[object] = id;
        registeredObjects[id] = object;
        wrappedObjects.insert(object, objectInfo);

        initializePropertyUpdates(object, objectInfo);
        return objectInfo;
    }

    // no need to wrap this
    return QJsonValue::fromVariant(result);
}

void QMetaObjectPublisher::deleteWrappedObject(QObject *object) const
{
    if (!wrappedObjects.contains(object)) {
        qWarning() << "Not deleting non-wrapped object" << object;
        return;
    }
    object->deleteLater();
}

bool QMetaObjectPublisher::handleRequest(const QJsonObject &message)
{
    if (!message.contains(KEY_DATA)) {
        return false;
    }

    const QJsonObject &payload = message.value(KEY_DATA).toObject();
    if (!payload.contains(KEY_TYPE)) {
        return false;
    }

    const Type type = toType(payload.value(KEY_TYPE));
    if (type == TypeIdle) {
        setClientIsIdle(true);
        return true;
    } else if (type == TypeInit) {
        if (!blockUpdates) {
            initializeClients();
        } else {
            pendingInit = true;
        }
        return true;
    } else if (type == TypeDebug) {
        static QTextStream out(stdout);
        out << "DEBUG: " << payload.value(KEY_MESSAGE).toString() << endl;
        return true;
    } else if (payload.contains(KEY_OBJECT)) {
        const QString &objectName = payload.value(KEY_OBJECT).toString();
        QObject *object = registeredObjects.value(objectName);
        if (!object) {
            qWarning() << "Unknown object encountered" << objectName;
            return false;
        }

        if (type == TypeInvokeMethod) {
            return invokeMethod(object, payload.value(KEY_METHOD).toInt(-1), payload.value(KEY_ARGS).toArray(), message.value(KEY_ID));
        } else if (type == TypeConnectToSignal) {
            signalHandler.connectTo(object, payload.value(KEY_SIGNAL).toInt(-1));
            return true;
        } else if (type == TypeDisconnectFromSignal) {
            signalHandler.disconnectFrom(object, payload.value(KEY_SIGNAL).toInt(-1));
            return true;
        } else if (type == TypeSetProperty) {
            const int propertyIdx = payload.value(KEY_PROPERTY).toInt(-1);
            QMetaProperty property = object->metaObject()->property(propertyIdx);
            if (!property.isValid()) {
                qWarning() << "Cannot set unknown property" << payload.value(KEY_PROPERTY) << "of object" << objectName;
                return false;
            } else if (!object->metaObject()->property(propertyIdx).write(object, payload.value(KEY_VALUE).toVariant())) {
                qWarning() << "Could not write value " << payload.value(KEY_VALUE)
                           << "to property" << property.name() << "of object" << objectName;
                return false;
            }
            return true;
        }
    }
    return false;
}

void QMetaObjectPublisher::handleMessage(const QString &message)
{
    const QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
    if (doc.isObject()) {
        handleRequest(doc.object());
    }
}

void QMetaObjectPublisher::setBlockUpdates(bool block)
{
    if (blockUpdates == block) {
        return;
    }
    blockUpdates = block;

    if (!blockUpdates) {
        if (pendingInit) {
            initializeClients();
        } else {
            sendPendingPropertyUpdates();
        }
    } else if (timer.isActive()) {
        timer.stop();
    }

    emit blockUpdatesChanged(block);
}

void QMetaObjectPublisher::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == timer.timerId()) {
        sendPendingPropertyUpdates();
    } else {
        QObject::timerEvent(event);
    }
}

QT_END_NAMESPACE
