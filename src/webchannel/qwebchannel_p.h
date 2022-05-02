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

#ifndef QWEBCHANNEL_P_H
#define QWEBCHANNEL_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "qwebchannelglobal.h"

#include <private/qobject_p.h>
#include <QList>

QT_BEGIN_NAMESPACE

class QJsonValue;
class QWebChannelAbstractTransport;
class QMetaObjectPublisher;

class Q_WEBCHANNEL_EXPORT QWebChannelPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QWebChannel)
public:
    QList<QWebChannelAbstractTransport *> transports;
    QMetaObjectPublisher *publisher;

    void init();

    void _q_transportDestroyed(QObject* object);
};

QT_END_NAMESPACE

#endif // QWEBCHANNEL_P_H
