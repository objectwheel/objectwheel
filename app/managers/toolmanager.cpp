#include <toolmanager.h>
#include <projectmanager.h>
#include <saveutils.h>
#include <toolboxtree.h>
#include <zipasync.h>
#include <hashfactory.h>
#include <filesystemutils.h>

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QUrl>
#include <QList>
#include <QIcon>
#include <QDir>
#include <QTreeWidgetItem>
#include <QEventLoop>
#include <QtCore/QBuffer>

#define DEFAULT_NAME     "Tool"
#define DEFAULT_CATEGORY "Others"

QList<ToolboxTree*> ToolManager::s_toolboxTreeList;

QStringList ToolManager::categories()
{
    QStringList categories;

    const QString& toolsDir = SaveUtils::toProjectToolsDir(ProjectManager::dir());

    if (!QFileInfo::exists(toolsDir))
        return categories;

    for (const QString& toolDirName : QDir(toolsDir).entryList(QDir::AllDirs | QDir::NoDotAndDotDot)) {
        const QString& category = SaveUtils::controlToolCategory(toolsDir + '/' + toolDirName);
        if (!categories.contains(category))
            categories << category;
    }

    return categories;
}

void ToolManager::fillTree(ToolboxTree* tree)
{
    const QString& toolsDir = SaveUtils::toProjectToolsDir(ProjectManager::dir());

    if (!QFileInfo::exists(toolsDir))
        return;

    tree->clear();
    tree->clearUrls();

    for (const QString& toolDirName : QDir(toolsDir).entryList(QDir::AllDirs | QDir::NoDotAndDotDot))
        addToTree(toolsDir + '/' + toolDirName, tree);
}

bool ToolManager::addToTree(const QString& toolPath, ToolboxTree* tree)
{
    if (ProjectManager::uid().isEmpty() || toolPath.isEmpty() || !SaveUtils::isControlValid(toolPath))
        return false;

    QList<QUrl> urls;
    auto category = SaveUtils::controlToolCategory(toolPath);
    auto name = SaveUtils::controlToolName(toolPath);

    urls << QUrl::fromLocalFile(toolPath);
    if (category.isEmpty())
        category = DEFAULT_CATEGORY;
    if (name.isEmpty())
        name = DEFAULT_NAME;

    auto topItem = tree->categoryItem(category);
    if (!topItem) {
        topItem = new QTreeWidgetItem;
        topItem->setText(0, category);
        tree->addTopLevelItem(topItem);
        topItem->setExpanded(true);
    }

    QPixmap icon = QPixmap::fromImage(QImage::fromData(SaveUtils::controlIcon(toolPath)));
    icon.setDevicePixelRatio(tree->devicePixelRatioF());
    QTreeWidgetItem* item = new QTreeWidgetItem;
    item->setText(0, name);
    item->setIcon(0, QIcon(icon));
    topItem->addChild(item);
    tree->addUrls(item, urls);

    return true;
}

void ToolManager::clear()
{
    for (ToolboxTree* tree : s_toolboxTreeList) {
        tree->clearSelection();
        tree->setCurrentItem(nullptr);
        tree->clear();
        tree->clearUrls();
        emit tree->itemSelectionChanged();
    }
}

bool ToolManager::addToolToTrees(const QString& toolPath, const bool select, const bool qrc)
{
    if (toolPath.isEmpty() || !SaveUtils::isControlValid(toolPath))
        return false;

    QList<QUrl> urls;
    auto category = SaveUtils::controlToolCategory(toolPath);
    auto name = SaveUtils::controlToolName(toolPath);

    urls << QUrl::fromLocalFile(toolPath);

    if (category.isEmpty())
        category = DEFAULT_CATEGORY;

    if (name.isEmpty())
        name = DEFAULT_NAME;

    for (auto tree : s_toolboxTreeList) {
        auto topItem = tree->categoryItem(category);
        if (!topItem) {
            topItem = new QTreeWidgetItem;
            topItem->setText(0, category);
            tree->addTopLevelItem(topItem);
            topItem->setExpanded(true);
        }

        QPixmap icon = QPixmap::fromImage(QImage::fromData(SaveUtils::controlIcon(toolPath)));
        icon.setDevicePixelRatio(tree->devicePixelRatioF());
        QTreeWidgetItem* item = new QTreeWidgetItem;
        item->setText(0, name);
        item->setIcon(0, QIcon(icon));
        topItem->addChild(item);
        tree->addUrls(item, urls);

        if (select) {
            tree->clearSelection();
            tree->setCurrentItem(nullptr);
            tree->setCurrentItem(item);
            tree->scrollToItem(item);
        }
    }

    return true;
}

void ToolManager::addToolboxTree(ToolboxTree* toolboxTree)
{
    s_toolboxTreeList << toolboxTree;
    fillTree(toolboxTree);
}

void ToolManager::initTools(const QString& projectDir)
{
    const QString& toolsDir = SaveUtils::toProjectToolsDir(projectDir);

    if (!QFileInfo::exists(projectDir))
        return;

    for (ToolboxTree* tree : s_toolboxTreeList) {
        tree->clear();
        tree->clearUrls();
    }

    const QString& toolsSourceDir = ":/tools";
    for (const QString& toolName : QDir(toolsSourceDir).entryList(QDir::Files)) {
        const QString& toolPath = toolsSourceDir + '/' + toolName;
        const QString& newToolPath = toolsDir + '/' + HashFactory::generate();

        if (!QDir(newToolPath).mkpath(".")) {
            qWarning() << QObject::tr("ToolsManager::exposeTools(): ERROR! 0x01");
            break;
        }

        if (!ZipAsync::unzipSync(toolPath, newToolPath)) {
            qWarning() << QObject::tr("ToolsManager::exposeTools(): ERROR! 0x02");
            break;
        }
    }
}

void ToolManager::exposeTools()
{
    const QString& toolsDir = SaveUtils::toProjectToolsDir(ProjectManager::dir());

    if (!QFileInfo::exists(toolsDir))
        return;

    for (ToolboxTree* tree : s_toolboxTreeList) {
        tree->clear();
        tree->clearUrls();
    }

    for (const QString& toolDirName : QDir(toolsDir).entryList(QDir::AllDirs | QDir::NoDotAndDotDot))
        addToolToTrees(toolsDir + '/' + toolDirName, false);
}
