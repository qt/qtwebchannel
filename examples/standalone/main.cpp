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

#include "qwebchannel.h"

#include <QApplication>
#include <QDialog>
#include <QVariantMap>
#include <QDesktopServices>
#include <QUrl>
#include <QDebug>

#include <QtWebSockets/QWebSocketServer>
#include <QtWebSockets/QWebSocket>
#include <QtWebChannel/QMessagePassingInterface>

#include "ui_dialog.h"

class Dialog : public QObject
{
    Q_OBJECT

public:
    explicit Dialog(const QString &baseUrl, QObject *parent = 0)
        : QObject(parent)
    {
        ui.setupUi(&dialog);
        dialog.show();

        connect(ui.send, SIGNAL(clicked()), SLOT(clicked()));

        QUrl url = QUrl::fromLocalFile(SOURCE_DIR "/index.html");
        url.setQuery(QStringLiteral("webChannelBaseUrl=") + baseUrl);
        ui.output->appendPlainText(tr("Initialization complete, opening browser at %1.").arg(url.toDisplayString()));
        QDesktopServices::openUrl(url);
    }

signals:
    void sendText(const QString &text);

public slots:
    void receiveText(const QString &text)
    {
        ui.output->appendPlainText(tr("Received message: %1").arg(text));
    }

private slots:
    void clicked()
    {
        const QString text = ui.input->text();

        if (text.isEmpty()) {
            return;
        }

        emit sendText(text);
        ui.output->appendPlainText(tr("Sent message: %1").arg(text));

        ui.input->clear();
    }

private:
    QDialog dialog;
    Ui::Dialog ui;
};

// a wrapper around a QWebSocket which implements the QMessagePassingInterface
class Client : public QObject, public QMessagePassingInterface
{
    Q_OBJECT
    Q_INTERFACES(QMessagePassingInterface)
public:
    Client(QWebSocket *socket)
        : QObject(socket)
        , m_socket(socket)
    {
        connect(m_socket, &QWebSocket::textMessageReceived,
                this, &Client::textMessageReceived);
    }

signals:
    void textMessageReceived(const QString &message) Q_DECL_OVERRIDE;

public slots:
    qint64 sendTextMessage(const QString &message) Q_DECL_OVERRIDE
    {
        return m_socket->sendTextMessage(message);
    }

private:
    QWebSocket *m_socket;
};

// boiler plate code to connect incoming WebSockets to the WebChannel, such that they receive
// messages and can access the published objects.
class Transport : public QObject
{
    Q_OBJECT

public:
    Transport(QObject *parent = 0)
        : QObject(parent)
        , m_server(QStringLiteral("QWebChannel Standalone Example Server"), QWebSocketServer::NonSecureMode)
    {
        if (!m_server.listen(QHostAddress::LocalHost)) {
            qFatal("Failed to open web socket server.");
        }

        connect(&m_server, SIGNAL(newConnection()),
                this, SLOT(handleNewConnection()));
    }

    QString baseUrl() const
    {
        return m_server.serverUrl().toString();
    }

signals:
    void clientConnected(QMessagePassingInterface *client);

private slots:
    void handleNewConnection()
    {
        emit clientConnected(new Client(m_server.nextPendingConnection()));
    }

private:
    QWebSocketServer m_server;
};

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    Transport transport;
    QWebChannel channel;

    QObject::connect(&transport, SIGNAL(clientConnected(QMessagePassingInterface*)),
                     &channel, SLOT(connectTo(QMessagePassingInterface*)));

    Dialog dialog(transport.baseUrl());

    channel.registerObject(QStringLiteral("dialog"), &dialog);

    return app.exec();
}

#include "main.moc"
