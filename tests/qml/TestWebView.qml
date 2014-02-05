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

import QtQuick 2.0
import QtTest 1.0

import QtWebKit 3.0
import QtWebKit.experimental 1.0

WebView {
    id: view
    property var lastLoadStatus

    experimental.preferences.developerExtrasEnabled: true
    experimental.preferences.navigatorQtObjectEnabled: true

    onLoadingChanged: {
        // NOTE: we cannot use spy.signalArguments nor save the loadRequest anywhere, as it gets
        // deleted after the slots connected to the signal have finished... i.e. it's a weak pointer,
        // not a shared pointer. As such, we have to copy out the interesting data we need later on here...
        lastLoadStatus = loadRequest.status
    }

    SignalSpy {
        id: loadingSpy
        target: view
        signalName: "onLoadingChanged"
    }

    function waitForLoaded()
    {
        do {
            loadingSpy.wait(500);
        } while (loading);
        return lastLoadStatus == WebView.LoadSucceededStatus;
    }

    function clear()
    {
        url = "";
        loadingSpy.clear()
    }
}
