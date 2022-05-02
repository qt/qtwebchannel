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

#ifndef VARIANTARGUMENT_H
#define VARIANTARGUMENT_H

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

#include <QVariant>

QT_BEGIN_NAMESPACE

/**
 * RAII QVariant to Q[Generic]Argument conversion
 */
struct VariantArgument
{
    operator QGenericArgument() const
    {
        if (type == QMetaType::QVariant) {
            return Q_ARG(QVariant, value);
        }
        if (!value.isValid()) {
            return QGenericArgument();
        }
        return QGenericArgument(value.typeName(), value.constData());
    }

    QVariant value;
    int type;
};

QT_END_NAMESPACE

#endif // VARIANTARGUMENT_H
