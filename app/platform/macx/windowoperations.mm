#include <windowoperations.h>

#include <QMainWindow>

#import <AppKit/AppKit.h>

namespace WindowOperations {

void removeTitleBar(QMainWindow* mainWindow)
{
    mainWindow->setUnifiedTitleAndToolBarOnMac(true);
    auto nsView = (NSView*) mainWindow->winId();
    auto window = [nsView window];
    auto toolBar = [[NSToolbar alloc] initWithIdentifier:@"ScreenNameToolbarIdentifier"];
    toolBar.showsBaselineSeparator = false;
    [window setToolbar: toolBar];
    [window setTitleVisibility: NSWindowTitleHidden];
    window.movable = false;
    window.styleMask |= NSWindowStyleMaskFullSizeContentView;
}

}
