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

#ifndef QMESSAGEPASSINGINTERFACE_H
#define QMESSAGEPASSINGINTERFACE_H

#include <QObject>

QT_BEGIN_NAMESPACE

/*!
    \inmodule QtWebChannel
    \brief The interface used for communication between the QWebChannel server and its HTML clients.

    In order to communicate with HTML clients an object implementing this interface must be passed
    to QWebChannel::connectTo(). Currently this interface is implemented by QWebSocket for pure C++
    applications. QML applications can use the declarative WebSocket or WebView.experimental's
    \c navigator.qt functionality.
 */
class QMessagePassingInterface
{
public:
    virtual ~QMessagePassingInterface() {}

    /*!
        Send a text \a message to the remote client.
    */
    virtual qint64 sendTextMessage(const QString &message) = 0;

    /*!
        Emitted when a new text \a message was received from the remote client.

        \note This should be implemented as a signal.
    */
    virtual void textMessageReceived(const QString &message) = 0;
};

#define QMessagePassingInterface_iid "org.qt-project.Qt.QMessagePassingInterface/1.0"
Q_DECLARE_INTERFACE(QMessagePassingInterface, QMessagePassingInterface_iid);
Q_DECLARE_METATYPE(QMessagePassingInterface*)

QT_END_NAMESPACE

#endif // QMESSAGEPASSINGINTERFACE_H
