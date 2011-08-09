#ifndef QTMETAOBJECTPUBLISHER_H
#define QTMETAOBJECTPUBLISHER_H

#include <QObject>
#include <QVariantMap>
#include <QPointer>
#include <QStringList>

class QtMetaObjectPublisher : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QStringList objectNames READ objectNames)
public:
    explicit QtMetaObjectPublisher(QObject *parent = 0);
    Q_INVOKABLE QVariantMap classInfoForObject(QObject*);
    QStringList objectNames() { return objects.keys(); }
    Q_INVOKABLE QObject* namedObject(const QString& name) {
        if (!objects.contains(name))
            return 0;
        return objects[name];
    }

public slots:
    void addObject(const QString& name, QObject* object)
    {
        objects[name] = object;
    }

private:
    QMap<QString, QPointer<QObject> > objects;

};

#endif // QTMETAOBJECTPUBLISHER_H
