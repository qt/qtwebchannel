#ifndef TESTOBJECT_H
#define TESTOBJECT_H

#include <QObject>
#include <QtDebug>
#include <QTimer>
class TestObject : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString prop1 READ prop1 WRITE setProp1 NOTIFY prop1Changed)
    Q_PROPERTY(QString prop2 READ prop2 WRITE setProp2 NOTIFY prop2Changed)
public:
    explicit TestObject(QObject *parent = 0);
    QString prop1() const { return "p1" + p1 + objectName(); }
    void setProp1(const QString& s);

    QString prop2() const { return "p2" +  p2 + objectName(); }
    void setProp2(const QString& s);

signals:
    void timeout();
    void sig1(int a, float b, const QString& c);
    void sig2();
    void prop1Changed();
    void prop2Changed(const QString& newValue);

public slots:
    void startTimer(int millis)
    {
        timer.start(millis);
    }

    QString debugMe(const QString& data);

    QString manyArgs(int a, float b, const QString& c) const;

private:
    QString p1;
    QString p2;
    QTimer timer;
};

#endif // TESTOBJECT_H
