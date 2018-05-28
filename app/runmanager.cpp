#include <runmanager.h>
#include <projectmanager.h>
#include <QApplication>

RunManager* RunManager::s_instance = nullptr;
QProcess* RunManager::s_process = nullptr;

RunManager::RunManager(QObject* parent) : QObject(parent)
{
    s_instance = this;
    s_process = new QProcess(this);

    connect(s_process, &QProcess::readyReadStandardError,
            this, &RunManager::onReadyReadStandardError);
    connect(s_process, &QProcess::readyReadStandardOutput,
            this, &RunManager::onReadyReadStandardOutput);
    connect(s_process, qOverload<int,QProcess::ExitStatus>(&QProcess::finished),
            this, qOverload<int,QProcess::ExitStatus>(&RunManager::finished));
    connect(s_process, &QProcess::started, this, &RunManager::started);
}

RunManager::~RunManager()
{
    s_instance = nullptr;
}

RunManager* RunManager::instance()
{
    return s_instance;
}

void RunManager::run()
{
    s_process->setArguments(QStringList() << ProjectManager::dir());
    s_process->setProgram(qApp->applicationDirPath() + "/objectwheel-interpreter");
    s_process->start();
}

void RunManager::kill()
{
    s_process->kill();
}

void RunManager::terminate()
{
    s_process->terminate();
}

void RunManager::onReadyReadStandardError()
{
    emit standardError(s_process->readAllStandardError());
    emit readyRead();
}

void RunManager::onReadyReadStandardOutput()
{
    emit standardOutput(s_process->readAllStandardOutput());
    emit readyRead();
}
