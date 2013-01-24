#ifndef TESTOBJECT_H
#define TESTOBJECT_H

#include <QObject>
#include <QtDebug>
#include <QTimer>
class TestObject : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString prop1 READ prop1 WRITE setProp1)
    Q_PROPERTY(QString prop2 READ prop2 WRITE setProp2)
public:
    explicit TestObject(QObject *parent = 0);
    QString prop1() const { return "p1" + p1 + objectName(); }
    void setProp1(const QString& s);

    QString prop2() const { return "p2" +  p2 + objectName(); }
    void setProp2(const QString& s);

signals:
    void timeout();

public slots:
    void startTimer(int millis)
    {
        timer.start(millis);
    }

    QString debugMe(const QString& data);

private:
    QString p1;
    QString p2;
    QTimer timer;
};

#endif // TESTOBJECT_H
