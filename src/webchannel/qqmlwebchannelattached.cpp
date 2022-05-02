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
******************************************************************************/

#include "qqmlwebchannelattached_p.h"

QT_USE_NAMESPACE

QQmlWebChannelAttached::QQmlWebChannelAttached(QObject *parent)
    : QObject(parent)
{

}

QQmlWebChannelAttached::~QQmlWebChannelAttached()
{

}

/*!
    \qmlattachedproperty QString WebChannel::id

    \brief The identifier under which an object, registered to a WebChannel, is known to remote clients.

    This property must be set for every object that should be published over the WebChannel.
    While no restrictions are enforced on the format of the id, it is usually a good idea to
    choose a string that is also a valid JavaScript identifier.
*/
QString QQmlWebChannelAttached::id() const
{
    return m_id;
}

void QQmlWebChannelAttached::setId(const QString &id)
{
    if (id != m_id) {
        m_id = id;
        emit idChanged(id);
    }
}
