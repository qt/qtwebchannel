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

#ifndef QTMETAOBJECTPUBLISHER_H
#define QTMETAOBJECTPUBLISHER_H

#include <QObject>
#include <QVariant>
#include <QJsonObject>

#include "qwebchannelglobal.h"

class QWebChannel;

struct QMetaObjectPublisherPrivate;

class Q_WEBCHANNEL_EXPORT QMetaObjectPublisher : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QWebChannel *webChannel READ webChannel WRITE setWebChannel NOTIFY webChannelChanged);
    Q_PROPERTY(bool blockUpdates READ blockUpdates WRITE setBlockUpdates NOTIFY blockUpdatesChanged);

public:
    explicit QMetaObjectPublisher(QObject *parent = 0);
    virtual ~QMetaObjectPublisher();

    Q_INVOKABLE QJsonObject classInfoForObjects(const QVariantMap &objects) const;
    Q_INVOKABLE QJsonObject classInfoForObject(QObject *object) const;

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
    Q_INVOKABLE bool test_clientIsIdle() const;

signals:
    void webChannelChanged(QWebChannel *channel);
    void blockUpdatesChanged(bool block);

public slots:
    /**
     * Helper slot which you can connect directly to WebChannel's rawMessageReceived signal.
     *
     * This slot then tries to parse the message as JSON and if it succeeds, calls handleRequest
     * with the obtained JSON object.
     */
    void handleRawMessage(const QString &message);

protected:
    void timerEvent(QTimerEvent *) Q_DECL_OVERRIDE;

private:
    QScopedPointer<QMetaObjectPublisherPrivate> d;
    friend struct QMetaObjectPublisherPrivate;
    friend class TestWebChannel;
};

#endif // QMETAOBJECTPUBLISHER_H
