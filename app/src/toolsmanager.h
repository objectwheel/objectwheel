#ifndef TOOLSMANAGER_H
#define TOOLSMANAGER_H

#include <QUrl>

class QJsonObject;
class QString;
class ToolboxTree;

namespace ToolsManager
{
	void resetTools();
    void setToolboxTree(ToolboxTree* toolboxTree);
	void downloadTools(const QUrl& url = QUrl());
	void addTool(const QString& name);
	bool toolsExists(const QJsonObject& toolsObject);
	QString toolsDir();
    QStringList categories();
}

#endif // TOOLSMANAGER_H
