#include <container.h>
#include <QAction>

Container::Container(QWidget* const parent)
	: QStackedWidget(parent)
{
}

void Container::handleAction()
{
	auto action = qobject_cast<QAction*>(sender());
	if (!action) {
		return;
	}
	auto widget = action->data().value<QWidget*>();
	if (widget) {
		setCurrentWidget(widget);
	} else {
		Q_ASSERT(0); //Invalid use of Container class
	}
}
