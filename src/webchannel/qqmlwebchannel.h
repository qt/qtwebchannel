/****************************************************************************
**
** Copyright (C) 2016 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com, author Milian Wolff <milian.wolff@kdab.com>
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWebChannel module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:COMM$
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
******************************************************************************/

#ifndef QQMLWEBCHANNEL_H
#define QQMLWEBCHANNEL_H

#include <QtWebChannel/QWebChannel>
#include <QtWebChannel/qwebchannelglobal.h>

#include <QtQml/qqml.h>
#include <QtQml/QQmlListProperty>

QT_BEGIN_NAMESPACE

class QQmlWebChannelPrivate;
class QQmlWebChannelAttached;
class Q_WEBCHANNEL_EXPORT QQmlWebChannel : public QWebChannel
{
    Q_OBJECT
    Q_DISABLE_COPY(QQmlWebChannel)

    Q_PROPERTY( QQmlListProperty<QObject> transports READ transports )
    Q_PROPERTY( QQmlListProperty<QObject> registeredObjects READ registeredObjects )

public:
    explicit QQmlWebChannel(QObject *parent = nullptr);
    virtual ~QQmlWebChannel();

    Q_INVOKABLE void registerObjects(const QVariantMap &objects);
    QQmlListProperty<QObject> registeredObjects();

    QQmlListProperty<QObject> transports();

    static QQmlWebChannelAttached *qmlAttachedProperties(QObject *obj);

    Q_INVOKABLE void connectTo(QObject *transport);
    Q_INVOKABLE void disconnectFrom(QObject *transport);

private:
    Q_DECLARE_PRIVATE(QQmlWebChannel)
    Q_PRIVATE_SLOT(d_func(), void _q_objectIdChanged(const QString &newId))

    static void registeredObjects_append(QQmlListProperty<QObject> *prop, QObject *item);
    static qsizetype registeredObjects_count(QQmlListProperty<QObject> *prop);
    static QObject *registeredObjects_at(QQmlListProperty<QObject> *prop, qsizetype index);
    static void registeredObjects_clear(QQmlListProperty<QObject> *prop);

    static void transports_append(QQmlListProperty<QObject> *prop, QObject *item);
    static qsizetype transports_count(QQmlListProperty<QObject> *prop);
    static QObject *transports_at(QQmlListProperty<QObject> *prop, qsizetype index);
    static void transports_clear(QQmlListProperty<QObject> *prop);
};

QT_END_NAMESPACE

QML_DECLARE_TYPE( QQmlWebChannel )
QML_DECLARE_TYPEINFO( QQmlWebChannel, QML_HAS_ATTACHED_PROPERTIES )

#endif // QQMLWEBCHANNEL_H
