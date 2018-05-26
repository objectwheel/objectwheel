#ifndef WINDOWMANAGER_H
#define WINDOWMANAGER_H

#include <QtGlobal>

class MainWindow;
class WelcomeWindow;
class AboutWindow;
class PreferencesWindow;
class ToolboxSettingsWindow;

class WindowManager final
{
    Q_DISABLE_COPY(WindowManager)

public:
    static void init();

    static AboutWindow* aboutWindow();
    static PreferencesWindow* preferencesWindow();
    static ToolboxSettingsWindow* toolboxSettingsWindow();
    static MainWindow* mainWindow();
    static WelcomeWindow* welcomeWindow();

private:
    WindowManager();

private:
    static AboutWindow* s_aboutWindow;
    static PreferencesWindow* s_preferencesWindow;
    static ToolboxSettingsWindow* s_toolboxSettingsWindow;
    static MainWindow* s_mainWindow;
    static WelcomeWindow* s_welcomeWindow;
};

#endif // WINDOWMANAGER_H