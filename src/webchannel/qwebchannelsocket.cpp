/****************************************************************************
**
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

#include "qwebchannelsocket_p.h"

#include <QUuid>
#include <QDebug>

QT_BEGIN_NAMESPACE

QWebChannelSocket::QWebChannelSocket(QObject *parent)
    : QWebSocketServer(parent)
    , m_messageHandler(Q_NULLPTR)
    , m_useSecret(true)
    , m_starting(false)
{
    connect(this, SIGNAL(error(QAbstractSocket::SocketError)),
            SLOT(socketError()));
    connect(this, SIGNAL(textDataReceived(QString)),
            SLOT(messageReceived(QString)));
}

void QWebChannelSocket::initLater()
{
    if (m_starting)
        return;
    metaObject()->invokeMethod(this, "init", Qt::QueuedConnection);
    m_starting = true;
}

bool QWebChannelSocket::isValid(const HeaderData &connection)
{
    if (!QWebSocketServer::isValid(connection)) {
        return false;
    }
    return connection.protocol == QByteArrayLiteral("QWebChannel")
            && connection.path == m_secret;
}

void QWebChannelSocket::init()
{
    close();

    m_starting = false;
    if (m_useSecret) {
        m_secret = QUuid::createUuid().toByteArray();
        // replace { by /
        m_secret[0] = '/';
        // chop of trailing }
        m_secret.chop(1);
    }

    if (!listen(QHostAddress::LocalHost)) {
        emit failed(errorString());
        return;
    }

    m_baseUrl = QStringLiteral("127.0.0.1:%1%2").arg(port()).arg(QString::fromLatin1(m_secret));
    emit initialized();
    emit baseUrlChanged(m_baseUrl);
}

void QWebChannelSocket::socketError()
{
    emit failed(errorString());
}

void QWebChannelSocket::messageReceived(const QString &message)
{
    if (m_messageHandler) {
        m_messageHandler->handleMessage(message);
    }
}

QT_END_NAMESPACE
