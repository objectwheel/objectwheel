#ifndef WINDOWMANAGER_H
#define WINDOWMANAGER_H

#include <QObject>
#include <QMap>

class QWidget;
class ProgressWidget;
class MainWindow;
class WelcomeWindow;
class AboutWindow;
class BuildsWindow;
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
        ProgressWidget* progressWidget() const;

    public slots:
        void hide(Windows key);
        void show(
            Windows key,
            Qt::WindowState state = Qt::WindowNoState,
            Qt::WindowModality modality = Qt::NonModal
        );

    private slots:
        void done();
        void busy(const QString& text);

    private:
        WindowManager();
        ~WindowManager();

    private:
        void add(Windows key, QWidget* window);

    private:
        QMap<Windows, QWidget*> _windows;
        ProgressWidget* _progressWidget;
        MainWindow* _mainWindow;
        WelcomeWindow* _welcomeWindow;
        AboutWindow* _aboutWindow;
        BuildsWindow* _buildsWindow;
        PreferencesWindow* _preferencesWindow;
        ToolboxSettingsWindow* _toolboxSettingsWindow;
};

#endif // WINDOWMANAGER_H