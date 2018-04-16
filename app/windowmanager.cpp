#include <windowmanager.h>
#include <welcomewindow.h>
#include <mainwindow.h>
#include <aboutwindow.h>
#include <preferenceswindow.h>
#include <toolboxsettingswindow.h>
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
            _mainWindow->resize({1580, 900});
            connect(_mainWindow, SIGNAL(done()), SLOT(done()));
            add(Main, _mainWindow);
            window = _mainWindow;
            break;
        }

        case About: {
            _aboutWindow = new AboutWindow;
            _aboutWindow->resize({700, 400});
            connect(_aboutWindow, SIGNAL(done()), SLOT(done()));
            add(About, _aboutWindow);
            window = _aboutWindow;
            break;
        }

        case Welcome: {
            _welcomeWindow = new WelcomeWindow;
            _welcomeWindow->resize({1160, 670});
            connect(_welcomeWindow, SIGNAL(done()), SLOT(done()));
            connect(_welcomeWindow, &WelcomeWindow::done, this, [=]
            {
                const auto& size = QSize{1580, 900};
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
            _preferencesWindow->resize({1160, 670});
            connect(_preferencesWindow, SIGNAL(done()), SLOT(done()));
            add(Preferences, _preferencesWindow);
            window = _preferencesWindow;
            break;
        }

        case ToolboxSettings: {
            _toolboxSettingsWindow = new ToolboxSettingsWindow;
            _toolboxSettingsWindow->resize({1160, 670});
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
