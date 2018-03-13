#ifndef WINDOWMANAGER_H
#define WINDOWMANAGER_H

#include <QObject>
#include <QMap>

class QWidget;
class MainWindow;
class WelcomeWindow;
class AboutWindow;
class PreferencesWindow;
class ToolboxSettingsWindow;

class WindowManager : public QObject
{
        Q_OBJECT
        Q_DISABLE_COPY(WindowManager)

    public:
        enum Windows {
            Welcome,
            Main,
            About,
            Builds,
            Preferences,
            ToolboxSettings
        };

    public:
        static WindowManager* instance();
        QWidget* get(Windows key);

    public slots:
        void hide(Windows key);
        void show(
            Windows key,
            Qt::WindowState state = Qt::WindowNoState,
            Qt::WindowModality modality = Qt::NonModal
        );

    private slots:
        void done();

    private:
        WindowManager();
        ~WindowManager();

    private:
        void add(Windows key, QWidget* window);

    private:
        QMap<Windows, QWidget*> _windows;
        MainWindow* _mainWindow;
        WelcomeWindow* _welcomeWindow;
        AboutWindow* _aboutWindow;
        PreferencesWindow* _preferencesWindow;
        ToolboxSettingsWindow* _toolboxSettingsWindow;
};

#endif // WINDOWMANAGER_H