#ifndef RUNCONTROLLER_H
#define RUNCONTROLLER_H

#include <QProcess>

class QAction;
class RunPane;

class RunController final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(RunController)

    enum MessageKind {
        Welcome,
        Starting,
        Failure,
        Disconnected,
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
    void onProjectsButtonClick();
    void onPreferencesButtonClick();
    void onRunButtonClick();
    void onStopButtonClick();
    void onSegmentedBarActionTrigger(QAction* action);

    void onApplicationStart();
    void onApplicationErrorOccur(QProcess::ProcessError error, const QString& errorString);
    void onApplicationFinish(int exitCode, QProcess::ExitStatus exitStatus);
    void onApplicationUploadProgress(int progress);

    void onDeviceConnect(const QString& uid);
    void onDeviceDisconnect(const QVariantMap& deviceInfo);

private:
    static QString progressBarMessageFor(MessageKind kind, const QString& arg = QString());

signals:
    void ran();
    void segmentedBarActionTriggered(int index, bool checked);

private:
    bool m_runScheduled;
    bool m_appManuallyTerminated;
    RunPane* m_runPane;
};

#endif // RUNCONTROLLER_H