#include <windowmanager.h>
#include <progresswidget.h>
#include <welcomewindow.h>
#include <mainwindow.h>
#include <aboutwindow.h>
#include <buildswindow.h>
#include <preferenceswindow.h>
#include <toolboxsettingswindow.h>

#include <QScreen>
#include <QApplication>
#define pS (QApplication::primaryScreen())

WindowManager::WindowManager()
{
    _progressWidget = new ProgressWidget;
    _mainWindow = new MainWindow;
    _aboutWindow = new AboutWindow;
    _welcomeWindow = new WelcomeWindow;
    _buildsWindow = new BuildsWindow;
    _preferencesWindow = new PreferencesWindow;
    _toolboxSettingsWindow = new ToolboxSettingsWindow;

    _mainWindow->resize(fit::fx(QSizeF{1620, 900}).toSize());
    _aboutWindow->resize(fit::fx(QSizeF{700, 400}).toSize());
    _welcomeWindow->resize(fit::fx(QSizeF{900, 550}).toSize());
    _buildsWindow->resize(fit::fx(QSizeF{900, 550}).toSize());
    _preferencesWindow->resize(fit::fx(QSizeF{900, 550}).toSize());
    _toolboxSettingsWindow->resize(fit::fx(QSizeF{900, 550}).toSize());

    connect(_welcomeWindow, SIGNAL(lazy()), _progressWidget, SLOT(hide()));
    connect(_welcomeWindow, SIGNAL(busy(QString)), SLOT(busy(QString)));
    connect(_mainWindow, SIGNAL(done()), SLOT(done()));
    connect(_aboutWindow, SIGNAL(done()), SLOT(done()));
    connect(_preferencesWindow, SIGNAL(done()), SLOT(done()));
    connect(_toolboxSettingsWindow, SIGNAL(done()), SLOT(done()));
    connect(_welcomeWindow, SIGNAL(done()), SLOT(done()));
    connect(_welcomeWindow, &WelcomeWindow::done, this, [=] {
        show(Main, Qt::WindowMaximized);
    });

    add(Main, _mainWindow);
    add(About, _aboutWindow);
    add(Welcome, _welcomeWindow);
    add(Builds, _buildsWindow);
    add(Preferences, _preferencesWindow);
    add(ToolboxSettings, _toolboxSettingsWindow);
    show(Welcome);

    for (auto w : _windows) {
        static const auto ww = _welcomeWindow;
        static const auto a = pS->availableGeometry();
        static const auto d = (ww->frameSize() - ww->size()).height() / 2.0;

        w->setGeometry(
            QStyle::alignedRect(
                Qt::LeftToRight,
                Qt::AlignCenter,
                w->size(),
                a.adjusted(0, d, 0, d)
            )
        );
    }
}

WindowManager::~WindowManager()
{
    delete _progressWidget;
    delete _mainWindow;
    delete _aboutWindow;
    delete _welcomeWindow;
    delete _buildsWindow;
    delete _preferencesWindow;
    delete _toolboxSettingsWindow;
}

WindowManager* WindowManager::instance()
{
    static WindowManager instance;
    return &instance;
}

void WindowManager::hide(WindowManager::Windows key)
{
    QWidget* window;
    if ((window = _windows.value(key)))
        window->hide();
}

void WindowManager::show(
    WindowManager::Windows key,
    Qt::WindowState state,
    Qt::WindowModality modality
    )
{
    QWidget* window;
    if ((window = _windows.value(key))) {
        window->setWindowState(state);
        window->setWindowModality(modality);
        window->show();
    }
}

void WindowManager::done()
{
    for (auto w : _windows) {
        if (w == sender()) {
            w->hide();
            break;
        }
    }
}

void WindowManager::busy(const QString& text)
{
    for (auto w : _windows) {
        if (w == sender()) {
            _progressWidget->show(text, w);
            _progressWidget->setGeometry(w->rect());
            break;
        }
    }
}

void WindowManager::add(WindowManager::Windows key, QWidget* window)
{
    _windows[key] = window;
}
