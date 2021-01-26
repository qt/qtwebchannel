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
****************************************************************************/

#ifndef QWEBCHANNELABSTRACTTRANSPORT_H
#define QWEBCHANNELABSTRACTTRANSPORT_H

#include <QtCore/QObject>
#include <QtWebChannel/qwebchannelglobal.h>

QT_BEGIN_NAMESPACE

class QJsonObject;
class Q_WEBCHANNEL_EXPORT QWebChannelAbstractTransport : public QObject
{
    Q_OBJECT
public:
    explicit QWebChannelAbstractTransport(QObject *parent = Q_NULLPTR);
    virtual ~QWebChannelAbstractTransport();

public Q_SLOTS:
    virtual void sendMessage(const QJsonObject &message) = 0;

Q_SIGNALS:
    void messageReceived(const QJsonObject &message, QWebChannelAbstractTransport *transport);
};

QT_END_NAMESPACE

#endif // QWEBCHANNELABSTRACTTRANSPORT_H
