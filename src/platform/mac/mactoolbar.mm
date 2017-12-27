#include <mactoolbar.h>
#include <QMainWindow>
#include <QtMacExtras>
#include <QTimer>

#import <Cocoa/Cocoa.h>
#import <QuartzCore/QuartzCore.h>
#import <QuartzCore/CoreAnimation.h>

#define INTERVAL 1000

MacToolbar::MacToolbar(QMainWindow* mainWindow)
    : QObject(mainWindow)
    , _toolbarHeight(0.0)
{
    _timer = new QTimer(this);
    _timer->start(INTERVAL);
    connect(_timer, SIGNAL(timeout()), SLOT(hideTitlebar()));

    QMacToolBar* toolBar = new QMacToolBar(mainWindow);
    QMacToolBarItem* toolBarItem = toolBar->addItem(QIcon("/Users/omergoktas/Desktop/Kara Kutu/keyword.png"), QStringLiteral("foo"));
    connect(toolBarItem, SIGNAL(activated()), mainWindow, SLOT(fooClicked()));

    mainWindow->windowHandle();
    mainWindow->window()->winId(); // create window->windowhandle()
    toolBar->attachToWindow(mainWindow->window()->windowHandle());

    NSView* mainWindowNSView = (NSView*) mainWindow->window()->winId();
    NSWindow* window = [mainWindowNSView window];

    [window setTitleVisibility: NSWindowTitleHidden];
    [window setAppearance: [NSAppearance appearanceNamed:NSAppearanceNameAqua]];
    [window setTitlebarAppearsTransparent: YES];

    float toolbarHeight = 0.0;
    NSRect windowFrame;
    windowFrame = [NSWindow contentRectForFrameRect:[window frame] styleMask:[window styleMask]];
    toolbarHeight = NSHeight(windowFrame) - NSHeight([[window contentView] frame]);

    window.styleMask |= NSFullSizeContentViewWindowMask;

//    CAGradientLayer* layer;
//    [ mainWindowNSView setLayer:[CALayer layer]];
//    [ mainWindowNSView setWantsLayer: YES];
//    layer = [CAGradientLayer layer];
//    layer.frame = CGRectMake(0, 0, mainWindowNSView.frame.size.width, toolbarHeight + 22);
//    layer.colors = @[ (id)CGColorCreateGenericRGB(0.086, 0.509, 0.984, 1), (id)CGColorCreateGenericRGB(0.086, 0.309, 0.784, 1)];
//    layer.locations = @[@0.0, @1.0];
//    [mainWindowNSView.layer addSublayer:layer];
//    [layer setNeedsDisplay];
    //    [mainWindowNSView setNeedsDisplay:YES];
}

void MacToolbar::hideTitlebar()
{

}
