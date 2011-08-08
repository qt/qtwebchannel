#include <QtGui/QApplication>
#include "qmlapplicationviewer.h"
#include "shell.h"
#include <qdeclarative.h>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    qmlRegisterType<Shell>("Qt.labs", 1, 0, "HybridShell");

    QmlApplicationViewer viewer;
    viewer.setOrientation(QmlApplicationViewer::ScreenOrientationAuto);
    viewer.setMainQmlFile(QLatin1String("qml/hybridshell/main.qml"));
    viewer.showExpanded();

    return app.exec();
}
