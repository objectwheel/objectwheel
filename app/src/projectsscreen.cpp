#include <projectsscreen.h>
#include <QQmlContext>
#include <fit.h>

ProjectsScreen::ProjectsScreen(QWidget *parent)
	: QQuickWidget(parent)
{
	rootContext()->setContextProperty("dpi", Fit::ratio());
	setSource(QUrl("qrc:/resources/qmls/projectsScreen/main.qml"));
	setResizeMode(SizeRootObjectToView);
}
