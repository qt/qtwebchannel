#ifndef QOBJECTWRAPPER_H
#define QOBJECTWRAPPER_H

#include <QObject>
#include <QVariant>
#include <QVariantMap>

class QtMetaObjectPublisher;

/**
 * C++ QObject wrapper for WebChannel
 */
class QObjectWrapper : public QObject
{
    Q_OBJECT
    /// Pairs of QObject and generated object info
    typedef QPair<QObject *, QVariantMap> WrapInfo;
    /// Maps object id to wrap info
    typedef QHash<QString, WrapInfo> WrapMap;
    /// Const iterator for map
    typedef WrapMap::const_iterator WrapMapCIt;

public:
    /// Constructor
    explicit QObjectWrapper(QtMetaObjectPublisher *parent = 0);

    /// Wrap a QObject and return object info
    Q_INVOKABLE QVariant wrap(QObject *object);
    /// Search object by id and return it
    Q_INVOKABLE QObject *unwrap(const QString &id) const;
    /// Test whether object with id is wrapped
    Q_INVOKABLE bool contains(const QString &id) const;
    /// Ask deletion of object
    Q_INVOKABLE void deleteObjectLater(QObject *object) const;

signals:
    /// Emitted when object with id `id` reached the end of its life
    void objectDestroyed(const QString& id);

private:
    /// Needed for C++03 support (no lambdas)
    class DestroyHelper;

private:
    /// Generate ID for QObject. Currently based on metatype name and physical address
    static QString objectId(QObject* object);
    /// Connect a handler to `destroyed` signal of object which removes it from the map
    void connectToObjectDestroyed(QObject* object, const QString& id);
    /// Object with ID destroyed
    void onObjectDestroyed(const QString& id);

private:
    /// Map of wrapped objects
    WrapMap m_wrappedObjects;
};

/**
 * @brief Helper class to compensate for lack of lambdas in old C++
 */
class QObjectWrapper::DestroyHelper: public QObject
{
    Q_OBJECT
public:
    DestroyHelper(const QString& id, QObjectWrapper *parent);
public slots:
    void objectDestroyed();
private:
    const QString m_id;
};

#endif // QOBJECTWRAPPER_H
