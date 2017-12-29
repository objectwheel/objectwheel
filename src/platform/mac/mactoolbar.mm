#include <mactoolbar.h>
#include <QMainWindow>
#include <QtMacExtras>
#import <AppKit/AppKit.h>

MacToolbar::MacToolbar(QMainWindow* mainWindow) : QObject(mainWindow)
{
    QMacToolBar* toolBar = new QMacToolBar(this);
    mainWindow->setUnifiedTitleAndToolBarOnMac(true);

    mainWindow->winId();
    toolBar->attachToWindow(mainWindow->windowHandle());

    auto nsView = (NSView*) mainWindow->winId();
    auto window = [nsView window];

    [window setTitlebarAppearsTransparent: YES];
    [window setTitleVisibility: NSWindowTitleHidden];
    [window setAppearance: [NSAppearance appearanceNamed:NSAppearanceNameAqua]];

    auto contentHeight = [window contentRectForFrameRect: window.frame].size.height;
    _toolbarHeight = window.frame.size.height - contentHeight;

    window.styleMask |= NSFullSizeContentViewWindowMask;
}

qreal MacToolbar::toolbarHeight() const
{
    return _toolbarHeight;
}
