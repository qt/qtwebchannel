#include "qtmetaobjectpublisher.h"
#include <QVariantMap>
#include <QStringList>
#include <QMetaObject>
#include <QMetaProperty>

QtMetaObjectPublisher::QtMetaObjectPublisher(QObject *parent) :
    QObject(parent)
{
}

QVariantMap QtMetaObjectPublisher::classInfoForObject(QObject *object)
{
    QVariantMap data;
    QStringList qtSignals, qtMethods, qtProperties;
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
    data["signals"] = qtSignals;
    data["methods"] = qtMethods;
    data["properties"] = qtProperties;
    return data;
}
