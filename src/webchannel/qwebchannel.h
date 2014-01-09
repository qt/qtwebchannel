/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Copyright (C) 2013 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com, author Milian Wolff <milian.wolff@kdab.com>
** Contact: http://www.qt-project.org/legal
*
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

class QWebChannelPrivate;

class Q_WEBCHANNEL_EXPORT QWebChannel : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(QWebChannel)
    Q_PROPERTY(QString baseUrl READ baseUrl NOTIFY baseUrlChanged)
    Q_PROPERTY(bool useSecret READ useSecret WRITE setUseSecret)
    Q_PROPERTY(bool blockUpdates READ blockUpdates WRITE setBlockUpdates NOTIFY blockUpdatesChanged);

public:
    QWebChannel(QObject *parent = 0);
    ~QWebChannel();

    QString baseUrl() const;

    void setUseSecret(bool);
    bool useSecret() const;

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

signals:
    void baseUrlChanged(const QString& baseUrl);
    void rawMessageReceived(const QString& rawMessage);
    void pongReceived();
    void initialized();

    void failed(const QString& reason);

    void blockUpdatesChanged(bool block);

public slots:
    void sendMessage(const QJsonValue& id, const QJsonValue& data = QJsonValue()) const;
    void respond(const QJsonValue& messageId, const QJsonValue& data = QJsonValue()) const;
    void sendRawMessage(const QString& rawMessage) const;
    void ping() const;

private:
    QScopedPointer<QWebChannelPrivate> d;
    friend class QmlWebChannel;
    friend class TestWebChannel;
};

#endif // QWEBCHANNEL_H

