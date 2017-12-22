#include <windowmanager.h>
#include <progresswidget.h>
#include <welcomewindow.h>
#include <mainwindow.h>
#include <aboutwindow.h>

WindowManager::WindowManager()
{
    _progressWidget = new ProgressWidget;
    _mainWindow = new MainWindow;
    _aboutWindow = new AboutWindow;
    _welcomeWindow = new WelcomeWindow;


    connect(_mainWindow, SIGNAL(comfy()), _progressWidget, SLOT(hide()));
    connect(_aboutWindow, SIGNAL(comfy()), _progressWidget, SLOT(hide()));
    connect(_welcomeWindow, SIGNAL(comfy()), _progressWidget, SLOT(hide()));

//    connect(_mainWindow, SIGNAL(comfy()), _progressWidget, SLOT(hide()));
//    connect(_aboutWindow, SIGNAL(comfy()), _progressWidget, SLOT(hide()));
//    connect(_welcomeWindow, SIGNAL(busy(QString)), _progressWidget, SLOT(sho));


    add(Main, _mainWindow);
    add(About, _aboutWindow);
    add(Welcome, _welcomeWindow);
    show(Welcome);
}

WindowManager::~WindowManager()
{
    _progressWidget->setParent(nullptr);

    delete _progressWidget;
}

WindowManager* WindowManager::instance()
{
    static WindowManager instance;
    return &instance;
}

void WindowManager::show(WindowManager::Windows key)
{
    QWidget* window;
    if ((window = _windows.value(key))) {
        for (auto w : _windows) {
            if (w == window)
                w->show();
            else
                w->hide();
        }

        _progressWidget->hide();
    }
}

void WindowManager::add(WindowManager::Windows key, QWidget* window)
{
    _windows[key] = window;
}
