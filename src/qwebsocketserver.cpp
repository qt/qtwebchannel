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

#include "qwebsocketserver.h"

#include <QTcpServer>
#include <QTcpSocket>
#include <QCryptographicHash>
#include <QtEndian>

#include <limits>

namespace {
template<typename T>
inline static void appendBytes(QByteArray& data, T value)
{
    data.append(reinterpret_cast<const char*>(&value), sizeof(value));
}

inline static void unmask(QByteArray& data, char mask[4])
{
    for (int i = 0; i < data.size(); ++i) {
        int j = i % 4;
        data[i] = data[i] ^ mask[j];
    }
}

inline static char bitMask(int bit)
{
    return 1 << bit;
}

// see: http://tools.ietf.org/html/rfc6455#page-28
static const char FIN_BIT = bitMask(7);
static const char MASKED_BIT = bitMask(7);
static const char OPCODE_RANGE = bitMask(4) - 1;
static const char PAYLOAD_RANGE = bitMask(7) - 1;
static const char EXTENDED_PAYLOAD = 126;
static const char EXTENDED_LONG_PAYLOAD = 127;
}

QWebSocketServer::QWebSocketServer(QObject* parent)
: QObject(parent)
, m_server(new QTcpServer(this))
{
    connect(m_server, SIGNAL(newConnection()),
            SLOT(newConnection()));
    connect(m_server, SIGNAL(acceptError(QAbstractSocket::SocketError)),
            SIGNAL(error(QAbstractSocket::SocketError)));
}

QWebSocketServer::~QWebSocketServer()
{
    close();
}

bool QWebSocketServer::listen(const QHostAddress& address, quint16 port)
{
    return m_server->listen(address, port);
}

void QWebSocketServer::close()
{
    sendFrame(Frame::ConnectionClose, QByteArray());
    m_server->close();
}

quint16 QWebSocketServer::port() const
{
    return m_server->serverPort();
}

QHostAddress QWebSocketServer::address() const
{
    return m_server->serverAddress();
}

QString QWebSocketServer::errorString() const
{
    return m_server->errorString();
}

void QWebSocketServer::newConnection()
{
    if (!m_server->hasPendingConnections())
        return;

    QTcpSocket* connection = m_server->nextPendingConnection();
    m_connections.insert(connection, Connection());
    connect(connection, SIGNAL(readyRead()),
            SLOT(readSocketData()));
    connect(connection, SIGNAL(error(QAbstractSocket::SocketError)),
            SIGNAL(error(QAbstractSocket::SocketError)));
    connect(connection, SIGNAL(disconnected()),
            SLOT(disconnected()));
}

void QWebSocketServer::disconnected()
{
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
    Q_ASSERT(socket);

    m_connections.remove(socket);
}

static const QByteArray headerSwitchProtocols = QByteArrayLiteral("HTTP/1.1 101 Switching Protocols");
static const QByteArray headerGet = QByteArrayLiteral("GET ");
static const QByteArray headerHTTP = QByteArrayLiteral("HTTP/1.1");
static const QByteArray headerHost = QByteArrayLiteral("Host: ");
static const QByteArray headerUpgrade = QByteArrayLiteral("Upgrade: websocket");
static const QByteArray headerConnection = QByteArrayLiteral("Connection: Upgrade");
static const QByteArray headerSecKey = QByteArrayLiteral("Sec-WebSocket-Key: ");
static const QByteArray headerSecProtocol = QByteArrayLiteral("Sec-WebSocket-Protocol: ");
static const QByteArray headerSecVersion = QByteArrayLiteral("Sec-WebSocket-Version: 13");
static const QByteArray headerSecAccept = QByteArrayLiteral("Sec-WebSocket-Accept: ");
static const QByteArray headerOrigin = QByteArrayLiteral("Origin: ");
static const QByteArray headerMagicKey = QByteArrayLiteral("258EAFA5-E914-47DA-95CA-C5AB0DC85B11");

void QWebSocketServer::readSocketData()
{
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
    Q_ASSERT(socket);

    Connection& connection = m_connections[socket];

    if (!connection.header.wasUpgraded) {
        readHeaderData(socket, connection.header);
    }

    if (connection.header.wasUpgraded) {
        while (socket->bytesAvailable()) {
            if (!readFrameData(socket, connection.currentFrame)) {
                close(socket, connection.header);
            }
        }
    }
}

