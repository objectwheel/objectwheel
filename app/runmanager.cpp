#include <runmanager.h>
#include <projectmanager.h>
#include <savemanager.h>

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
    connect(qApp, &QApplication::aboutToQuit, this, &RunManager::kill);
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
    s_process->setArguments(QStringList(ProjectManager::dir()));
    s_process->setProgram(QCoreApplication::applicationDirPath() + "/interpreter");
    s_process->start();
}

void RunManager::kill()
{
    s_process->kill();
}

void RunManager::waitForKill(int msecs)
{
    s_process->waitForFinished(msecs);
}

void RunManager::onReadyReadStandardError()
{
    emit standardErrorOutput(SaveManager::correctedKnownPaths(s_process->readAllStandardError()));
    emit readyRead();
}

void RunManager::onReadyReadStandardOutput()
{
    emit standardOutput(SaveManager::correctedKnownPaths(s_process->readAllStandardOutput()));
    emit readyRead();
}
