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
#include "qwebsockettransport.h"

#include <QApplication>
#include <QDialog>
#include <QVariantMap>
#include <QDesktopServices>
#include <QUrl>
#include <QDebug>

#include "ui_dialog.h"

class Dialog : public QObject
{
    Q_OBJECT

public:
    explicit Dialog(QWebSocketTransport *transport, QObject *parent = 0)
        : QObject(parent)
    {
        ui.setupUi(&dialog);
        dialog.show();

        connect(ui.send, SIGNAL(clicked()), SLOT(clicked()));
        connect(transport, SIGNAL(baseUrlChanged(QString)),
                SLOT(baseUrlChanged(QString)));
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

    void baseUrlChanged(const QString &baseUrl)
    {
        ui.output->appendPlainText(tr("Initialization complete, opening browser."));

        QUrl url = QUrl::fromLocalFile(SOURCE_DIR "/index.html");
        url.setQuery(QStringLiteral("webChannelBaseUrl=") + baseUrl);
        QDesktopServices::openUrl(url);
    }

private:
    QDialog dialog;
    Ui::Dialog ui;
};

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    QWebChannel channel;
    QWebSocketTransport transport;
    channel.connectTo(&transport);

    Dialog dialog(&transport);

    channel.registerObject(QStringLiteral("dialog"), &dialog);

    return app.exec();
}

#include "main.moc"
