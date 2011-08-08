#ifndef SHELL_H
#define SHELL_H

#include <QObject>
#include <QProcess>
class Shell : public QObject
{
    Q_OBJECT
public:
    explicit Shell(QObject *parent = 0);

signals:
    void stdoutData(const QString& data);
    void stderrData(const QString& data);

public slots:
    void start();
    void exec(const QString& data);

private slots:
    void handleOutput();

private:
    QProcess process;
};

#endif // SHELL_H
