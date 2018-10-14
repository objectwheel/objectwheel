#ifndef RUNMANAGER_H
#define RUNMANAGER_H

#include <QProcess>

class RunManager final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(RunManager)

    friend class ApplicationCore;

public:
    static RunManager* instance();

    static void run();
    static void kill();
    static void waitForKill(int msecs);

private slots:
    void onReadyReadStandardError();
    void onReadyReadStandardOutput();

signals:
    void started();
    void readyRead();
    void standardErrorOutput(const QString& error);
    void standardOutput(const QString& output);
    void finished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    explicit RunManager(QObject* parent = nullptr);
    ~RunManager();

private:
    static RunManager* s_instance;
    static QProcess* s_process;
};

#endif // RUNMANAGER_H