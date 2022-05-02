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

#ifndef QQMLWEBCHANNELATTACHED_H
#define QQMLWEBCHANNELATTACHED_H

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

#include <QObject>

#include "qwebchannelglobal.h"

QT_BEGIN_NAMESPACE

class Q_WEBCHANNEL_EXPORT QQmlWebChannelAttached : public QObject
{
    Q_OBJECT

    Q_PROPERTY( QString id READ id WRITE setId NOTIFY idChanged FINAL )
public:
    explicit QQmlWebChannelAttached(QObject *parent = 0);
    virtual ~QQmlWebChannelAttached();

    QString id() const;
    void setId(const QString &id);

Q_SIGNALS:
    void idChanged(const QString &id);

private:
    QString m_id;
};

QT_END_NAMESPACE

#endif // QQMLWEBCHANNELATTACHED_H
