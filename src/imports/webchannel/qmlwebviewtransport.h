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

#ifndef QMLWEBVIEWTRANSPORT_H
#define QMLWEBVIEWTRANSPORT_H

#include <qwebchanneltransportinterface.h>

QT_BEGIN_NAMESPACE

class QmlWebViewTransport : public QObject, public QWebChannelTransportInterface
{
    Q_OBJECT
    Q_INTERFACES(QWebChannelTransportInterface)
    Q_PROPERTY(QObject *webViewExperimental READ webViewExperimental WRITE setWebViewExperimental NOTIFY webViewChanged)
public:
    explicit QmlWebViewTransport(QObject *parent = 0);
    ~QmlWebViewTransport() Q_DECL_OVERRIDE;

    void sendMessage(const QString &message) const Q_DECL_OVERRIDE;
    void sendMessage(const QByteArray &message) const Q_DECL_OVERRIDE;
    void setMessageHandler(QWebChannelMessageHandlerInterface *handler) Q_DECL_OVERRIDE;

    void setWebViewExperimental(QObject *webViewExperimental);
    QObject *webViewExperimental() const;

signals:
    void webViewChanged(QObject *webViewExperimental);
    void messageReceived(const QString &message);

private slots:
    void handleWebViewMessage(const QVariantMap &message);

private:
    QObject *m_webViewExperimental;
    QWebChannelMessageHandlerInterface *m_handler;
};

QT_END_NAMESPACE

#endif // QMLWEBVIEWTRANSPORT_H
