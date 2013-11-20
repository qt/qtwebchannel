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

#ifndef QWEBCHANNEL_H
#define QWEBCHANNEL_H

#include <QObject>
#include <QJsonValue>

class QWebChannelPrivate;

class QWebChannel : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(QWebChannel)
    Q_PROPERTY(QString baseUrl READ baseUrl NOTIFY baseUrlChanged)
    Q_PROPERTY(bool useSecret READ useSecret WRITE setUseSecret)

public:
    QWebChannel(QObject *parent = 0);
    ~QWebChannel();

    QString  baseUrl() const;

    void setUseSecret(bool);
    bool useSecret() const;

signals:
    void baseUrlChanged(const QString& baseUrl);
    void rawMessageReceived(const QString& rawMessage);
    void pongReceived();
    void initialized();

    void failed(const QString& reason);

public slots:
    void sendMessage(const QJsonValue& id, const QJsonValue& data = QJsonValue());
    void respond(const QJsonValue& messageId, const QJsonValue& data = QJsonValue());
    void sendRawMessage(const QString& rawMessage);
    void ping();

private slots:
    void onInitialized();

private:
    QWebChannelPrivate* d;
};

#endif // QWEBCHANNEL_H

