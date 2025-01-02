// Copyright (C) 2016 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com, author Milian Wolff <milian.wolff@kdab.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

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
