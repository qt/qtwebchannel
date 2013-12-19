/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Copyright (C) 2013 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com, author Milian Wolff <milian.wolff@kdab.com>
** Contact: http://www.qt-project.org/legal
*
** This file is part of the QtCore module of the Qt Toolkit.
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

#ifndef TST_WEBCHANNEL_H
#define TST_WEBCHANNEL_H

#include <QObject>

class TestObject : public QObject
{
    Q_OBJECT
    Q_ENUMS(Foo)

    Q_PROPERTY(Foo foo READ foo CONSTANT)
    Q_PROPERTY(int asdf READ asdf NOTIFY asdfChanged)
    Q_PROPERTY(QString bar READ bar NOTIFY theBarHasChanged)
public:
    explicit TestObject(QObject *parent = 0)
        : QObject(parent)
    { }

    enum Foo {
        Bar,
        Asdf
    };

    Foo foo() const {return Bar;}
    int asdf() const {return 42;}
    QString bar() const {return QString();}

    Q_INVOKABLE void method1() {}

protected:
    Q_INVOKABLE void method2() {}

private:
    Q_INVOKABLE void method3() {}

signals:
    void sig1();
    void sig2(const QString&);
    void asdfChanged();
    void theBarHasChanged();

public slots:
    void slot1() {}
    void slot2(const QString&) {}

protected slots:
    void slot3() {}

private slots:
    void slot4() {}
};

class TestWebChannel : public QObject
{
    Q_OBJECT

public:
    explicit TestWebChannel(QObject *parent = 0);
    virtual ~TestWebChannel();

private slots:
    void testInitChannel();
    void testRegisterObjects();
    void testInfoForObject();
};

#endif // TST_WEBCHANNEL_H
