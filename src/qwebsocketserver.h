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

#ifndef QWEBSOCKET_H
#define QWEBSOCKET_H

#include <QObject>
#include <QHostAddress>

class QTcpServer;
class QTcpSocket;

class QWebSocketServer : public QObject
{
    Q_OBJECT

public:
    explicit QWebSocketServer(QObject* parent = 0);

    bool listen(const QHostAddress& address = QHostAddress::LocalHost, quint16 port = 0);
    void close();

    QHostAddress address() const;
    quint16 port() const;

    QString errorString() const;

signals:
    void opened();
    void error(QAbstractSocket::SocketError);
    void textDataReceived(const QString& data);
    void binaryDataReceived(const QByteArray& data);
    void pongReceived();

public slots:
    void sendMessage(const QString& message);

private slots:
    void newConnection();
    void readSocketData();
    void disconnected();

protected:
    struct HeaderData
    {
        HeaderData()
        : hasVersion(false)
        , hasUpgrade(false)
        , hasConnection(false)
        , wasUpgraded(false)
        {
        }
        QByteArray path;
        QByteArray host;
        QByteArray origin;
        QByteArray key;
        QByteArray protocol;
        QVector<QByteArray> otherHeaders;
        // no bitmap here - we only have few of these objects
        bool hasVersion;
        bool hasUpgrade;
        bool hasConnection;
        bool wasUpgraded;
    };
    struct Frame
    {
        enum State {
            ReadStart,
            ReadExtendedPayload,
            ReadExtendedLongPayload,
            ReadMask,
            ReadData,
            Finished
        };
        enum Opcode {
            ContinuationFrame = 0x0,
            TextFrame = 0x1,
            BinaryFrame = 0x2,
            ConnectionClose = 0x8,
            Ping = 0x9,
            Pong = 0xA
        };
        // no bitmap here - we only have a few of these objects
        State state;
        Opcode opcode;
        bool fin;
        bool masked;
        ///NOTE: standard says unsigned 64bit integer but QByteArray only supports 'int' size
        int length;
        char mask[4];
        QByteArray data;
        // fragmentation support
        Opcode initialOpcode;
        QByteArray fragments;
    };
    struct Connection
    {
        HeaderData header;
        Frame currentFrame;
    };

    virtual bool isValid(const HeaderData& connection);

private:
    void readHeaderData(QTcpSocket* socket, HeaderData& header);
    void close(QTcpSocket* socket, const HeaderData& header);
    void upgrade(QTcpSocket* socket, HeaderData& header);
    bool readFrameData(QTcpSocket* socket, Frame& frame);
    void handleFrame(QTcpSocket* socket, Frame& frame);

    void sendFrame(Frame::Opcode opcode, const QByteArray& data);
    void sendFrame(QTcpSocket* socket, Frame::Opcode opcode, const QByteArray& data);

    QTcpServer* m_server;
    QHash<QTcpSocket*, Connection> m_connections;
};

#endif // QWEBSOCKET_H
