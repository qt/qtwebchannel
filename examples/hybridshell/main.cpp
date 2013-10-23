#include <QtGui/QGuiApplication>

#include <QtQml>

#include "qtquick2applicationviewer.h"

#include "shell.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    qmlRegisterType<Shell>("Qt.labs", 1, 0, "HybridShell");

    QtQuick2ApplicationViewer viewer;
    viewer.setMainQmlFile(app.applicationDirPath() + QStringLiteral("/qml/hybridshell/main.qml"));
    viewer.showExpanded();

    return app.exec();
}
