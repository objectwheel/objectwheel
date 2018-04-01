#include <windowmanager.h>
#include <welcomewindow.h>
#include <mainwindow.h>
#include <aboutwindow.h>
#include <preferenceswindow.h>
#include <toolboxsettingswindow.h>
#include <fit.h>
#include <savebackend.h>
#include <dpr.h>

#include <QStyle>

WindowManager::WindowManager() : _mainWindow(nullptr)
  , _welcomeWindow(nullptr)
  , _aboutWindow(nullptr)
  , _preferencesWindow(nullptr)
  , _toolboxSettingsWindow(nullptr)
{
}

WindowManager::~WindowManager()
{
    for (auto w : _windows)
        w->deleteLater(); //TODO: delete w;
}

WindowManager* WindowManager::instance()
{
    static WindowManager instance;
    return &instance;
}

QWidget* WindowManager::get(WindowManager::Windows key)
{    
    QWidget* window;
    if ((window = _windows.value(key)))
        return window;

    switch (key) {
        case Main: {
            _mainWindow = new MainWindow;
            _mainWindow->resize(fit::fx(QSizeF{1580, 900}).toSize());
            connect(_mainWindow, SIGNAL(done()), SLOT(done()));
            add(Main, _mainWindow);
            window = _mainWindow;
            break;
        }

        case About: {
            _aboutWindow = new AboutWindow;
            _aboutWindow->resize(fit::fx(QSizeF{700, 400}).toSize());
            connect(_aboutWindow, SIGNAL(done()), SLOT(done()));
            add(About, _aboutWindow);
            window = _aboutWindow;
            break;
        }

        case Welcome: {
            _welcomeWindow = new WelcomeWindow;
            _welcomeWindow->resize(fit::fx(QSizeF{1160, 670}).toSize());
            connect(_welcomeWindow, SIGNAL(done()), SLOT(done()));
            connect(_welcomeWindow, &WelcomeWindow::done, this, [=]
            {
                const auto& size = fit::fx(QSizeF{1580, 900}).toSize();
                const auto& ssize = QGuiApplication::primaryScreen()->size();
                const bool full = (size.height() + 100 > ssize.height()) ||
                                  (size.width() + 50 > ssize.width());
                show(Main, full ? Qt::WindowMaximized : Qt::WindowNoState);
            });
            add(Welcome, _welcomeWindow);
            window = _welcomeWindow;
            break;
        }

        case Preferences: {
            _preferencesWindow = new PreferencesWindow;
            _preferencesWindow->resize(fit::fx(QSizeF{1160, 670}).toSize());
            connect(_preferencesWindow, SIGNAL(done()), SLOT(done()));
            add(Preferences, _preferencesWindow);
            window = _preferencesWindow;
            break;
        }

        case ToolboxSettings: {
            _toolboxSettingsWindow = new ToolboxSettingsWindow;
            _toolboxSettingsWindow->resize(fit::fx(QSizeF{1160, 670}).toSize());
            connect(_toolboxSettingsWindow, SIGNAL(done()), SLOT(done()));
            add(ToolboxSettings, _toolboxSettingsWindow);
            window = _toolboxSettingsWindow;
            break;
        }

        default:
            break;
    }

    if (window) {
        window->setGeometry(
            QStyle::alignedRect(
                Qt::LeftToRight,
                Qt::AlignCenter,
                window->size(),
                QGuiApplication::primaryScreen()->availableGeometry()
            )
        );
    }

    return window;
}

void WindowManager::hide(WindowManager::Windows key)
{
    QWidget* window;
    if ((window = get(key)))
        window->hide();
}

void WindowManager::show(
    WindowManager::Windows key,
    Qt::WindowState state,
    Qt::WindowModality modality
)
{
    QWidget* window;
    if ((window = get(key))) {
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

void WindowManager::add(WindowManager::Windows key, QWidget* window)
{
    _windows[key] = window;
}