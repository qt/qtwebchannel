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
}

void TestObject::setProp2(const QString& s)
{
    p2 = s;
    qWarning() << __func__ << p2;
}
