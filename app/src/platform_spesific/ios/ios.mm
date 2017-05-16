#include <ios.h>
#include <QWindow>
#include <QQuickItem>
#include <UIKit/UIKit.h>

#import <UIKit/UIView.h>
#import <UIKit/UIWebView.h>
#import <UIKit/UIWindow.h>
#import <UIKit/UIViewController.h>

void IOS::fixCoordOfWebView(QWindow* main_window, QQuickItem* item, int count)
{
	UIView* view = reinterpret_cast<UIView *>(main_window->winId());

	// Get the subviews of the view
	NSArray *subviews = [view subviews];

	// Return if there are no subviews
	if ([subviews count] == 0) return;

	int count_2 = 0;
	for (UIView *subview in subviews) {
		if([subview isKindOfClass:[UIWebView class]]) {
			if (count_2 == count) {
				QPoint point = item->parentItem()->mapToGlobal(item->position()).toPoint();
				subview.frame = CGRectMake( point.x(), point.y(), subview.frame.size.width, subview.frame.size.height );
				break;
			}
			count_2++;
		}
	}
}
