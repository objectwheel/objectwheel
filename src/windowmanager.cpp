#include <windowmanager.h>
#include <progresswidget.h>

WindowManager::WindowManager()
{
    _progressWidget = new ProgressWidget;
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

void WindowManager::busy(const QString& text)
{
    _progressWidget->show();
    _progressWidget->raise();
    _progressWidget->setGeometry(
      _progressWidget->parentWidget()->rect());
}

void WindowManager::busy(WindowManager::Windows key, const QString& text)
{
    QWidget* window;
    if ((window = _windows.value(key))) {
        _progressWidget->setParent(window);
        busy();
    }
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

        _current = key;
        _progressWidget->hide();
    }
}

void WindowManager::add(WindowManager::Windows key, QWidget* window)
{
    _windows[key] = window;
}

WindowManager::Windows WindowManager::current() const
{
    return _current;
}
