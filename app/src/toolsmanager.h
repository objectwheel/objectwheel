#ifndef TOOLSMANAGER_H
#define TOOLSMANAGER_H

#include <QUrl>

class QJsonObject;
class QString;
class ListWidget;

namespace ToolsManager
{
	void resetTools();
	void setListWidget(ListWidget* listWidget);
	void downloadTools(const QUrl& url = QUrl());
	void addTool(const QString& name);
	bool toolsExists(const QJsonObject& toolsObject);
	QString toolsDir();
}

#endif // TOOLSMANAGER_H
