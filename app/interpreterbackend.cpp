#include <interpreterbackend.h>
#include <projectbackend.h>
#include <QApplication>

InterpreterBackend* InterpreterBackend::s_instance = nullptr;
QProcess* InterpreterBackend::s_process = nullptr;

InterpreterBackend::InterpreterBackend(QObject* parent) : QObject(parent)
{
    s_instance = this;
    s_process = new QProcess(this);

    connect(s_process, &QProcess::readyReadStandardError,
            this, &InterpreterBackend::onReadyReadStandardError);
    connect(s_process, &QProcess::readyReadStandardOutput,
            this, &InterpreterBackend::onReadyReadStandardOutput);
    connect(s_process, qOverload<int,QProcess::ExitStatus>(&QProcess::finished),
            this, qOverload<int,QProcess::ExitStatus>(&InterpreterBackend::finished));
    connect(s_process, &QProcess::started, this, &InterpreterBackend::started);
}

InterpreterBackend::~InterpreterBackend()
{
    s_instance = nullptr;
}

InterpreterBackend* InterpreterBackend::instance()
{
    return s_instance;
}

void InterpreterBackend::run()
{
    s_process->setArguments(QStringList() << ProjectBackend::dir());
    s_process->setProgram(qApp->applicationDirPath() + "/objectwheel-interpreter");
    s_process->start();
}

void InterpreterBackend::kill()
{
    s_process->kill();
}

void InterpreterBackend::terminate()
{
    s_process->terminate();
}

void InterpreterBackend::onReadyReadStandardError()
{
    emit standardError(s_process->readAllStandardError());
    emit readyRead();
}

void InterpreterBackend::onReadyReadStandardOutput()
{
    emit standardOutput(s_process->readAllStandardOutput());
    emit readyRead();
}
