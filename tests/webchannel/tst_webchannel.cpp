/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Copyright (C) 2013 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com, author Milian Wolff <milian.wolff@kdab.com>
** Contact: http://www.qt-project.org/legal
*
** This file is part of the QtCore module of the Qt Toolkit.
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

#include "tst_webchannel.h"

#include <qwebchannel.h>
#include <qmetaobjectpublisher.h>
#include <qmetaobjectpublisher_p.h>

#include <QtTest>

TestWebChannel::TestWebChannel(QObject *parent)
    : QObject(parent)
{
}

TestWebChannel::~TestWebChannel()
{

}

    void TestWebChannel::testInitChannel()
{
    QWebChannel channel;

    QSignalSpy initSpy(&channel, SIGNAL(initialized()));
    QSignalSpy baseUrlSpy(&channel, SIGNAL(baseUrlChanged(QString)));

    QVERIFY(initSpy.wait());
    QCOMPARE(initSpy.size(), 1);
    QCOMPARE(baseUrlSpy.size(), 1);
    QCOMPARE(baseUrlSpy.first().size(), 1);
    QCOMPARE(channel.baseUrl(), baseUrlSpy.first().first().toString());
    QVERIFY(!channel.baseUrl().isEmpty());
}

void TestWebChannel::testRegisterObjects()
{
    QWebChannel channel;
    QMetaObjectPublisher publisher;
    publisher.setWebChannel(&channel);

    QObject plain;

    QVariantMap objects;
    objects["plain"] = QVariant::fromValue(&plain);
    objects["channel"] = QVariant::fromValue(&channel);
    objects["publisher"] = QVariant::fromValue(&publisher);
    objects["test"] = QVariant::fromValue(this);

    publisher.registerObjects(objects);
}

void TestWebChannel::testInfoForObject()
{
    TestObject obj;
    obj.setObjectName("myTestObject");
    QMetaObjectPublisher publisher;
    const QJsonObject info = publisher.classInfoForObject(&obj);

    QCOMPARE(info.keys(), QStringList() << "enums" << "methods" << "properties" << "signals");

    { // enums
        QJsonObject fooEnum;
        fooEnum["Asdf"] = TestObject::Asdf;
        fooEnum["Bar"] = TestObject::Bar;
        QJsonObject expected;
        expected["Foo"] = fooEnum;
        QCOMPARE(info["enums"].toObject(), expected);
    }

    { // methods & slots
        QJsonArray expected;
        {
            QJsonArray method;
            method.append(QStringLiteral("deleteLater"));
            method.append(obj.metaObject()->indexOfMethod("deleteLater()"));
            expected.append(method);
        }
        {
            QJsonArray method;
            method.append(QStringLiteral("slot1"));
            method.append(obj.metaObject()->indexOfMethod("slot1()"));
            expected.append(method);
        }
        {
            QJsonArray method;
            method.append(QStringLiteral("slot2"));
            method.append(obj.metaObject()->indexOfMethod("slot2(QString)"));
            expected.append(method);
        }
        {
            QJsonArray method;
            method.append(QStringLiteral("method1"));
            method.append(obj.metaObject()->indexOfMethod("method1()"));
            expected.append(method);
        }
        QCOMPARE(info["methods"].toArray(), expected);
    }

    { // signals
        QJsonArray expected;
        {
            QJsonArray signal;
            signal.append(QStringLiteral("destroyed"));
            signal.append(obj.metaObject()->indexOfMethod("destroyed(QObject*)"));
            expected.append(signal);
        }
        {
            QJsonArray signal;
            signal.append(QStringLiteral("sig1"));
            signal.append(obj.metaObject()->indexOfMethod("sig1()"));
            expected.append(signal);
        }
        {
            QJsonArray signal;
            signal.append(QStringLiteral("sig2"));
            signal.append(obj.metaObject()->indexOfMethod("sig2(QString)"));
            expected.append(signal);
        }
        QCOMPARE(info["signals"].toArray(), expected);
    }

    { // properties
        QJsonArray expected;
        {
            QJsonArray property;
            property.append(obj.metaObject()->indexOfProperty("objectName"));
            property.append(QStringLiteral("objectName"));
            {
                QJsonArray signal;
                signal.append(1);
                signal.append(obj.metaObject()->indexOfMethod("objectNameChanged(QString)"));
                property.append(signal);
            }
            property.append(obj.objectName());
            expected.append(property);
        }
        {
            QJsonArray property;
            property.append(obj.metaObject()->indexOfProperty("foo"));
            property.append(QStringLiteral("foo"));
            {
                QJsonArray signal;
                property.append(signal);
            }
            property.append(obj.foo());
            expected.append(property);
        }
        {
            QJsonArray property;
            property.append(obj.metaObject()->indexOfProperty("asdf"));
            property.append(QStringLiteral("asdf"));
            {
                QJsonArray signal;
                signal.append(1);
                signal.append(obj.metaObject()->indexOfMethod("asdfChanged()"));
                property.append(signal);
            }
            property.append(obj.asdf());
            expected.append(property);
        }
        {
            QJsonArray property;
            property.append(obj.metaObject()->indexOfProperty("bar"));
            property.append(QStringLiteral("bar"));
            {
                QJsonArray signal;
                signal.append(QStringLiteral("theBarHasChanged"));
                signal.append(obj.metaObject()->indexOfMethod("theBarHasChanged()"));
                property.append(signal);
            }
            property.append(obj.bar());
            expected.append(property);
        }
        QCOMPARE(info["properties"].toArray(), expected);
    }
}

