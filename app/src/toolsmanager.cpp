#include <toolsmanager.h>
#include <filemanager.h>
#include <zipper.h>
#include <projectmanager.h>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QString>
#include <QUrl>
#include <QList>
#include <QIcon>
#include <QDir>
#include <listwidget.h>
#include <QListWidgetItem>
#include <QEventLoop>

#define TOOLS_DIRECTORY "tools"
#define TOOLS_URL "qrc:/resources/tools/tools.json"

namespace ToolsManager {
	ListWidget* m_listWidget;
}

QString ToolsManager::toolsDir()
{
	auto projectDir = ProjectManager::projectDirectory(ProjectManager::currentProject());
	if (projectDir.isEmpty()) qFatal("ToolsManager : Error occurred");
	return projectDir + separator() + TOOLS_DIRECTORY;
}

void ToolsManager::resetTools()
{
	if (ProjectManager::currentProject().isEmpty()) return;
	rm(ProjectManager::projectDirectory(ProjectManager::currentProject()) + separator() + TOOLS_DIRECTORY);
	downloadTools();
}

void ToolsManager::setListWidget(ListWidget* listWidget)
{
	m_listWidget = listWidget;
}

void ToolsManager::downloadTools(const QUrl& url)
{
	if (ProjectManager::currentProject().isEmpty()) return;
	QNetworkAccessManager *manager = new QNetworkAccessManager;
	QNetworkRequest request;
	if (!url.isEmpty()) request.setUrl(url);
	else request.setUrl(QUrl::fromUserInput(TOOLS_URL));
	request.setRawHeader("User-Agent", "objectwheel 1.0");

	QEventLoop loop;
	QNetworkReply *reply = manager->get(request);
	QObject::connect(reply, static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error),
			[] { qFatal("downloadTools() : Network Error"); });
	QObject::connect(reply, &QNetworkReply::sslErrors,
			[] { qFatal("downloadTools() : Ssl Error"); });
	QObject::connect(reply, &QNetworkReply::finished, [=,&loop]
	{
		QJsonDocument toolsDoc = QJsonDocument::fromJson(reply->readAll());
		QJsonObject toolsObject = toolsDoc.object();

		if (toolsExists(toolsObject)) {
			for (auto toolName : lsdir(ProjectManager::projectDirectory(ProjectManager::currentProject()) + separator() + TOOLS_DIRECTORY)) {
				addTool(toolName);
			}
			reply->deleteLater();
			loop.quit();
			return;
		}

		for (int i = 0; i < toolsObject.size(); i++)
		{
			QString toolName = toolsObject.keys().at(i);
			QDir(ProjectManager::projectDirectory(ProjectManager::currentProject()) + separator() + TOOLS_DIRECTORY).mkpath(toolName);
			QJsonObject toolObject = toolsObject.value(toolName).toObject();
			QUrl toolUrl = QUrl::fromUserInput(toolObject.value("toolUrl").toString());

			QNetworkRequest request;
			request.setUrl(toolUrl);
			request.setRawHeader("User-Agent", "objectwheel 1.0");

			QNetworkReply *toolReply = manager->get(request);
			QObject::connect(toolReply, static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>
					(&QNetworkReply::error), [] { qFatal("downloadTools() : Network Error"); });
			QObject::connect(toolReply, &QNetworkReply::sslErrors, [] { qFatal("downloadTools() : Ssl Error"); });
			QObject::connect(toolReply, &QNetworkReply::finished, [=,&loop] {
				Zipper::extractZip(toolReply->readAll(), ProjectManager::projectDirectory(ProjectManager::currentProject()) + separator() + TOOLS_DIRECTORY + separator() + toolName);
				addTool(toolName);
				toolReply->deleteLater();
				loop.quit();
			});
		}

		reply->deleteLater();
	});
	loop.exec();
}

void ToolsManager::addTool(const QString& name)
{
	if (ProjectManager::currentProject().isEmpty()) return;
	QList<QUrl> urls;
	QListWidgetItem* item = new QListWidgetItem(QIcon(ProjectManager::projectDirectory(ProjectManager::currentProject()) + separator() + TOOLS_DIRECTORY + separator() + name + separator() + "icon.png"), name);
	urls << QUrl::fromLocalFile(ProjectManager::projectDirectory(ProjectManager::currentProject()) + separator() + TOOLS_DIRECTORY + separator() + name + separator() + "main.qml");
	m_listWidget->addItem(item);
	m_listWidget->AddUrls(item,urls);
}

bool ToolsManager::toolsExists(const QJsonObject& toolsObject)
{ // Return true if tools exist
	if (ProjectManager::currentProject().isEmpty()) return false;
	Q_UNUSED(toolsObject);
	if (!QDir().exists(ProjectManager::projectDirectory(ProjectManager::currentProject()) + separator() + TOOLS_DIRECTORY)) return false;
	else return true;

	//	for (int i = 0; i < toolsObject.size(); i++)
	//	{
	//		QString toolName = toolsObject.keys().at(i);
	//		if (!QDir(ProjectManager::projectDirectory(ProjectManager::currentProject()) + separator() + TOOLS_DIRECTORY).exists(toolName))
	//			return false;
	//	}
	//	return true;
}