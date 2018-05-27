#ifndef INTERPRETERBACKEND_H
#define INTERPRETERBACKEND_H

#include <QProcess>

class InterpreterBackend final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(InterpreterBackend)

    friend class BackendManager;

public:
    static InterpreterBackend* instance();

    static void run();
    static void kill();
    static void terminate();

private slots:
    void onReadyReadStandardError();
    void onReadyReadStandardOutput();

signals:
    void started();
    void readyRead();
    void standardError(const QString& error);
    void standardOutput(const QString& output);
    void finished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    explicit InterpreterBackend(QObject* parent = nullptr);
    ~InterpreterBackend();

private:
    static InterpreterBackend* s_instance;
    static QProcess* s_process;
};

#endif // INTERPRETERBACKEND_H