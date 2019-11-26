#ifndef WINDOWMANAGER_H
#define WINDOWMANAGER_H

#include <QObject>

class MainWindow;
class WelcomeWindow;
class AboutWindow;
class PreferencesWindow;

class WindowManager final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(WindowManager)

    friend class ApplicationCore;

public:
    static AboutWindow* aboutWindow();
    static PreferencesWindow* preferencesWindow();
    static MainWindow* mainWindow();
    static WelcomeWindow* welcomeWindow();

private:
    explicit WindowManager(QObject* parent = nullptr);
    ~WindowManager() override;

private:
    static AboutWindow* s_aboutWindow;
    static PreferencesWindow* s_preferencesWindow;
    static MainWindow* s_mainWindow;
    static WelcomeWindow* s_welcomeWindow;
};

#endif // WINDOWMANAGER_H