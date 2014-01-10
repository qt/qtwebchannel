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

#ifndef QWEBSOCKETTRANSPORT_H
#define QWEBSOCKETTRANSPORT_H

#include "qwebchanneltransportinterface.h"

QT_BEGIN_NAMESPACE

class QWebChannelSocket;
class Q_WEBCHANNEL_EXPORT QWebSocketTransport : public QObject, public QWebChannelTransportInterface
{
    Q_OBJECT
    Q_INTERFACES(QWebChannelTransportInterface)
    Q_PROPERTY(QString baseUrl READ baseUrl NOTIFY baseUrlChanged)
    Q_PROPERTY(bool useSecret READ useSecret WRITE setUseSecret)
public:
    explicit QWebSocketTransport(QObject *parent = 0);
    ~QWebSocketTransport() Q_DECL_OVERRIDE;

    void sendMessage(const QByteArray &message) const Q_DECL_OVERRIDE;
    void sendMessage(const QString &message) const Q_DECL_OVERRIDE;
    void setMessageHandler(QWebChannelMessageHandlerInterface *handler) Q_DECL_OVERRIDE;

    QString baseUrl() const;

    void setUseSecret(bool);
    bool useSecret() const;

Q_SIGNALS:
    void baseUrlChanged(const QString &baseUrl);
    void failed(const QString &reason);
    void initialized();
    void messageReceived(const QString &message);

private:
    QScopedPointer<QWebChannelSocket> d;
};

QT_END_NAMESPACE

#endif // QWEBSOCKETTRANSPORT_H
