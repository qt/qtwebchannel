#include "testobject.h"

TestObject::TestObject(QObject *parent) :
    QObject(parent)
  , p1("Hello World")
{
    connect(&timer, SIGNAL(timeout()), this, SIGNAL(timeout()));
}

QString TestObject::debugMe(const QString& data)
{
    qWarning() << data;
    return QString("OK from %1: %2").arg(objectName()).arg(data);
}

void TestObject::setProp1(const QString& s)
{
    p1 = s;
    qWarning() << __func__ << p1;
    emit sig1(1, 0.5, QStringLiteral("asdf"));
    emit prop1Changed();
}

void TestObject::setProp2(const QString& s)
{
    p2 = s;
    qWarning() << __func__ << p2;
    emit sig2();
    emit prop2Changed(s);
}

QString TestObject::manyArgs(int a, float b, const QString& c) const
{
    qDebug() << a << b << c;
    return c;
}

TestObjectFactory::TestObjectFactory(QObject* parent)
    : QObject(parent)
{

}

TestObject* TestObjectFactory::createObject(const QString& name)
{
    TestObject* ret = new TestObject(this);
    ret->setObjectName(name);
    return ret;
}
