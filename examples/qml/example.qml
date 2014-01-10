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

import QtQuick 2.1

import QtWebChannel 1.0

import QtQuick.Controls 1.0
import QtQuick.Layouts 1.0

import QtWebKit 3.0
import QtWebKit.experimental 1.0

ApplicationWindow {
    id: window
    title: "QtWebChannel Example: QML Server to QtWebKit WebView Client"
    width: 600
    height: 400

    WebChannel {
        id: webChannel

        connections: WebViewTransport {
            webViewExperimental: webView.experimental
            onMessageReceived: {
                textEdit.text += "Received message: " + message + "\n";
            }
        }
    }

    RowLayout {
        id: myRow
        anchors.fill: parent
        ColumnLayout {
            id: myCol
            Label {
                id: caption
                text: "QML Server"
                font.bold: true
            }
            TextArea {
                Layout.fillHeight: true
                Layout.fillWidth: true
                id: textEdit
                readOnly: true
            }
            RowLayout {
                Label {
                    id: label
                    text: "Input: "
                }
                TextField {
                    id: input
                    Layout.fillWidth: true
                }
                Button {
                    id: send
                    text: "Send"
                    onClicked: {
                        if (input.text) {
                            webChannel.sendMessage("message", input.text);
                            textEdit.text += "Sent message: " + input.text + "\n";
                            input.text = ""
                        }
                    }
                }
            }
        }
        WebView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.minimumWidth: window.width / 2
            id: webView
            url: "index.html"
            experimental.preferences.developerExtrasEnabled: true
            experimental.preferences.navigatorQtObjectEnabled: true
        }
    }
}
