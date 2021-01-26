/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
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
****************************************************************************/

#ifndef QWEBCHANNEL_H
#define QWEBCHANNEL_H

#include <QtCore/QObject>
#include <QtCore/QJsonValue>

#include <QtWebChannel/qwebchannelglobal.h>

QT_BEGIN_NAMESPACE

class QWebChannelPrivate;
class QWebChannelAbstractTransport;

class Q_WEBCHANNEL_EXPORT QWebChannel : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(QWebChannel)
    Q_PROPERTY(bool blockUpdates READ blockUpdates WRITE setBlockUpdates NOTIFY blockUpdatesChanged)
public:
    explicit QWebChannel(QObject *parent = Q_NULLPTR);
    ~QWebChannel();

    void registerObjects(const QHash<QString, QObject*> &objects);
    QHash<QString, QObject*> registeredObjects() const;
    Q_INVOKABLE void registerObject(const QString &id, QObject *object);
    Q_INVOKABLE void deregisterObject(QObject *object);

    bool blockUpdates() const;

    void setBlockUpdates(bool block);

Q_SIGNALS:
    void blockUpdatesChanged(bool block);

public Q_SLOTS:
    void connectTo(QWebChannelAbstractTransport *transport);
    void disconnectFrom(QWebChannelAbstractTransport *transport);

private:
    Q_DECLARE_PRIVATE(QWebChannel)
    QWebChannel(QWebChannelPrivate &dd, QObject *parent = Q_NULLPTR);
    Q_PRIVATE_SLOT(d_func(), void _q_transportDestroyed(QObject*))

    friend class QMetaObjectPublisher;
    friend class QQmlWebChannel;
    friend class TestWebChannel;
};

QT_END_NAMESPACE

#endif // QWEBCHANNEL_H