static QVariantMap createObjects(QObject *parent)
{
    const int num = 100;
    QVariantMap objects;
    for (int i = 0; i < num; ++i) {
        objects[QStringLiteral("obj%1").arg(i)] = QVariant::fromValue(new BenchObject(parent));
    }
    return objects;
}

void TestWebChannel::benchClassInfo()
{
    QWebChannel channel;
    QSignalSpy initSpy(&channel, SIGNAL(initialized()));
    QVERIFY(initSpy.wait());

    QMetaObjectPublisher publisher;
    publisher.setWebChannel(&channel);

    QObject parent;
    const QVariantMap objects = createObjects(&parent);

    QBENCHMARK {
        publisher.classInfoForObjects(objects);
    }
}

void TestWebChannel::benchInitializeClients()
{
    QWebChannel channel;
    QSignalSpy initSpy(&channel, SIGNAL(initialized()));
    QVERIFY(initSpy.wait());

    QMetaObjectPublisher publisher;
    publisher.setWebChannel(&channel);

    QObject parent;
    const QVariantMap objects = createObjects(&parent);
    publisher.registerObjects(objects);

    QBENCHMARK {
        publisher.d->initializeClients();

        publisher.d->propertyUpdatesInitialized = false;
        publisher.d->signalToPropertyMap.clear();
        publisher.d->signalHandler.clear();
    }
}

void TestWebChannel::benchPropertyUpdates()
{
    QWebChannel channel;
    QSignalSpy initSpy(&channel, SIGNAL(initialized()));
    QVERIFY(initSpy.wait());

    QMetaObjectPublisher publisher;
    publisher.setWebChannel(&channel);

    QObject parent;
    const QVariantMap objects = createObjects(&parent);
    QVector<BenchObject*> objectList;
    foreach (const QVariant &var, objects) {
        objectList << var.value<BenchObject*>();
    }

    publisher.registerObjects(objects);
    publisher.d->initializeClients();

    QBENCHMARK {
        foreach (BenchObject *obj, objectList) {
            obj->change();
        }

        publisher.d->clientIsIdle = true;
        publisher.d->sendPendingPropertyUpdates();
    }
}

void TestWebChannel::benchRegisterObjects()
{
    QWebChannel channel;
    QSignalSpy initSpy(&channel, SIGNAL(initialized()));
    QVERIFY(initSpy.wait());

    QMetaObjectPublisher publisher;
    publisher.setWebChannel(&channel);

    QObject parent;
    const QVariantMap objects = createObjects(&parent);

    QBENCHMARK {
        publisher.registerObjects(objects);
    }
}

QTEST_MAIN(TestWebChannel)
