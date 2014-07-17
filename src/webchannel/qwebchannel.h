/****************************************************************************
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
** Copyright (C) 2014 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com, author Milian Wolff <milian.wolff@kdab.com>
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtWebChannel module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QWEBCHANNEL_H
#define QWEBCHANNEL_H

#include <QObject>
#include <QJsonValue>

#include <QtWebChannel/qwebchannelglobal.h>

QT_BEGIN_NAMESPACE

class QWebChannelPrivate;
class QWebChannelAbstractTransport;

class Q_WEBCHANNEL_EXPORT QWebChannel : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(QWebChannel)
    Q_PROPERTY(bool blockUpdates READ blockUpdates WRITE setBlockUpdates NOTIFY blockUpdatesChanged);
public:
    explicit QWebChannel(QObject *parent = 0);
    ~QWebChannel();

    /**
     * Register a map of string ID to QObject* objects.
     *
     * The properties, signals and public methods of the QObject are
     * published to the remote client, where an object with the given identifier
     * is constructed.
     *
     * TODO: This must be called, before clients are initialized.
     */
    void registerObjects(const QHash<QString, QObject*> &objects);
    QHash<QString, QObject*> registeredObjects() const;
    Q_INVOKABLE void registerObject(const QString &id, QObject *object);
    Q_INVOKABLE void deregisterObject(QObject *object);

    /**
     * @return true when property updates are blocked, false otherwise.
     */
    bool blockUpdates() const;

    /**
     * Set whether property updates should be blocked or not.
     *
     * When they are blocked, the remote clients will not be notified about
     * property changes. The changes are recorded and sent to the clients once
     * setBlockUpdates(false) is called.
     */
    void setBlockUpdates(bool block);

Q_SIGNALS:
    void blockUpdatesChanged(bool block);

public Q_SLOTS:
    void connectTo(QWebChannelAbstractTransport *transport);
    void disconnectFrom(QWebChannelAbstractTransport *transport);

    void sendMessage(const QJsonValue &id, const QJsonValue &data = QJsonValue()) const;

private:
    Q_DECLARE_PRIVATE(QWebChannel)
    QWebChannel(QWebChannelPrivate &dd, QObject *parent = 0);
    Q_PRIVATE_SLOT(d_func(), void _q_transportDestroyed(QObject*));

    friend class QMetaObjectPublisher;
    friend class QQmlWebChannel;
    friend class TestWebChannel;
};

QT_END_NAMESPACE

#endif // QWEBCHANNEL_H

