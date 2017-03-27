#include <projectsscreen.h>

ProjectsScreen::ProjectsScreen(QWidget *parent)
	: QQuickWidget(parent)
{
	setSource(QUrl("qrc:/resources/qmls/projectsScreen/main.qml"));
	setResizeMode(SizeRootObjectToView);
}
