#include "qobjectwrapper.h"
#include "qtmetaobjectpublisher.h"

#include <QDebug>
#include <QStringList>
#include <QPointer>
#include <QTextStream>

QObjectWrapper::QObjectWrapper(QtMetaObjectPublisher *parent)
    : QObject(parent)
{
}

static const QString KEY_QOBJECT = QStringLiteral("__QObject*__");
static const QString KEY_ID = QStringLiteral("id");
static const QString KEY_DATA = QStringLiteral("data");
#ifdef DEBUG_QOBJECTWRAPPER
static const char* ID_PROPERTY = "__id__";
#endif

QVariant QObjectWrapper::wrap(QObject *object)
{
    if (!object)
        return QVariant();

    const QString& id = objectId(object);

    const WrapMapCIt& p = m_wrappedObjects.constFind(id);
    if (p != m_wrappedObjects.constEnd())
        return p.value().second;

    QtMetaObjectPublisher *publisher = qobject_cast<QtMetaObjectPublisher*>(parent());
    Q_ASSERT(publisher);
    QVariantMap objectInfo;
    objectInfo[KEY_QOBJECT] = true;
    objectInfo[KEY_ID] = id;
    objectInfo[KEY_DATA] = publisher->classInfoForObject(object);

    m_wrappedObjects.insert(id, WrapInfo(object, objectInfo));
#ifdef DEBUG_QOBJECTWRAPPER
    Q_ASSERT(!object->property(ID_PROPERTY).isValid());
    object->setProperty(ID_PROPERTY, id);
#endif
    connectToObjectDestroyed(object, id);

    return objectInfo;
}

QObject *QObjectWrapper::unwrap(const QString& id) const
{
    const WrapMapCIt& p = m_wrappedObjects.constFind(id);
    if (p != m_wrappedObjects.constEnd())
        return p.value().first;
    return 0;
}

bool QObjectWrapper::contains(const QString &id) const
{
    return m_wrappedObjects.constFind(id) != m_wrappedObjects.constEnd();
}

void QObjectWrapper::deleteObjectLater(QObject *object) const
{
    Q_ASSERT(object);
#ifdef DEBUG_QOBJECTWRAPPER
    Q_ASSERT(object->property(ID_PROPERTY).isValid()
             && contains(object->property(ID_PROPERTY).toString()));
#endif
    object->deleteLater();
}

QString QObjectWrapper::objectId(QObject *object)
{
    QString result;
    QTextStream(&result, QIODevice::WriteOnly)
       << object->metaObject()->className()
       << QChar('@')
       << static_cast<void*>(object);
    return result;
}

void QObjectWrapper::connectToObjectDestroyed(QObject *object, const QString &id)
{
    connect(object, &QObject::destroyed, new DestroyHelper(id, this), &DestroyHelper::objectDestroyed);
}

void QObjectWrapper::onObjectDestroyed(const QString &id)
{
    {
#ifdef DEBUG_QOBJECTWRAPPER
        qDebug() << "Removing" << id << "from QObjectWrapper @" << static_cast<void*>(this);
#endif
        m_wrappedObjects.remove(id);
    }
    emit objectDestroyed(id);
}


QObjectWrapper::DestroyHelper::DestroyHelper(const QString &id, QObjectWrapper *parent)
    : QObject(parent)
    , m_id(id)
{
    Q_ASSERT(parent);
}

void QObjectWrapper::DestroyHelper::objectDestroyed()
{
    static_cast<QObjectWrapper*>(QObject::parent())->onObjectDestroyed(m_id);
    delete this;
}


