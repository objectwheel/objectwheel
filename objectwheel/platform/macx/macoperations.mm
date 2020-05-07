#include <macoperations.h>

#include <QMainWindow>

#import <AppKit/AppKit.h>

namespace MacOperations {

void disableAppNap(const QString& reason)
{
    auto processInfo = [NSProcessInfo processInfo];
    if ([processInfo respondsToSelector: @selector(beginActivityWithOptions:reason:)]) {
        [processInfo beginActivityWithOptions: NSActivityBackground reason: reason.toNSString()];
    }
}

void removeTitleBar(QMainWindow* mainWindow)
{
    auto nsView = (NSView*) mainWindow->winId();
    auto window = [nsView window];
    auto close = [window standardWindowButton: NSWindowCloseButton];
    auto minimize = [window standardWindowButton: NSWindowMiniaturizeButton];
    auto maximize = [window standardWindowButton: NSWindowZoomButton];
    auto container = close.superview;

    window.movable = false;
    window.titlebarAppearsTransparent = true;
    window.styleMask |= NSWindowStyleMaskFullSizeContentView;
    [window setTitleVisibility: NSWindowTitleHidden];

    container.translatesAutoresizingMaskIntoConstraints = false;
    [container.superview willRemoveSubview: container];
    [container removeFromSuperview];
    [container viewWillMoveToSuperview: window.contentView];
    [window.contentView addSubview: container];
    [container viewDidMoveToSuperview];

    close.translatesAutoresizingMaskIntoConstraints = false;
    [close.widthAnchor constraintEqualToConstant: 14].active = true;
    [close.heightAnchor constraintEqualToConstant: 14].active = true;
    [close.leadingAnchor constraintEqualToAnchor: container.leadingAnchor constant: 0.0].active = true;
    [close.topAnchor constraintEqualToAnchor: container.topAnchor constant: 0.0].active = true;

    minimize.translatesAutoresizingMaskIntoConstraints = false;
    [minimize.widthAnchor constraintEqualToConstant: 14].active = true;
    [minimize.heightAnchor constraintEqualToConstant: 14].active = true;
    [minimize.leadingAnchor constraintEqualToAnchor: close.trailingAnchor constant: 6.0].active = true;
    [minimize.topAnchor constraintEqualToAnchor: container.topAnchor constant: 0.0].active = true;

    maximize.translatesAutoresizingMaskIntoConstraints = false;
    [maximize.widthAnchor constraintEqualToConstant: 14].active = true;
    [maximize.heightAnchor constraintEqualToConstant: 14].active = true;
    [maximize.leadingAnchor constraintEqualToAnchor: minimize.trailingAnchor constant: 6.0].active = true;
    [maximize.topAnchor constraintEqualToAnchor: container.topAnchor constant: 0.0].active = true;

    [container.widthAnchor constraintEqualToConstant: 54].active = true;
    [container.heightAnchor constraintEqualToConstant: 14].active = true;
    [container.leadingAnchor constraintEqualToAnchor: window.contentView.leadingAnchor constant: 11.5].active = true;
    [container.topAnchor constraintEqualToAnchor: window.contentView.topAnchor constant: 12.0].active = true;

    [window.contentView layoutSubtreeIfNeeded];
    [window.contentView.superview viewDidEndLiveResize];
}

}