void QWebSocketServer::readHeaderData(QTcpSocket* socket, HeaderData& header)
{
    while (socket->canReadLine()) {
        QByteArray line = socket->readLine().trimmed();
        if (line.isEmpty()) {
            // finalize
            if (isValid(header)) {
                upgrade(socket, header);
            } else {
                close(socket, header);
            }
            break;
        } else if (line.startsWith(headerGet) && line.endsWith(headerHTTP)) {
            header.path = line.mid(headerGet.size(), line.size() - headerGet.size() - headerHTTP.size()).trimmed();
        } else if (line.startsWith(headerHost)) {
            header.host = line.mid(headerHost.size()).trimmed();
        } else if (line.startsWith(headerSecKey)) {
            header.key = line.mid(headerSecKey.size()).trimmed();
        } else if (line.startsWith(headerOrigin)) {
            header.origin = line.mid(headerOrigin.size()).trimmed();
        } else if (line.startsWith(headerSecProtocol)) {
            header.protocol = line.mid(headerSecProtocol.size()).trimmed();
        } else if (line == headerUpgrade) {
            header.hasUpgrade = true;
        } else if (line == headerConnection) {
            header.hasConnection = true;
        } else if (line == headerSecVersion) {
            header.hasVersion = true;
        } else {
            header.otherHeaders << line;
        }
    }
}

// see: http://tools.ietf.org/html/rfc6455#page-28
bool QWebSocketServer::readFrameData(QTcpSocket* socket, Frame& frame)
{
    int bytesAvailable = socket->bytesAvailable();
    if (frame.state == Frame::ReadStart) {
        if (bytesAvailable < 2) {
            return true;
        }
        uchar buffer[2];
        socket->read(reinterpret_cast<char*>(buffer), 2);
        bytesAvailable -= 2;
        frame.fin = buffer[0] & FIN_BIT;
        // skip rsv1, rsv2, rsv3
        // last four bits are the opcode
        quint8 opcode = buffer[0] & OPCODE_RANGE;
        if (opcode != Frame::ContinuationFrame && opcode != Frame::BinaryFrame &&
            opcode != Frame::ConnectionClose && opcode != Frame::TextFrame &&
            opcode != Frame::Ping && opcode != Frame::Pong)
        {
            qWarning() << "invalid opcode: " << opcode;
            return false;
        }
        frame.opcode = static_cast<Frame::Opcode>(opcode);
        // test first, i.e. highest bit for mask
        frame.masked = buffer[1] & MASKED_BIT;
        if (!frame.masked) {
            qWarning() << "unmasked frame received";
            return false;
        }
        // final seven bits are the payload length
        frame.length = static_cast<quint8>(buffer[1] & PAYLOAD_RANGE);
        if (frame.length == EXTENDED_PAYLOAD) {
            frame.state = Frame::ReadExtendedPayload;
        } else if (frame.length == EXTENDED_LONG_PAYLOAD) {
            frame.state = Frame::ReadExtendedLongPayload;
        } else {
            frame.state = Frame::ReadMask;
        }
    }
    if (frame.state == Frame::ReadExtendedPayload) {
        if (bytesAvailable < 2) {
            return true;
        }
        uchar buffer[2];
        socket->read(reinterpret_cast<char*>(buffer), 2);
        bytesAvailable -= 2;
        frame.length = qFromBigEndian<quint16>(buffer);
        frame.state = Frame::ReadMask;
    }
    if (frame.state == Frame::ReadExtendedLongPayload) {
        if (bytesAvailable < 8) {
            return true;
        }
        uchar buffer[8];
        socket->read(reinterpret_cast<char*>(buffer), 8);
        bytesAvailable -= 8;
        quint64 longSize = qFromBigEndian<quint64>(buffer);
        // QByteArray uses int for size type so limit ourselves to that size as well
        if (longSize > static_cast<quint64>(std::numeric_limits<int>::max())) {
            return false;
        }
        frame.length = static_cast<int>(longSize);
        frame.state = Frame::ReadMask;
    }
    if (frame.state == Frame::ReadMask) {
        if (bytesAvailable < 4) {
            return true;
        }
        socket->read(frame.mask, 4);
        bytesAvailable -= 4;
        frame.state = Frame::ReadData;
        frame.data.reserve(frame.length);
    }
    if (frame.state == Frame::ReadData && bytesAvailable) {
        frame.data.append(socket->read(qMin(frame.length - frame.data.size(), bytesAvailable)));
        if (frame.data.size() == frame.length) {
            frame.state = Frame::ReadStart;
            handleFrame(socket, frame);
        }
    }
    return true;
}

