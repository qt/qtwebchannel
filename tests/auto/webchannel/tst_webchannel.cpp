/****************************************************************************
**
** Copyright (C) 2016 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com, author Milian Wolff <milian.wolff@kdab.com>
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWebChannel module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "tst_webchannel.h"

#include <qwebchannel.h>
#include <qwebchannel_p.h>
#include <qmetaobjectpublisher_p.h>

#include <QtTest>
#ifdef WEBCHANNEL_TESTS_CAN_USE_JS_ENGINE
#include <QJSEngine>
#endif

QT_USE_NAMESPACE

#ifdef WEBCHANNEL_TESTS_CAN_USE_JS_ENGINE
class TestJSEngine;

class TestEngineTransport : public QWebChannelAbstractTransport
{
    Q_OBJECT
public:
    TestEngineTransport(TestJSEngine *);
    void sendMessage(const QJsonObject &message) Q_DECL_OVERRIDE;

    Q_INVOKABLE void channelSetupReady();
    Q_INVOKABLE void send(const QByteArray &message);
private:
    TestJSEngine *m_testEngine;
};

class ConsoleLogger : public QObject
{
    Q_OBJECT
public:
    ConsoleLogger(QObject *parent = 0);

    Q_INVOKABLE void log(const QString &text);
    Q_INVOKABLE void error(const QString &text);

    int errorCount() const { return m_errCount; }
    int logCount() const { return m_logCount; }
    QString lastError() const { return m_lastError; }

private:
    int m_errCount;
    int m_logCount;
    QString m_lastError;

};



ConsoleLogger::ConsoleLogger(QObject *parent)
    : QObject(parent)
    , m_errCount(0)
    , m_logCount(0)
{
}

void ConsoleLogger::log(const QString &text)
{
    m_logCount++;
    qDebug("LOG: %s", qPrintable(text));
}

void ConsoleLogger::error(const QString &text)
{
    m_errCount++;
    m_lastError = text;
    qWarning("ERROR: %s", qPrintable(text));
}


// A test JS engine with convenience integration with WebChannel.
class TestJSEngine : public QJSEngine
{
    Q_OBJECT
public:
    TestJSEngine();

    TestEngineTransport *transport() const;
    ConsoleLogger *logger() const;
    void initWebChannelJS();

signals:
    void channelSetupReady(TestEngineTransport *transport);

private:
    TestEngineTransport *m_transport;
    ConsoleLogger *m_logger;
};

TestEngineTransport::TestEngineTransport(TestJSEngine *engine)
    : QWebChannelAbstractTransport(engine)
    , m_testEngine(engine)
{
}

void TestEngineTransport::sendMessage(const QJsonObject &message)
{
    QByteArray json = QJsonDocument(message).toJson(QJsonDocument::Compact);
    QJSValue callback = m_testEngine->evaluate(QStringLiteral("transport.onmessage"));
    Q_ASSERT(callback.isCallable());
    QJSValue arg = m_testEngine->newObject();
    QJSValue data = m_testEngine->evaluate(QString::fromLatin1("JSON.parse('%1');").arg(QString::fromUtf8(json)));
    Q_ASSERT(!data.isError());
    arg.setProperty(QStringLiteral("data"), data);
    QJSValue val = callback.call((QJSValueList() << arg));
    Q_ASSERT(!val.isError());
}

void TestEngineTransport::channelSetupReady()
{
    emit m_testEngine->channelSetupReady(m_testEngine->transport());
}

void TestEngineTransport::send(const QByteArray &message)
{
    QJsonDocument doc(QJsonDocument::fromJson(message));
    emit messageReceived(doc.object(), this);
}


TestJSEngine::TestJSEngine()
    : m_transport(new TestEngineTransport(this))
    , m_logger(new ConsoleLogger(this))
{
    globalObject().setProperty("transport", newQObject(m_transport));
    globalObject().setProperty("console", newQObject(m_logger));

    QString webChannelJSPath(QStringLiteral(":/qtwebchannel/qwebchannel.js"));
    QFile webChannelJS(webChannelJSPath);
    if (!webChannelJS.open(QFile::ReadOnly))
        qFatal("Error opening qwebchannel.js");
    QString source(QString::fromUtf8(webChannelJS.readAll()));
    evaluate(source, webChannelJSPath);
}

TestEngineTransport *TestJSEngine::transport() const
{
    return m_transport;
}

ConsoleLogger *TestJSEngine::logger() const
{
    return m_logger;
}

void TestJSEngine::initWebChannelJS()
{
    globalObject().setProperty(QStringLiteral("channel"), newObject());
    QJSValue channel = evaluate(QStringLiteral("channel = new QWebChannel(transport, function(channel) { transport.channelSetupReady();});"));
    Q_ASSERT(!channel.isError());
}

#endif // WEBCHANNEL_TESTS_CAN_USE_JS_ENGINE


TestWebChannel::TestWebChannel(QObject *parent)
    : QObject(parent)
    , m_dummyTransport(new DummyTransport(this))
    , m_lastInt(0)
    , m_lastDouble(0)
{
}

TestWebChannel::~TestWebChannel()
{

}

void TestWebChannel::setInt(int i)
{
    m_lastInt = i;
}

void TestWebChannel::setDouble(double d)
{
    m_lastDouble = d;
}

void TestWebChannel::setVariant(const QVariant &v)
{
    m_lastVariant = v;
}

void TestWebChannel::testRegisterObjects()
{
    QWebChannel channel;
    QObject plain;

    QHash<QString, QObject*> objects;
    objects[QStringLiteral("plain")] = &plain;
    objects[QStringLiteral("channel")] = &channel;
    objects[QStringLiteral("publisher")] = channel.d_func()->publisher;
    objects[QStringLiteral("test")] = this;

    channel.registerObjects(objects);
}

void TestWebChannel::testDeregisterObjects()
{
    QWebChannel channel;
    TestObject testObject;
    testObject.setObjectName("myTestObject");


    channel.registerObject(testObject.objectName(), &testObject);

    channel.connectTo(m_dummyTransport);
    channel.d_func()->publisher->initializeClient(m_dummyTransport);

    QJsonObject connectMessage =
            QJsonDocument::fromJson(("{\"type\": 7,"
                                    "\"object\": \"myTestObject\","
                                    "\"signal\": " + QString::number(testObject.metaObject()->indexOfSignal("sig1()"))
                                    + "}").toLatin1()).object();
    channel.d_func()->publisher->handleMessage(connectMessage, m_dummyTransport);

    emit testObject.sig1();
    channel.deregisterObject(&testObject);
    emit testObject.sig1();
}

void TestWebChannel::testInfoForObject()
{
    TestObject obj;
    obj.setObjectName("myTestObject");

    QWebChannel channel;
    const QJsonObject info = channel.d_func()->publisher->classInfoForObject(&obj, m_dummyTransport);

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
            method.append(QStringLiteral("setObjectProperty"));
            method.append(obj.metaObject()->indexOfMethod("setObjectProperty(QObject*)"));
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
        {
            QJsonArray property;
            property.append(obj.metaObject()->indexOfProperty("objectProperty"));
            property.append(QStringLiteral("objectProperty"));
            {
                QJsonArray signal;
                signal.append(1);
                signal.append(obj.metaObject()->indexOfMethod("objectPropertyChanged()"));
                property.append(signal);
            }
            property.append(QJsonValue::fromVariant(QVariant::fromValue(obj.objectProperty())));
            expected.append(property);
        }
        QCOMPARE(info["properties"].toArray(), expected);
    }
}

void TestWebChannel::testInvokeMethodConversion()
{
    QWebChannel channel;
    channel.connectTo(m_dummyTransport);

    QJsonArray args;
    args.append(QJsonValue(1000));

    {
        int method = metaObject()->indexOfMethod("setInt(int)");
        QVERIFY(method != -1);
        channel.d_func()->publisher->invokeMethod(this, method, args);
        QCOMPARE(m_lastInt, args.at(0).toInt());
    }
    {
        int method = metaObject()->indexOfMethod("setDouble(double)");
        QVERIFY(method != -1);
        channel.d_func()->publisher->invokeMethod(this, method, args);
        QCOMPARE(m_lastDouble, args.at(0).toDouble());
    }
    {
        int method = metaObject()->indexOfMethod("setVariant(QVariant)");
        QVERIFY(method != -1);
        channel.d_func()->publisher->invokeMethod(this, method, args);
        QCOMPARE(m_lastVariant, args.at(0).toVariant());
    }
}

void TestWebChannel::testDisconnect()
{
    QWebChannel channel;
    channel.connectTo(m_dummyTransport);
    channel.disconnectFrom(m_dummyTransport);
    m_dummyTransport->emitMessageReceived(QJsonObject());
}

void TestWebChannel::testWrapRegisteredObject()
{
    QWebChannel channel;
    TestObject obj;
    obj.setObjectName("myTestObject");

    channel.registerObject(obj.objectName(), &obj);
    channel.connectTo(m_dummyTransport);
    channel.d_func()->publisher->initializeClient(m_dummyTransport);

    QJsonObject objectInfo = channel.d_func()->publisher->wrapResult(QVariant::fromValue(&obj), m_dummyTransport).toObject();

    QCOMPARE(2, objectInfo.length());
    QVERIFY(objectInfo.contains("id"));
    QVERIFY(objectInfo.contains("__QObject*__"));
    QVERIFY(objectInfo.value("__QObject*__").isBool() && objectInfo.value("__QObject*__").toBool());

    QString returnedId = objectInfo.value("id").toString();

    QCOMPARE(&obj, channel.d_func()->publisher->registeredObjects.value(obj.objectName()));
    QCOMPARE(obj.objectName(), channel.d_func()->publisher->registeredObjectIds.value(&obj));
    QCOMPARE(obj.objectName(), returnedId);
}

void TestWebChannel::testInfiniteRecursion()
{
    QWebChannel channel;
    TestObject obj;
    obj.setObjectProperty(&obj);
    obj.setObjectName("myTestObject");

    channel.connectTo(m_dummyTransport);
    channel.d_func()->publisher->initializeClient(m_dummyTransport);

    QJsonObject objectInfo = channel.d_func()->publisher->wrapResult(QVariant::fromValue(&obj), m_dummyTransport).toObject();
}

static QHash<QString, QObject*> createObjects(QObject *parent)
{
    const int num = 100;
    QHash<QString, QObject*> objects;
    objects.reserve(num);
    for (int i = 0; i < num; ++i) {
        objects[QStringLiteral("obj%1").arg(i)] = new BenchObject(parent);
    }
    return objects;
}

void TestWebChannel::benchClassInfo()
{
    QWebChannel channel;
    channel.connectTo(m_dummyTransport);

    QObject parent;
    const QHash<QString, QObject*> objects = createObjects(&parent);

    QBENCHMARK {
        foreach (const QObject *object, objects) {
            channel.d_func()->publisher->classInfoForObject(object, m_dummyTransport);
        }
    }
}

void TestWebChannel::benchInitializeClients()
{
    QWebChannel channel;
    channel.connectTo(m_dummyTransport);

    QObject parent;
    channel.registerObjects(createObjects(&parent));

    QMetaObjectPublisher *publisher = channel.d_func()->publisher;
    QBENCHMARK {
        publisher->initializeClient(m_dummyTransport);

        publisher->propertyUpdatesInitialized = false;
        publisher->signalToPropertyMap.clear();
        publisher->signalHandler.clear();
    }
}

void TestWebChannel::benchPropertyUpdates()
{
    QWebChannel channel;
    channel.connectTo(m_dummyTransport);

    QObject parent;
    const QHash<QString, QObject*> objects = createObjects(&parent);
    QVector<BenchObject*> objectList;
    objectList.reserve(objects.size());
    foreach (QObject *obj, objects) {
        objectList << qobject_cast<BenchObject*>(obj);
    }

    channel.registerObjects(objects);
    channel.d_func()->publisher->initializeClient(m_dummyTransport);

    QBENCHMARK {
        foreach (BenchObject *obj, objectList) {
            obj->change();
        }

        channel.d_func()->publisher->clientIsIdle = true;
        channel.d_func()->publisher->sendPendingPropertyUpdates();
    }
}

void TestWebChannel::benchRegisterObjects()
{
    QWebChannel channel;
    channel.connectTo(m_dummyTransport);

    QObject parent;
    const QHash<QString, QObject*> objects = createObjects(&parent);

    QBENCHMARK {
        channel.registerObjects(objects);
    }
}
#ifdef WEBCHANNEL_TESTS_CAN_USE_JS_ENGINE

class SubclassedTestObject : public TestObject
{
    Q_OBJECT
    Q_PROPERTY(QString bar READ bar WRITE setBar NOTIFY theBarHasChanged)
public:
    void setBar(const QString &newBar);
signals:
    void theBarHasChanged();
};

void SubclassedTestObject::setBar(const QString &newBar)
{
    if (!newBar.isNull())
        emit theBarHasChanged();
}

class TestSubclassedFunctor {
public:
    TestSubclassedFunctor(TestJSEngine *engine)
        : m_engine(engine)
    {
    }

    void operator()() {
        QCOMPARE(m_engine->logger()->errorCount(), 0);
    }

private:
    TestJSEngine *m_engine;
};
#endif // WEBCHANNEL_TESTS_CAN_USE_JS_ENGINE

void TestWebChannel::qtbug46548_overriddenProperties()
{
#ifndef WEBCHANNEL_TESTS_CAN_USE_JS_ENGINE
    QSKIP("A JS engine is required for this test to make sense.");
#else
    SubclassedTestObject obj;
    obj.setObjectName("subclassedTestObject");

    QWebChannel webChannel;
    webChannel.registerObject(obj.objectName(), &obj);
    TestJSEngine engine;
    webChannel.connectTo(engine.transport());
    QSignalSpy spy(&engine, &TestJSEngine::channelSetupReady);
    connect(&engine, &TestJSEngine::channelSetupReady, TestSubclassedFunctor(&engine));
    engine.initWebChannelJS();
    if (!spy.count())
        spy.wait();
    QCOMPARE(spy.count(), 1);
    QJSValue subclassedTestObject = engine.evaluate("channel.objects[\"subclassedTestObject\"]");
    QVERIFY(subclassedTestObject.isObject());

#endif // WEBCHANNEL_TESTS_CAN_USE_JS_ENGINE
}
QTEST_MAIN(TestWebChannel)

#include "tst_webchannel.moc"
