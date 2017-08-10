#ifndef IOS_H
#define IOS_H

#include <QObject>

class QWindow;
class QQuickItem;

namespace IOS {
	void fixCoordOfWebView(QForm* main_window, QQuickItem* item, int count);
}

#endif // IOS_H
