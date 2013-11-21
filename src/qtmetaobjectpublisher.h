/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** Copyright (C) 2013 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com, author Milian Wolff <milian.wolff@kdab.com>
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QWebChannel module on Qt labs.
**
** $QT_BEGIN_LICENSE:LGPL$
** No Commercial Usage
** This file contains pre-release code and may not be distributed.
** You may use this file in accordance with the terms and conditions
** contained in the Technology Preview License Agreement accompanying
** this package.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights.  These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** If you have questions regarding the use of this file, please contact
** Nokia at qt-info@nokia.com.
**
**
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QTMETAOBJECTPUBLISHER_H
#define QTMETAOBJECTPUBLISHER_H

#include <QObject>

class QWebChannel;
typedef QMap<QString, QVariant> QVariantMap;

struct QtMetaObjectPublisherPrivate;

class QtMetaObjectPublisher : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QWebChannel *webChannel READ webChannel WRITE setWebChannel NOTIFY webChannelChanged);
    Q_PROPERTY(bool blockUpdates READ blockUpdates WRITE setBlockUpdates NOTIFY blockUpdatesChanged);

public:
    explicit QtMetaObjectPublisher(QObject *parent = 0);
    virtual ~QtMetaObjectPublisher();

    Q_INVOKABLE QVariantMap classInfoForObjects(const QVariantMap &objects) const;
    Q_INVOKABLE QVariantMap classInfoForObject(QObject *object) const;

    /**
     * Register a map of string ID to QObject* objects.
     *
     * The properties, signals and public methods of the QObject are
     * published to the remote client, where an object with the given identifier
     * is constructed.
     *
     * TODO: This must be called, before clients are initialized.
     */
    Q_INVOKABLE void registerObjects(const QVariantMap &objects);

    /**
     * Handle the given WebChannel client request and potentially give a response.
     *
     * @return true if the request was handled, false otherwise.
     */
    Q_INVOKABLE bool handleRequest(const QJsonObject &message);

    QWebChannel *webChannel() const;
    void setWebChannel(QWebChannel *webChannel);

    /**
     * When updates are blocked, no property updates are transmitted to remote clients.
     */
    bool blockUpdates() const;
    void setBlockUpdates(bool block);

    /// TODO: cleanup: rewrite tests in C++ and access PIMPL data from there
    Q_INVOKABLE void bench_ensureUpdatesInitialized();
    Q_INVOKABLE void bench_sendPendingPropertyUpdates();
    Q_INVOKABLE void bench_registerObects(const QVariantMap &objects);
    Q_INVOKABLE void bench_initializeClients();
    Q_INVOKABLE bool test_clientIsIdle() const;

signals:
    void webChannelChanged(QWebChannel *channel);
    void blockUpdatesChanged(bool block);

private:
    QScopedPointer<QtMetaObjectPublisherPrivate> d;
    friend struct QtMetaObjectPublisherPrivate;

    Q_PRIVATE_SLOT(d, void sendPendingPropertyUpdates());
};

#endif // QTMETAOBJECTPUBLISHER_H
