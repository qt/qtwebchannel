/****************************************************************************
**
** Copyright (C) 2013 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com, author Milian Wolff <milian.wolff@kdab.com>
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtWebChannel module of the Qt Toolkit.
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
** $QT_END_LICENSE$
**
****************************************************************************/

import QtQuick 2.0

import QtWebChannel 1.0

WebChannelTest {
    name: "Bench"
    id: test

    Component {
        id: component
        QtObject {
            property var p0 : 0
            property var p1 : 0
            property var p2 : 0
            property var p3 : 0
            property var p4 : 0
            property var p5 : 0
            property var p6 : 0
            property var p7 : 0
            property var p8 : 0
            property var p9 : 0
            function m0(arg1, arg2) {}
            function m1(arg1, arg2) {}
            function m2(arg1, arg2) {}
            function m3(arg1, arg2) {}
            function m4(arg1, arg2) {}
            function m5(arg1, arg2) {}
            function m6(arg1, arg2) {}
            function m7(arg1, arg2) {}
            function m8(arg1, arg2) {}
            function m9(arg1, arg2) {}
            signal s0(var arg1, var arg2)
            signal s1(var arg1, var arg2)
            signal s2(var arg1, var arg2)
            signal s3(var arg1, var arg2)
            signal s4(var arg1, var arg2)
            signal s5(var arg1, var arg2)
            signal s6(var arg1, var arg2)
            signal s7(var arg1, var arg2)
            signal s8(var arg1, var arg2)
            signal s9(var arg1, var arg2)
        }
    }

    property var objects: ({})

    function initTestCase()
    {
        for (var i = 0; i < 100; ++i) {
            var id = "obj" + i;
            var properties = {objectName: id};
            objects[id] = component.createObject(test, properties);
        }

        webChannel.registerObjects(objects);
    }

    function benchmark_init_baseline()
    {
        loadUrl("bench_init.html");
    }

    function benchmark_init()
    {
        loadUrl("bench_init.html");
        // init
        awaitMessage();
        // idle
        awaitMessage();
    }
}