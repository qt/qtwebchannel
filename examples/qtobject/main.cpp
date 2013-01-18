#include <QtGui/QGuiApplication>

#include <QtQml>

#include "qtquick2applicationviewer.h"
#include "testobject.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    qmlRegisterType<TestObject>("Qt.labs", 1, 0, "TestObject");

    QtQuick2ApplicationViewer viewer;
    viewer.setMainQmlFile(QStringLiteral("qml/qtobject/main.qml"));
    viewer.showExpanded();

    return app.exec();
}
