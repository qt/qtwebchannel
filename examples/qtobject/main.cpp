#include <QtGui/QApplication>
#include "qmlapplicationviewer.h"
#include <qdeclarative.h>
#include "qtmetaobjectpublisher.h"
#include "testobject.h"
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    qmlRegisterType<QtMetaObjectPublisher>("Qt.labs", 1, 0, "QtMetaObjectPublisher");
    qmlRegisterType<TestObject>("Qt.labs", 1, 0, "TestObject");

    QmlApplicationViewer viewer;
    viewer.setOrientation(QmlApplicationViewer::ScreenOrientationAuto);
    viewer.setMainQmlFile(QLatin1String("qml/qtobject/main.qml"));
    viewer.showExpanded();

    return app.exec();
}
