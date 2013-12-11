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
    virtual ~QWebSocketServer();

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
    void sendMessage(const QByteArray& message) const;
    void ping() const;

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
    virtual bool isValid(const HeaderData& connection);

private:
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

    void readHeaderData(QTcpSocket* socket, HeaderData& header);
    void close(QTcpSocket* socket, const HeaderData& header);
    void upgrade(QTcpSocket* socket, HeaderData& header);
    bool readFrameData(QTcpSocket* socket, Frame& frame);
    void handleFrame(QTcpSocket* socket, Frame& frame);

    void sendFrame(Frame::Opcode opcode, const QByteArray& data) const;
    void sendFrame(QTcpSocket* socket, Frame::Opcode opcode, const QByteArray& data) const;
    QByteArray frameHeader(Frame::Opcode opcode, const int dataSize) const;

    QTcpServer* m_server;
    QHash<QTcpSocket*, Connection> m_connections;
};

#endif // QWEBSOCKET_H
