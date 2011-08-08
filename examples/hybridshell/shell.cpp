#include "shell.h"
#include <QtDebug>

Shell::Shell(QObject *parent) :
    QObject(parent)
{
    connect(&process, SIGNAL(readyReadStandardError()), this, SLOT(handleOutput()));
    connect(&process, SIGNAL(readyReadStandardOutput()), this, SLOT(handleOutput()));
}

void Shell::start()
{
    process.start("sh");
}

void Shell::exec(const QString& data)
{
    qWarning() << "executing" << data;
    process.write(data.toUtf8());
    process.write("\n");
}

void Shell::handleOutput()
{
    QByteArray data = process.readAllStandardOutput();
    if (!data.isEmpty())
        emit stdoutData(data);
    data = process.readAllStandardError();
    if (!data.isEmpty())
        emit stderrData(data);
}
