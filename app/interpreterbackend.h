#ifndef INTERPRETERBACKEND_H
#define INTERPRETERBACKEND_H

#include <QProcess>

class InterpreterBackend : public QObject
{
        Q_OBJECT
        Q_DISABLE_COPY(InterpreterBackend)

    public:
        static InterpreterBackend* instance();

    public slots:
        void run();
        void kill();
        void terminate();

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
        InterpreterBackend();

    private:
        QProcess* _process;
};

#endif // INTERPRETERBACKEND_H