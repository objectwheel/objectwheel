#ifndef APPLICATIONCORE_H
#define APPLICATIONCORE_H

#include <QObject>

class SignalHandler;
class RenderSocket;
class RenderEngine;
class CommandDispatcher;

class ApplicationCore final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(ApplicationCore)

public:
    explicit ApplicationCore(QObject* parent = nullptr);
    ~ApplicationCore() override;

    static QString modulesPath();

    void run();

    static void prepare();

private slots:
    void startQuitCountdown(int msec);
    void quitIfDisconnected();

private:
    SignalHandler* m_signalHandler;
    RenderSocket* m_renderSocket;
    QThread* m_socketThread;
    CommandDispatcher* m_commandDispatcher;
    RenderEngine* m_renderEngine;
};

#endif // APPLICATIONCORE_H
