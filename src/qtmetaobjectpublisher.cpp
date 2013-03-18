/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
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

QtMetaObjectPublisher::QtMetaObjectPublisher(QObject *parent)
    : QObject(parent)
{
}

QVariantMap QtMetaObjectPublisher::classInfoForObject(QObject *object) const
{
    QVariantMap data;
    if (!object) {
        qWarning("null object given to MetaObjectPublisher - bad API usage?");
        return data;
    }
    QStringList qtSignals, qtMethods, qtProperties;
    QVariantMap qtEnums;
    const QMetaObject* metaObject = object->metaObject();
    for (int i = 0; i < metaObject->propertyCount(); ++i)
        qtProperties.append(metaObject->property(i).name());
    for (int i = 0; i < metaObject->methodCount(); ++i) {
        QMetaMethod method = metaObject->method(i);
        QString signature = method.methodSignature();
        QString name = signature.left(signature.indexOf("("));
        if (method.access() == QMetaMethod::Public)
            qtMethods << signature << name;
        if (method.methodType() == QMetaMethod::Signal)
            qtSignals << signature << name;
    }
    for (int i = 0; i < metaObject->enumeratorCount(); ++i) {
        QMetaEnum enumerator = metaObject->enumerator(i);
        QVariantMap values;
        for(int k = 0; k < enumerator.keyCount(); ++k) {
            values[enumerator.key(k)] = enumerator.value(k);
        }
        qtEnums[enumerator.name()] = values;
    }
    data["signals"] = qtSignals;
    data["methods"] = qtMethods;
    data["properties"] = qtProperties;
    data["enums"] = qtEnums;
    return data;
}
