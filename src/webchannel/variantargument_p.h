/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Copyright (C) 2013 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com, author Milian Wolff <milian.wolff@kdab.com>
** Contact: http://www.qt-project.org/legal
*
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

#ifndef VARIANTARGUMENT_H
#define VARIANTARGUMENT_H

#include <QVariant>
#include <QMetaType>

/**
 * RAII QVariant to Q[Generic]Argument conversion
 */
class VariantArgument
{
public:
    explicit VariantArgument()
    : m_data(0)
    , m_paramType(0)
    {
    }

    /// TODO: test with C++ methods that don't take a QVariant as arg
    ///       also test conversions
    void setValue(const QVariant &value, int paramType)
    {
        if (m_data) {
            QMetaType::destroy(m_paramType, m_data);
            m_name.clear();
            m_data = 0;
        }

        m_paramType = paramType;

        if (value.isValid()) {
            m_name = value.typeName();
            m_data = QMetaType::create(m_paramType, value.constData());
        }
    }

    ~VariantArgument()
    {
        if (m_data) {
            QMetaType::destroy(m_paramType, m_data);
            m_data = 0;
        }
    }

    operator QGenericArgument() const
    {
        if (!m_data) {
            return QGenericArgument();
        }
        return QGenericArgument(m_name.constData(), m_data);
    }

private:
    Q_DISABLE_COPY(VariantArgument)

    QByteArray m_name;
    void* m_data;
    int m_paramType;
};

#endif // VARIANTARGUMENT_H
