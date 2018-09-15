#include <runmanager.h>
#include <projectmanager.h>
#include <control.h>
#include <saveutils.h>
#include <filemanager.h>

#include <QApplication>
#include <QRegularExpression>

namespace {

Control* deepest(const QList<Control*>& controls)
{
    if (controls.isEmpty())
        return nullptr;

    Control* deepest = controls.first();
    for (const auto control : controls) {
        if (control->dir() > deepest->dir())
            deepest = control;
    }
    return deepest;
}

QString polishOutput(QString str)
{
    QList<Control*> controls;

    for (const auto control : Control::controls()) {
        if (str.contains(control->dir())
                || str.contains(QRegularExpression("file:\\/{1,3}" + control->dir()))) {
            controls << control;
        }
    }

    Control* deepestControl = deepest(controls);
    if (!deepestControl)
        return str;

    // FIXME: What if the file is a global resources file?
    const QString& dir = deepestControl->dir() + separator() + DIR_THIS + separator();
    const QString& newDir = deepestControl->id() + "::" + deepestControl->uid() + ": ";
    const QRegularExpression& exp = QRegularExpression("file:\\/{1,3}" + dir);

    str.replace(exp, newDir);
    str.replace(dir, newDir);
    return str;
}
}

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
    s_process->setArguments(QStringList(ProjectManager::dir()));
    s_process->setProgram(QCoreApplication::applicationDirPath() + "/interpreter");
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
    emit standardError(polishOutput(s_process->readAllStandardError()));
    emit readyRead();
}

void RunManager::onReadyReadStandardOutput()
{
    emit standardOutput(polishOutput(s_process->readAllStandardOutput()));
    emit readyRead();
}
