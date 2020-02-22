#ifndef APPLICATIONCORE_H
#define APPLICATIONCORE_H

#include <QObject>
#include <cleanexit.h>

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
    CleanExit m_cleanExit;
    RenderSocket* m_renderSocket;
    QThread* m_socketThread;
    CommandDispatcher* m_commandDispatcher;
    RenderEngine* m_renderEngine;
};

#endif // APPLICATIONCORE_H