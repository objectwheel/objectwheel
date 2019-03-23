#ifndef RUNCONTROLLER_H
#define RUNCONTROLLER_H

#include <QObject>
#include <QProcess>

class RunPane;
struct InterfaceSettings;

class RunController final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(RunController)

    enum MessageKind {
        Welcome,
        Starting,
        Failure,
        Running,
        Crashed,
        Stopped,
        Finished
    };

public:
    explicit RunController(RunPane* m_runPane, QObject* parent = nullptr);

public slots:
    void discharge();

private slots:
    void onInterfaceSettingsChange();
    void onRunButtonClick();
    void onStopButtonClick();
    void onProcessStart();
    void onProcessFinish(int exitCode, QProcess::ExitStatus exitStatus);
    void onProcessErrorOccur(QProcess::ProcessError error, const QString& errorString);

private:
    static QString progressBarMessageFor(MessageKind kind, const QString& arg = QString());

signals:
    void ran();

private:
    bool m_appManuallyTerminated;
    RunPane* m_runPane;
};

#endif // RUNCONTROLLER_H