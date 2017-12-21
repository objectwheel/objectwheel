#ifndef WINDOWMANAGER_H
#define WINDOWMANAGER_H

#include <QWidget>
#include <QMap>

class ProgressWidget;

class WindowManager
{
        Q_DISABLE_COPY(WindowManager)

    public:
        enum Windows {
            Login,
            Projects,
            Studio,
        };

    public:
        static WindowManager* instance();
        void busy(const QString& text);
        void busy(Windows key, const QString& text);
        void show(Windows key);
        void add(Windows key, QWidget* window);
        Windows current() const;

    private:
        WindowManager();
        ~WindowManager();

    private:
        QMap<Windows, QWidget*> _windows;
        ProgressWidget* _progressWidget;
        Windows _current;
};

#endif // WINDOWMANAGER_H