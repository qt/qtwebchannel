/****************************************************************************
**
** Copyright (C) 2016 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com, author Milian Wolff <milian.wolff@kdab.com>
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWebChannel module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:COMM$
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
******************************************************************************/

#include <qqml.h>
#include <QtQml/QQmlExtensionPlugin>

#include <qqmlwebchannel.h>
#include <qqmlwebchannelattached_p.h>

QT_BEGIN_NAMESPACE

class QWebChannelPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QQmlExtensionInterface_iid)

public:
    QWebChannelPlugin(QObject *parent = 0) : QQmlExtensionPlugin(parent) { }
    void registerTypes(const char *uri) override;
};

void QWebChannelPlugin::registerTypes(const char *uri)
{
    int major = 1;
    int minor = 0;
    qmlRegisterType<QQmlWebChannel>(uri, major, minor, "WebChannel");

    // The minor version used to be the current Qt 5 minor. For compatibility it is the last
    // Qt 5 release.
    qmlRegisterModule(uri, major, 15);
}

QT_END_NAMESPACE

#include "plugin.moc"
