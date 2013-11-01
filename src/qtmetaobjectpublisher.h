/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** Copyright (C) 2013 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com, author Milian Wolff <milian.wolff@kdab.com>
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QWebChannel module on Qt labs.
**
** $QT_BEGIN_LICENSE:LGPL$
** No Commercial Usage
** This file contains pre-release code and may not be distributed.
** You may use this file in accordance with the terms and conditions
** contained in the Technology Preview License Agreement accompanying
** this package.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights.  These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** If you have questions regarding the use of this file, please contact
** Nokia at qt-info@nokia.com.
**
**
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QTMETAOBJECTPUBLISHER_H
#define QTMETAOBJECTPUBLISHER_H

#include <QObject>
#include <QVariantMap>
#include <QQuickItem>

// NOTE: QQuickItem inheritance required to enable QML item nesting (i.e. Timer in MetaObjectPublisher)
class QtMetaObjectPublisher : public QQuickItem
{
    Q_OBJECT
public:
    explicit QtMetaObjectPublisher(QQuickItem *parent = 0);

    Q_INVOKABLE QVariantMap classInfoForObjects(const QVariantMap &objects) const;
    Q_INVOKABLE QVariantMap classInfoForObject(QObject *object) const;

    /// wrap object and return class info
    Q_INVOKABLE QVariant wrapObject(QObject *object);
    /// Search object by id and return it, or null if it could not be found.
    Q_INVOKABLE QObject *unwrapObject(const QString &id) const;
    /// Invoke delete later on @p object, but only if it is a wrapped object.
    Q_INVOKABLE void deleteWrappedObject(QObject *object) const;

signals:
    void wrappedObjectDestroyed(const QString& id);

private slots:
    void wrappedObjectDestroyed(QObject* object);

private:
    /// Pairs of QObject and generated object info
    typedef QPair<QObject *, QVariantMap> WrapInfo;
    /// Maps object id to wrap info
    typedef QHash<QString, WrapInfo> WrapMap;
    /// Const iterator for map
    typedef WrapMap::const_iterator WrapMapCIt;

    /// Map of wrapped objects
    WrapMap m_wrappedObjects;
};

#endif // QTMETAOBJECTPUBLISHER_H
