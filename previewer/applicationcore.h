#ifndef APPLICATIONCORE_H
#define APPLICATIONCORE_H

#include <QObject>

class PreviewerSocket;
class Previewer;
class CommandDispatcher;
class GlobalResources;

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
    ~ApplicationCore();

private:
    static GlobalResources* s_globalResources;
    static PreviewerSocket* s_previewerSocket;
    static QThread* s_socketThread;
    static CommandDispatcher* s_commandDispatcher;
    static Previewer* s_previewer;
};

#endif // APPLICATIONCORE_H