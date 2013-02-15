/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
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

#include "qwebchannel.h"

#include <QUuid>
#include <QStringList>
#include <QDebug>

#include "qwebsocketserver.h"

class QWebChannelPrivate : public QObject
{
    Q_OBJECT
public:
    QWebSocketServer m_server;
    QString m_secret;
    bool m_useSecret;

    QString m_baseUrl;
    bool m_starting;

    QWebChannelPrivate(QObject* parent)
    : QObject(parent)
    , m_useSecret(true)
    , m_starting(false)
    {
        connect(&m_server, SIGNAL(error(QAbstractSocket::SocketError)),
                SLOT(socketError()));
    }

    virtual ~QWebChannelPrivate()
    {
        m_server.close();
    }

    void initLater()
    {
        if (m_starting)
            return;
        metaObject()->invokeMethod(this, "init", Qt::QueuedConnection);
        m_starting = true;
    }

signals:
    void failed(const QString& reason);
    void initialized();

private slots:
    void init();
    void socketError();
};

void QWebChannelPrivate::init()
{
    m_server.close();

    m_starting = false;
    if (m_useSecret) {
        m_secret = QUuid::createUuid().toString();
        m_secret = m_secret.mid(1, m_secret.size() - 2);
    }

    if (!m_server.listen(QHostAddress::LocalHost)) {
        emit failed(m_server.errorString());
        return;
    }

    m_baseUrl = QString("localhost:%1/%2").arg(m_server.port()).arg(m_secret);
    qDebug() << m_baseUrl;
    emit initialized();
}

void QWebChannelPrivate::socketError()
{
    emit failed(m_server.errorString());
}

QWebChannel::QWebChannel(QObject *parent)
: QObject(parent)
, d(new QWebChannelPrivate(this))
{
    connect(&d->m_server, SIGNAL(textDataReceived(QString)),
            SIGNAL(rawMessageReceived(QString)));
    connect(d, SIGNAL(failed(QString)),
            SIGNAL(failed(QString)));
    connect(d, SIGNAL(initialized()),
            SLOT(onInitialized()));
    d->initLater();
}

QWebChannel::~QWebChannel()
{
}

QString QWebChannel::baseUrl() const
{
    return d->m_baseUrl;
}

void QWebChannel::setUseSecret(bool s)
{
    if (d->m_useSecret == s)
        return;
    d->m_useSecret = s;
    d->initLater();
}

bool QWebChannel::useSecret() const
{
    return d->m_useSecret;
}

void QWebChannel::onInitialized()
{
    emit initialized();
    emit baseUrlChanged(d->m_baseUrl);
}

void QWebChannel::sendRawMessage(const QString& message)
{
    d->m_server.sendMessage(message);
}

#include <qwebchannel.moc>
