#include <windowmanager.h>
#include <progresswidget.h>
#include <welcomewindow.h>
#include <mainwindow.h>
#include <aboutwindow.h>

#include <QScreen>
#include <QApplication>
#define pS (QApplication::primaryScreen())

WindowManager::WindowManager()
{
    _progressWidget = new ProgressWidget;
    _mainWindow = new MainWindow;
    _aboutWindow = new AboutWindow;
    _welcomeWindow = new WelcomeWindow;

    _mainWindow->resize(fit::fx(QSizeF{1620, 900}).toSize());
    _aboutWindow->resize(fit::fx(QSizeF{700, 400}).toSize());
    _welcomeWindow->resize(fit::fx(QSizeF{900, 550}).toSize());

    connect(_mainWindow, SIGNAL(lazy()), _progressWidget, SLOT(hide()));
    connect(_welcomeWindow, SIGNAL(lazy()), _progressWidget, SLOT(hide()));
    connect(_mainWindow, SIGNAL(busy(QString)), SLOT(busy(QString)));
    connect(_welcomeWindow, SIGNAL(busy(QString)), SLOT(busy(QString)));
    connect(_mainWindow, SIGNAL(done()), SLOT(done()));
    connect(_aboutWindow, SIGNAL(done()), SLOT(done()));
    connect(_welcomeWindow, SIGNAL(done()), SLOT(done()));
    connect(_welcomeWindow, SIGNAL(done()), _mainWindow, SLOT(show()));

    add(Main, _mainWindow);
    add(About, _aboutWindow);
    add(Welcome, _welcomeWindow);
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

void WindowManager::show(WindowManager::Windows key)
{
    QWidget* window;
    if ((window = _windows.value(key)))
        window->show();
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
            _progressWidget->setParent(w);
            _progressWidget->setGeometry(w->rect());
            _progressWidget->show(text);
            break;
        }
    }
}

void WindowManager::add(WindowManager::Windows key, QWidget* window)
{
    _windows[key] = window;
}
