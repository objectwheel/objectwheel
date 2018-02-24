#include <interpreterbackend.h>
#include <projectbackend.h>

InterpreterBackend::InterpreterBackend()
{
    _process = new QProcess(this);

    connect(_process, SIGNAL(readyReadStandardError()),
      SLOT(onReadyReadStandardError()));
    connect(_process, SIGNAL(readyReadStandardOutput()),
      SLOT(onReadyReadStandardOutput()));
    connect(_process,SIGNAL(finished(int,QProcess::ExitStatus)),
      SIGNAL(finished(int,QProcess::ExitStatus)));
    connect(_process,SIGNAL(started()),
      SIGNAL(started()));
}

InterpreterBackend* InterpreterBackend::instance()
{
    static InterpreterBackend instance;
    return &instance;
}

void InterpreterBackend::run()
{
    _process->setArguments(QStringList() << ProjectBackend::instance()->dir());
    _process->setProgram("./objectwheel-interpreter");
    _process->start();
}

void InterpreterBackend::kill()
{
    _process->kill();
}

void InterpreterBackend::terminate()
{
    _process->terminate();
}

void InterpreterBackend::onReadyReadStandardError()
{
    emit standardError(_process->readAllStandardError());
    emit readyRead();
}

void InterpreterBackend::onReadyReadStandardOutput()
{
    emit standardOutput(_process->readAllStandardOutput());
    emit readyRead();
}
