#include <toolsmanager.h>
#include <filemanager.h>
#include <zipper.h>
#include <projectmanager.h>
#include <toolboxtree.h>
#include <savemanager.h>

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonValue>
#include <QString>
#include <QUrl>
#include <QList>
#include <QIcon>
#include <QDir>
#include <QTreeWidgetItem>
#include <QEventLoop>

#define DEFAULT_CATEGORY "Others"
#define DEFAULT_NAME "Tool"
#define DEFAULT_TOOLS_DIRECTORY "tools"
#define DEFAULT_TOOLS_URL "qrc:/resources/tools/tools.json"

void fillTree(ToolboxTree* toolboxTree)
{

}

bool isProjectFull()
{
    if (ProjectManager::currentProject().isEmpty())
        return false;
    if (!QDir().exists(ProjectManager::projectDirectory
      (ProjectManager::currentProject()) + separator() + DEFAULT_TOOLS_DIRECTORY))
        return false;
    else
        return true;
}

ToolsManager* ToolsManager::instance()
{
    static ToolsManager instance;
    return &instance;
}

QString ToolsManager::toolsDir() const
{
    auto projectDir = ProjectManager::projectDirectory(ProjectManager::currentProject());
    if (projectDir.isEmpty()) qFatal("ToolsManager : Error occurred");
    return projectDir + separator() + DEFAULT_TOOLS_DIRECTORY;
}

QStringList ToolsManager::categories() const
{
    QStringList categories;
    for (auto dir : lsdir(toolsDir())) {
        auto toolPath = toolsDir() + separator() + dir;
        auto category = SaveManager::toolCategory(toolPath);
        if (!categories.contains(category))
            categories << category;
    }
    return categories;
}

bool ToolsManager::addTool(const QString& toolPath)
{
    if (ProjectManager::currentProject().isEmpty() ||
      toolPath.isEmpty() || !SaveManager::isOwctrl(toolPath))
        return false;

    const bool isNewTool = !toolPath.contains(toolsDir());
    QString newToolPath;
    if (isNewTool) {
        newToolPath = toolsDir() + separator() +
          QString::number(SaveManager::biggestDir(toolsDir()) + 1);

        if (!mkdir(newToolPath))
            return false;

        if (!cp(toolPath, newToolPath, true))
            return false;

        SaveManager::refreshToolUid(newToolPath);
    } else {
        newToolPath = toolPath;
    }

    QList<QUrl> urls;
    auto dir = newToolPath + separator() + DIR_THIS + separator();
    auto category = SaveManager::toolCategory(newToolPath);
    auto name = SaveManager::toolName(newToolPath);

    urls << QUrl::fromLocalFile(dir + "main.qml");
    if (category.isEmpty())
        category = DEFAULT_CATEGORY;
    if (name.isEmpty())
        name = DEFAULT_NAME;

    for (auto tree : _toolboxTreeList) {
        auto topItem = tree->categoryItem(category);
        if (!topItem) {
            topItem = new QTreeWidgetItem;
            topItem->setText(0, category);
            tree->addTopLevelItem(topItem);
            topItem->setExpanded(true);
        }

        QTreeWidgetItem* item = new QTreeWidgetItem;
        item->setText(0, name);
        item->setIcon(0, QIcon(dir + "icon.png"));
        topItem->addChild(item);
        tree->addUrls(item, urls);

        if (isNewTool) {
            item->setSelected(true);
            tree->scrollToItem(item);
        }
    }
    return true;
}

void ToolsManager::removeTool(const QString& toolPath)
{

}

void ToolsManager::addToolboxTree(ToolboxTree* toolboxTree)
{
    _toolboxTreeList << toolboxTree;
    fillTree(toolboxTree);
}

void ToolsManager::downloadTools(const QUrl& url)
{
    if (ProjectManager::currentProject().isEmpty()) return;
    QNetworkAccessManager *manager = new QNetworkAccessManager;
    QNetworkRequest request;
    if (!url.isEmpty()) request.setUrl(url);
    else request.setUrl(QUrl::fromUserInput(DEFAULT_TOOLS_URL));
    request.setRawHeader("User-Agent", "objectwheel 1.0");

    QSharedPointer<QEventLoop> loop(new QEventLoop);
    QNetworkReply *reply = manager->get(request);
    QObject::connect(reply, static_cast<void (QNetworkReply::*)
      (QNetworkReply::NetworkError)>(&QNetworkReply::error),
      [] { qFatal("downloadTools() : Network Error"); });
    QObject::connect(reply, &QNetworkReply::sslErrors,
      [] { qFatal("downloadTools() : Ssl Error"); });
    QObject::connect(reply, &QNetworkReply::finished, [=]
    {
        QJsonDocument toolsDoc = QJsonDocument::fromJson(reply->readAll());
        QJsonArray toolsArray = toolsDoc.array();

        if (isProjectFull()) {
            for (auto toolDir : lsdir(toolsDir()))
                addTool(toolsDir() + separator() + toolDir);
            reply->deleteLater();
            if (loop)
                loop->quit();
            return;
        }

        for (auto val : toolsArray)
        {
            QString toolDir = QString::number(SaveManager::biggestDir(toolsDir()) + 1);
            QDir(toolsDir()).mkpath(toolDir);
            QUrl toolUrl = QUrl::fromUserInput(val.toString());

            QNetworkRequest request;
            request.setUrl(toolUrl);
            request.setRawHeader("User-Agent", "objectwheel 1.0");

            QNetworkReply *toolReply = manager->get(request);
            QObject::connect(toolReply, static_cast<void (QNetworkReply::*)
              (QNetworkReply::NetworkError)> (&QNetworkReply::error), []
              { qFatal("downloadTools() : Network Error"); });
            QObject::connect(toolReply, &QNetworkReply::sslErrors, []
              { qFatal("downloadTools() : Ssl Error"); });
            QObject::connect(toolReply, &QNetworkReply::finished, [=] {
                Zipper::extractZip(toolReply->readAll(), toolsDir() + separator() + toolDir);
                addTool(toolsDir() + separator() + toolDir);
                toolReply->deleteLater();
                if (loop)
                    loop->quit();
            });
        }

        reply->deleteLater();
    });
    loop->exec();
}


void ToolsManager::createNewTool()
{

}

void ToolsManager::resetTools()
{
    rm(toolsDir());
    downloadTools();
}

