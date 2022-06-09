// Copyright (C) 2016 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com, author Milian Wolff <milian.wolff@kdab.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

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
#include <private/qglobal_p.h>

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
