#ifndef SIGNALWATCHER_H
#define SIGNALWATCHER_H

#include <atomic>
#include <QObject>

class QLocalServer;
class QLocalSocket;

class SignalWatcher final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(SignalWatcher)

public:
    static SignalWatcher* instance();

private:
    explicit SignalWatcher(QObject* parent = nullptr);

private slots:
    void onSocketReadyRead();

private:
    static void handleSignal(int signal);

signals:
    void signal(int signal);

private:
    static QLocalServer* s_server;
    static QLocalSocket* s_socket;
    static std::atomic<qintptr> s_socketDescriptor;
};

#endif // SIGNALWATCHER_H
