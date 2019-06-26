#ifndef APPLICATIONCORE_H
#define APPLICATIONCORE_H

#include <QObject>

class RenderSocket;
class RenderEngine;
class CommandDispatcher;

class ApplicationCore final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(ApplicationCore)

public:
    static void init(QObject* parent);
    static void prepare();

private slots:
    void startQuitCountdown(int msec);

    void onTerminateCommand();

private:
    static void quitIfDisconnected();

private:
    explicit ApplicationCore(QObject* parent = nullptr);
    ~ApplicationCore() override;

private:
    static RenderSocket* s_renderSocket;
    static QThread* s_socketThread;
    static CommandDispatcher* s_commandDispatcher;
    static RenderEngine* s_renderEngine;
};

#endif // APPLICATIONCORE_H