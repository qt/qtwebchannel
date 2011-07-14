/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
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

#include <QtDeclarative/QDeclarativeItem>
#include <QTcpSocket>
#include <QPointer>

class QWebChannelPrivate;

class QWebChannelResponder : public QObject {
    Q_OBJECT

public:
    QWebChannelResponder(QTcpSocket* s);

public slots:
    void open();
    void write(const QString& data);
    void close();
    void send(const QString& data)
    {
        open();
        write(data);
        close();
    }

private:
    QPointer<QTcpSocket> socket;
};

class QWebChannel : public QDeclarativeItem
{
    Q_OBJECT
    Q_DISABLE_COPY(QWebChannel)
    Q_PROPERTY(QUrl baseUrl READ baseUrl NOTIFY baseUrlChanged)
    Q_PROPERTY(int port READ port)
    Q_PROPERTY(int maxPort READ maxPort WRITE setMaxPort)
    Q_PROPERTY(int minPort READ minPort WRITE setMinPort)
    Q_PROPERTY(bool useSecret READ useSecret WRITE setUseSecret)
    Q_PROPERTY(QStringList allowedOrigins READ allowedOrigins WRITE setAllowedOrigins)

public:
    QWebChannel(QDeclarativeItem *parent = 0);
    QUrl  baseUrl() const;
    void setUseSecret(bool);
    bool useSecret() const;
    int port() const;
    int minPort() const;
    int maxPort() const;
    void setMinPort(int);
    void setMaxPort(int);
    QStringList allowedOrigins() const;
    void setAllowedOrigins(const QStringList&);
    ~QWebChannel();

signals:
    void baseUrlChanged(const QUrl &);
    void scriptUrlChanged(const QUrl &);
    // To be able to access the object from QML, it has to be an explicit QObject* and not a subclass.
    void execute(const QString& requestData, QObject* response);
    void noPortAvailable();

public slots:
    void broadcast(const QString& id, const QString& data);

private slots:
    void onInitialized();

private:
    QWebChannelPrivate* d;
};

#endif // QWEBCHANNEL_H

