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
    const QVariantMap info = publisher.classInfoForObject(&obj);

    QCOMPARE(info.keys(), QList<QString>() << "enums" << "methods" << "properties" << "signals");

    { // enums
        QVariantMap expected;
        QVariantMap fooEnum;
        fooEnum["Asdf"] = TestObject::Asdf;
        fooEnum["Bar"] = TestObject::Bar;
        expected["Foo"] = fooEnum;
        QCOMPARE(info["enums"].toMap(), expected);
    }

    { // methods & slots
        QVariantList expected;
        expected << QVariant::fromValue(QVariantList() << "deleteLater" << obj.metaObject()->indexOfMethod("deleteLater()"));
        expected << QVariant::fromValue(QVariantList() << "slot1" << obj.metaObject()->indexOfMethod("slot1()"));
        expected << QVariant::fromValue(QVariantList() << "slot2" << obj.metaObject()->indexOfMethod("slot2(QString)"));
        expected << QVariant::fromValue(QVariantList() << "method1" << obj.metaObject()->indexOfMethod("method1()"));
        QCOMPARE(info["methods"].toList(), expected);
    }

    { // signals
        QVariantList expected;
        expected << QVariant::fromValue(QVariantList() << "destroyed" << obj.metaObject()->indexOfMethod("destroyed(QObject*)"));
        expected << QVariant::fromValue(QVariantList() << "sig1" << obj.metaObject()->indexOfMethod("sig1()"));
        expected << QVariant::fromValue(QVariantList() << "sig2" << obj.metaObject()->indexOfMethod("sig2(QString)"));
        QCOMPARE(info["signals"].toList(), expected);
    }

    { // properties
        QVariantList expected;
        expected << QVariant::fromValue(QVariantList() << "objectName"
                                                       << QVariant::fromValue(QVariantList()
                                                            << 1 << obj.metaObject()->indexOfMethod("objectNameChanged(QString)"))
                                                       << obj.objectName());
        expected << QVariant::fromValue(QVariantList() << "foo"
                                                       << QVariant::fromValue(QVariantList())
                                                       << obj.foo());
        expected << QVariant::fromValue(QVariantList() << "asdf"
                                                       << QVariant::fromValue(QVariantList()
                                                            << 1 << obj.metaObject()->indexOfMethod("asdfChanged()"))
                                                       << obj.asdf());
        expected << QVariant::fromValue(QVariantList() << "bar"
                                                       << QVariant::fromValue(QVariantList()
                                                            << "theBarHasChanged" << obj.metaObject()->indexOfMethod("theBarHasChanged()"))
                                                       << obj.bar());
        QCOMPARE(info["properties"].toList(), expected);
    }
}

QTEST_MAIN(TestWebChannel)
