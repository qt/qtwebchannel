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

#ifndef QMLWEBCHANNEL_H
#define QMLWEBCHANNEL_H

#include <qwebchannel.h>

#include "qmlwebchannelattached.h"
#include "qmessagepassinginterface.h"

#include <QVector>

#include <QtQml/qqml.h>
#include <QtQml/QQmlListProperty>

QT_BEGIN_NAMESPACE

class QmlWebChannel : public QWebChannel
{
    Q_OBJECT

    Q_PROPERTY( QQmlListProperty<QMessagePassingInterface> connections READ transports );
    Q_PROPERTY( QQmlListProperty<QObject> registeredObjects READ registeredObjects )

public:
    explicit QmlWebChannel(QObject *parent = 0);
    virtual ~QmlWebChannel();

    Q_INVOKABLE void registerObjects(const QVariantMap &objects);
    QQmlListProperty<QObject> registeredObjects();

    QQmlListProperty<QMessagePassingInterface> transports();

    // TODO: remove this by replacing QML with C++ tests
    Q_INVOKABLE bool test_clientIsIdle() const;

    static QmlWebChannelAttached *qmlAttachedProperties(QObject *obj);

    Q_INVOKABLE void connectTo(QObject *transport);
    Q_INVOKABLE void disconnectFrom(QObject *transport);

private slots:
    void objectIdChanged(const QString &newId);
    void transportDestroyed(QObject *transport);

private:
    static void registeredObjects_append(QQmlListProperty<QObject> *prop, QObject *item);
    static int registeredObjects_count(QQmlListProperty<QObject> *prop);
    static QObject *registeredObjects_at(QQmlListProperty<QObject> *prop, int index);
    static void registeredObjects_clear(QQmlListProperty<QObject> *prop);

    static void transports_append(QQmlListProperty<QMessagePassingInterface> *prop, QMessagePassingInterface *item);
    static int transports_count(QQmlListProperty<QMessagePassingInterface> *prop);
    static QMessagePassingInterface *transports_at(QQmlListProperty<QMessagePassingInterface> *prop, int index);
    static void transports_clear(QQmlListProperty<QMessagePassingInterface> *prop);

    QVector<QObject*> m_registeredObjects;
    // required as when the object is destroyed, we must still find the address of the base class somehow
    QHash<QObject*, QMessagePassingInterface*> m_connectedObjects;
};

QML_DECLARE_TYPE( QmlWebChannel )
QML_DECLARE_TYPEINFO( QmlWebChannel, QML_HAS_ATTACHED_PROPERTIES )

QT_END_NAMESPACE

#endif // QMLWEBCHANNEL_H