void QWebSocketServer::handleFrame(QTcpSocket* socket, Frame& frame)
{
    unmask(frame.data, frame.mask);

    // fragmentation support -  see http://tools.ietf.org/html/rfc6455#page-33
    if (!frame.fin) {
        if (frame.opcode != Frame::ContinuationFrame) {
            frame.initialOpcode = frame.opcode;
        }
        frame.fragments += frame.data;
    } else if (frame.fin && frame.opcode == Frame::ContinuationFrame) {
        frame.opcode = frame.initialOpcode;
        frame.data = frame.fragments + frame.data;
    } // otherwise if it's fin and a non-continuation frame its a single-frame message

    switch (frame.opcode) {
    case Frame::ContinuationFrame:
        // do nothing
        break;
    case Frame::Ping:
        sendFrame(socket, Frame::Pong, QByteArray());
        break;
    case Frame::Pong:
        emit pongReceived();
        break;
    case Frame::ConnectionClose:
        ///TODO: handle?
        qWarning("Unhandled connection close frame");
        break;
    case Frame::BinaryFrame:
        emit binaryDataReceived(frame.data);
        break;
    case Frame::TextFrame:
        emit textDataReceived(QString::fromUtf8(frame.data));
        break;
    }

    if (frame.fin) {
        frame = Frame();
    }
}

bool QWebSocketServer::isValid(const HeaderData& header)
{
    return !header.path.isEmpty() && !header.host.isEmpty() && !header.key.isEmpty()
        && header.hasUpgrade && header.hasConnection && header.hasVersion;
}

void QWebSocketServer::close(QTcpSocket* socket, const HeaderData& header)
{
    if (header.wasUpgraded) {
        //TODO: implement this properly - see http://tools.ietf.org/html/rfc6455#page-36
        sendFrame(socket, Frame::ConnectionClose, QByteArray());
    } else {
        socket->write("HTTP/1.1 400 Bad Request\r\n");
    }
    socket->close();
}

void QWebSocketServer::upgrade(QTcpSocket* socket, HeaderData& header)
{
    socket->write(headerSwitchProtocols);
    socket->write("\r\n");

    socket->write(headerUpgrade);
    socket->write("\r\n");

    socket->write(headerConnection);
    socket->write("\r\n");

    socket->write(headerSecAccept);
    socket->write(QCryptographicHash::hash( header.key + headerMagicKey, QCryptographicHash::Sha1 ).toBase64());
    socket->write("\r\n");

    if (!header.protocol.isEmpty()) {
        socket->write(headerSecProtocol);
        socket->write(header.protocol);
        socket->write("\r\n");
    }

    socket->write("\r\n");

    header.wasUpgraded = true;
}

void QWebSocketServer::sendMessage(const QString& message)
{
    sendFrame(Frame::TextFrame, message.toUtf8());
}

void QWebSocketServer::sendFrame(Frame::Opcode opcode, const QByteArray& data)
{
    QHash< QTcpSocket*, Connection >::const_iterator it = m_connections.constBegin();
    while (it != m_connections.constEnd()) {
        if (it.value().header.wasUpgraded) {
            sendFrame(it.key(), opcode, data);
        }
        ++it;
    }
}

// see: http://tools.ietf.org/html/rfc6455#page-28
void QWebSocketServer::sendFrame(QTcpSocket* socket, Frame::Opcode opcode, const QByteArray& data)
{
    // we only support single frames for now
    Q_ASSERT(opcode != Frame::ContinuationFrame);

    QByteArray header;
    header.reserve(4);
    header.append(FIN_BIT | opcode);
    if (data.size() < EXTENDED_PAYLOAD) {
        header.append(static_cast<char>(data.size()));
    } else if (data.size() < std::numeric_limits<quint16>::max()) {
        header.append(EXTENDED_PAYLOAD);
        appendBytes(header, qToBigEndian<quint16>(data.size()));
    } else {
        header.append(EXTENDED_LONG_PAYLOAD);
        appendBytes(header, qToBigEndian<quint64>(data.size()));
    }
    socket->write(header);
    socket->write(data);
}
