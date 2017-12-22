#ifndef WINDOWMANAGER_H
#define WINDOWMANAGER_H

#include <QWidget>
#include <QMap>

class ProgressWidget;
class MainWindow;
class WelcomeWindow;
class AboutWindow;

class WindowManager : public QObject
{
        Q_OBJECT
        Q_DISABLE_COPY(WindowManager)

    public:
        enum Windows {
            Welcome,
            Main,
            About
        };

    public:
        static WindowManager* instance();

    public slots:
        void hide(Windows key);
        void show(Windows key);

    private slots:
        void done();
        void busy(const QString& text);

    private:
        WindowManager();
        ~WindowManager();
        void add(Windows key, QWidget* window);

    private:
        QMap<Windows, QWidget*> _windows;
        ProgressWidget* _progressWidget;
        MainWindow* _mainWindow;
        WelcomeWindow* _welcomeWindow;
        AboutWindow* _aboutWindow;
};

#endif // WINDOWMANAGER_H