#include <toolsmanager.h>
#include <filemanager.h>
#include <zipper.h>
#include <projectmanager.h>
#include <toolboxtree.h>
#include <savebackend.h>

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QString>
#include <QUrl>
#include <QList>
#include <QIcon>
#include <QDir>
#include <QTreeWidgetItem>
#include <QEventLoop>
#include <QBuffer>

#define DEFAULT_CATEGORY "Others"
#define DEFAULT_NAME "Tool"
#define DEFAULT_TOOLS_DIRECTORY "tools"
#define DEFAULT_TOOLS_URL "qrc:/resources/tools/tools.json"
#define DIR_QRC_CONTROL ":/resources/qmls/control"

//!
//! *************************** [global] ***************************
//!

static void fillTree(ToolboxTree*)
{
    // TODO
}

static void flushChangeSet(const ChangeSet& changeSet)
{
    auto dirCtrl = changeSet.toolPath +
      separator() + DIR_THIS;
    auto dirIcon = dirCtrl + separator() + FILE_ICON;
    auto propertyPath = dirCtrl + separator() + FILE_PROPERTIES;
    auto propertyData = rdfile(propertyPath);

    // Write properties
    Q_ASSERT(!propertyData.isEmpty());
    auto jobj = QJsonDocument::fromJson(propertyData).object();
    jobj[TAG_NAME] = changeSet.name;
    jobj[TAG_CATEGORY] = changeSet.category;
    propertyData = QJsonDocument(jobj).toJson();
    wrfile(propertyPath, propertyData);

    // Write icon
    const auto remoteTry = dlfile(changeSet.iconPath);
    QPixmap pixmap;
    pixmap.loadFromData(!remoteTry.isEmpty() ?
      remoteTry : dlfile(dirCtrl + separator() + changeSet.iconPath));
    Q_ASSERT(!pixmap.isNull());
    QByteArray bArray;
    QBuffer buffer(&bArray);
    buffer.open(QIODevice::WriteOnly);
    if (!pixmap.save(&buffer, "PNG")) return;
    buffer.close();
    if (!wrfile(dirIcon, bArray)) return;
}

static bool isProjectFull()
{
    if (ProjectManager::currentProject().isEmpty())
        return false;
    if (!QDir().exists(ProjectManager::projectDirectory
      (ProjectManager::currentProject()) + separator() + DEFAULT_TOOLS_DIRECTORY))
        return false;
    else
        return true;
}

//!
//! *********************** [ToolsManager] ***********************
//!

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
        auto category = SaveBackend::toolCategory(toolPath);
        if (!categories.contains(category))
            categories << category;
    }
    return categories;
}

bool ToolsManager::addTool(const QString& toolPath, const bool select, const bool qrc)
{
    if (ProjectManager::currentProject().isEmpty() ||
      toolPath.isEmpty() || !SaveBackend::isOwctrl(toolPath))
        return false;

    const bool isNewTool = !toolPath.contains(toolsDir());
    QString newToolPath;
    if (isNewTool) {
        newToolPath = toolsDir() + separator() +
          QString::number(SaveBackend::biggestDir(toolsDir()) + 1);

        if (!mkdir(newToolPath))
            return false;

        if (!cp(toolPath, newToolPath, true, qrc))
            return false;

        SaveBackend::refreshToolUid(newToolPath);
    } else {
        newToolPath = toolPath;
    }

    QList<QUrl> urls;
    auto dir = newToolPath + separator() + DIR_THIS + separator();
    auto category = SaveBackend::toolCategory(newToolPath);
    auto name = SaveBackend::toolName(newToolPath);

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

        if (select) {
            tree->clearSelection(); tree->setCurrentItem(nullptr);
            tree->setCurrentItem(item);
            tree->scrollToItem(item);
        }
    }
    return true;
}

void ToolsManager::changeTool(const ChangeSet& changeSet)
{
    for (auto tree : _toolboxTreeList) {
        tree->clearSelection(); tree->setCurrentItem(nullptr);
        for (int i = 0; i < tree->topLevelItemCount(); i++) {
            auto tli = tree->topLevelItem(i);
            for (int j = 0; j < tli->childCount(); j++) {
                auto ci = tli->child(j);
                if (dname(dname(tree->urls(ci).first().
                  toLocalFile())) == changeSet.toolPath) {
                    tree->removeUrls(ci);
                    delete tli->takeChild(j);
                    if (tli->childCount() <= 0)
                        delete tree->takeTopLevelItem(i);
                }
            }
        }
    }

    flushChangeSet(changeSet);
    addTool(changeSet.toolPath, true);
}

void ToolsManager::removeTool(const QString& toolPath)
{
    for (auto tree : _toolboxTreeList) {
        tree->clearSelection(); tree->setCurrentItem(nullptr);
        for (int i = 0; i < tree->topLevelItemCount(); i++) {
            auto tli = tree->topLevelItem(i);
            for (int j = 0; j < tli->childCount(); j++) {
                auto ci = tli->child(j);
                if (dname(dname(tree->urls(ci).first().
                  toLocalFile())) == toolPath) {
                    tree->removeUrls(ci);
                    delete tli->takeChild(j);
                    if (tli->childCount() <= 0)
                        delete tree->takeTopLevelItem(i);
                    rm(toolPath);
                    emit tree->itemSelectionChanged();
                }
            }
        }
    }
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
                addTool(toolsDir() + separator() + toolDir, false);
            reply->deleteLater();
            if (loop)
                loop->quit();
            return;
        }

        for (auto val : toolsArray)
        {
            QString toolDir = QString::number(SaveBackend::biggestDir(toolsDir()) + 1);
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
                addTool(toolsDir() + separator() + toolDir, false);
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
    addTool(DIR_QRC_CONTROL, true, true);
}

void ToolsManager::resetTools()
{
    for (auto tree : _toolboxTreeList) {
        tree->clearSelection();
        tree->setCurrentItem(nullptr);
        tree->clear();
        tree->clearUrls();
        emit tree->itemSelectionChanged();
    }
    rm(toolsDir());
    downloadTools();
}

