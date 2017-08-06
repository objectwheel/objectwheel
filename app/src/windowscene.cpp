#include <windowscene.h>

WindowScene::WindowScene(QObject *parent)
    : ControlScene(parent)
{
}

void WindowScene::addWindow(Window* window)
{
    if (_windows.contains(window))
        return;

    addItem(window);
    window->setVisible(false);

    _windows.append(window);

    if (!_mainControl)
        setMainControl(window);
}

void WindowScene::removeWindow(Window* window)
{
    if (_windows.contains(window) == false ||
        window->isMain())
        return;

    removeItem(window);
    _windows.removeOne(window);

    if (_mainControl == window)
        setMainControl(_windows[0]);
}

Window* WindowScene::mainWindow()
{
    return (Window*)_mainControl.data();
}

void WindowScene::setMainWindow(Window* mainWindow)
{
    if (_windows.contains(mainWindow) == false ||
        _mainControl == mainWindow)
        return;

    if (_mainControl)
        _mainControl->setVisible(false);

    _mainControl = mainWindow;
    _mainControl->setVisible(true);

    nonGuiControlsPanel()->clear();
    for (auto control : mainWindow->childControls())
        if (control->gui() == false)
            nonGuiControlsPanel()->addControl(control);
}

void WindowScene::setMainControl(Control* mainControl)
{
    Window* window;
    if ((window = dynamic_cast<Window*>(mainControl)))
        setMainWindow(window);
}
