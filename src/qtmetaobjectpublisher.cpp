/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** Copyright (C) 2013 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com, author Milian Wolff <milian.wolff@kdab.com>
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QWebChannel module on Qt labs.
**
** $QT_BEGIN_LICENSE:LGPL$
** No Commercial Usage
** This file contains pre-release code and may not be distributed.
** You may use this file in accordance with the terms and conditions
** contained in the Technology Preview License Agreement accompanying
** this package.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights.  These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** If you have questions regarding the use of this file, please contact
** Nokia at qt-info@nokia.com.
**
**
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qtmetaobjectpublisher.h"

#include <QStringList>
#include <QMetaObject>
#include <QMetaProperty>
#include <QDebug>

static const QString KEY_SIGNALS = QStringLiteral("signals");
static const QString KEY_METHODS = QStringLiteral("methods");
static const QString KEY_PROPERTIES = QStringLiteral("properties");
static const QString KEY_ENUMS = QStringLiteral("enums");

static const QString KEY_QOBJECT = QStringLiteral("__QObject*__");
static const QString KEY_ID = QStringLiteral("id");
static const QString KEY_DATA = QStringLiteral("data");

QtMetaObjectPublisher::QtMetaObjectPublisher(QQuickItem *parent)
    : QQuickItem(parent)
{
}

QVariantMap QtMetaObjectPublisher::classInfoForObjects(const QVariantMap &objectMap) const
{
    QVariantMap ret;
    QMap<QString, QVariant>::const_iterator it = objectMap.constBegin();
    while (it != objectMap.constEnd()) {
        QObject* object = it.value().value<QObject*>();
        if (object) {
            const QVariantMap &info = classInfoForObject(object);
            if (!info.isEmpty()) {
                ret[it.key()] = info;
            }
        }
        ++it;
    }
    return ret;
}

QVariantMap QtMetaObjectPublisher::classInfoForObject(QObject *object) const
{
    QVariantMap data;
    if (!object) {
        qWarning("null object given to MetaObjectPublisher - bad API usage?");
        return data;
    }
    QVariantList qtSignals, qtMethods;
    QVariantList qtProperties;
    QVariantMap qtEnums;
    const QMetaObject* metaObject = object->metaObject();
    QSet<int> notifySignals;
    QSet<QString> properties;
    for (int i = 0; i < metaObject->propertyCount(); ++i) {
        const QMetaProperty &prop = metaObject->property(i);
        QVariantList propertyInfo;
        const QString &propertyName = QString::fromLatin1(prop.name());
        propertyInfo.append(propertyName);
        properties << propertyName;
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
                propertyInfo.append(1);
            } else {
                propertyInfo.append(QString::fromLatin1(notifySignal));
            }
        } else {
            if (!prop.isConstant()) {
                qWarning("Property '%s'' of object '%s' has no notify signal and is not constant, "
                         "value updates in HTML will be broken!",
                         prop.name(), object->metaObject()->className());
            }
            propertyInfo.append(0);
        }
        propertyInfo.append(prop.read(object));
        qtProperties.append(QVariant::fromValue(propertyInfo));
    }
    for (int i = 0; i < metaObject->methodCount(); ++i) {
        if (notifySignals.contains(i)) {
            continue;
        }
        const QMetaMethod &method = metaObject->method(i);
        //NOTE: This will not work for overloaded methods/signals.
        //NOTE: this must be a string, otherwise it will be converted to '{}' in QML
        const QString &name = QString::fromLatin1(method.name());
        if (properties.contains(name)) {
            // optimize: Don't send the getter method, it gets overwritten by the
            // property on the client side anyways.
            continue;
        }
        if (method.methodType() == QMetaMethod::Signal)
            qtSignals << name;
        else if (method.access() == QMetaMethod::Public)
            qtMethods << name;
    }
    for (int i = 0; i < metaObject->enumeratorCount(); ++i) {
        QMetaEnum enumerator = metaObject->enumerator(i);
        QVariantMap values;
        for (int k = 0; k < enumerator.keyCount(); ++k) {
            values[enumerator.key(k)] = enumerator.value(k);
        }
        qtEnums[enumerator.name()] = values;
    }
    data[KEY_SIGNALS] = qtSignals;
    data[KEY_METHODS] = qtMethods;
    data[KEY_PROPERTIES] = QVariant::fromValue(qtProperties);
    data[KEY_ENUMS] = qtEnums;
    return data;
}

static QString objectId(QObject *object)
{
    return QString::number(quintptr(object), 16);
}

QVariant QtMetaObjectPublisher::wrapObject(QObject *object)
{
    if (!object)
        return QVariant();

    const QString& id = objectId(object);

    const WrapMapCIt& p = m_wrappedObjects.constFind(id);
    if (p != m_wrappedObjects.constEnd())
        return p.value().second;

    QVariantMap objectInfo;
    objectInfo[KEY_QOBJECT] = true;
    objectInfo[KEY_ID] = id;
    objectInfo[KEY_DATA] = classInfoForObject(object);

    m_wrappedObjects.insert(id, WrapInfo(object, objectInfo));
    connect(object, SIGNAL(destroyed(QObject*)), SLOT(wrappedObjectDestroyed(QObject*)));

    return objectInfo;
}

QObject *QtMetaObjectPublisher::unwrapObject(const QString& id) const
{
    const WrapMapCIt& p = m_wrappedObjects.constFind(id);
    if (p != m_wrappedObjects.constEnd())
        return p.value().first;
    return 0;
}

void QtMetaObjectPublisher::wrappedObjectDestroyed(QObject* object)
{
    const QString& id = objectId(object);
    m_wrappedObjects.remove(id);
    emit wrappedObjectDestroyed(id);
}

void QtMetaObjectPublisher::deleteWrappedObject(QObject* object) const
{
    if (!m_wrappedObjects.contains(objectId(object))) {
        qWarning() << "Not deleting non-wrapped object" << object;
        return;
    }
    object->deleteLater();
}
