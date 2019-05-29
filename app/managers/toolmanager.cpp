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

#define DEFAULT_CATEGORY            "Others"
#define DEFAULT_NAME                "Tool"
#define TOOLS_SOURCE_DIRECTORY      ":/tools"
#define TOOLS_DESTINATION_DIRECTORY "tools"
#define DIR_QRC_CONTROL             ":/resources/qmls/control"

namespace {

static bool currentProjectHasToolsInstalled()
{
    if (ProjectManager::uid().isEmpty())
        return false;
    if (!QDir().exists(
                ProjectManager::dir() +
                '/' +
                TOOLS_DESTINATION_DIRECTORY
                ))
        return false;
    else
        return true;
}
}

QList<ToolboxTree*> ToolManager::s_toolboxTreeList;

QString ToolManager::toolsDir()
{
    auto projectDir = ProjectManager::dir();
    if (projectDir.isEmpty()) return projectDir;
    return projectDir + '/' + TOOLS_DESTINATION_DIRECTORY;
}

QStringList ToolManager::categories()
{
    QStringList categories;

    if (!currentProjectHasToolsInstalled())
        return categories;

    for (auto dir : QDir(toolsDir()).entryList(QDir::AllDirs | QDir::NoDotAndDotDot)) {
        auto toolPath = toolsDir() + '/' + dir;
        auto category = SaveUtils::controlToolCategory(toolPath);
        if (!categories.contains(category))
            categories << category;
    }
    return categories;
}

void ToolManager::fillTree(ToolboxTree* tree)
{
    if (!currentProjectHasToolsInstalled())
        return;

    tree->clear();
    tree->clearUrls();
    for (auto toolDir : QDir(toolsDir()).entryList(QDir::AllDirs | QDir::NoDotAndDotDot))
        addToTree(toolsDir() + '/' + toolDir, tree);
}

bool ToolManager::addToTree(const QString& toolPath, ToolboxTree* tree)
{
    if (ProjectManager::uid().isEmpty() ||
            toolPath.isEmpty() || !SaveUtils::isControlValid(toolPath))
        return false;

    QList<QUrl> urls;
    auto dir = SaveUtils::toThisDir(toolPath) + '/';
    auto category = SaveUtils::category(toolPath);
    auto name = SaveUtils::name(toolPath);

    urls << QUrl::fromLocalFile(dir + SaveUtils::mainQmlFileName());
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

    QPixmap icon = QPixmap::fromImage(QImage::fromData(SaveUtils::icon(toolPath)));
    icon.setDevicePixelRatio(tree->devicePixelRatioF());
    QTreeWidgetItem* item = new QTreeWidgetItem;
    item->setText(0, name);
    item->setIcon(0, QIcon(icon));
    topItem->addChild(item);
    tree->addUrls(item, urls);

    return true;
}

void ToolManager::discharge()
{
    if (!currentProjectHasToolsInstalled())
        return;

    for (auto tree : s_toolboxTreeList) {
        tree->clearSelection();
        tree->setCurrentItem(nullptr);
        tree->clear();
        tree->clearUrls();
        emit tree->itemSelectionChanged();
    }
}

bool ToolManager::addTool(const QString& toolPath, const bool select, const bool qrc)
{
    if (ProjectManager::uid().isEmpty() ||
            toolPath.isEmpty() || !SaveUtils::isControlValid(toolPath))
        return false;

    if (!currentProjectHasToolsInstalled())
        return false;

    const bool isNewTool = !toolPath.contains(toolsDir());
    QString newToolPath;
    if (isNewTool) {
        newToolPath = toolsDir() + '/' + HashFactory::generate();

        if (!QDir(newToolPath).mkpath("."))
            return false;

        if (!FileSystemUtils::copy(toolPath, newToolPath, true, qrc))
            return false;

        SaveUtils::regenerateUids(newToolPath);
    } else {
        newToolPath = toolPath;
    }

    QList<QUrl> urls;
    auto dir = SaveUtils::toThisDir(newToolPath) + '/';
    auto category = SaveUtils::category(newToolPath);
    auto name = SaveUtils::name(newToolPath);

    urls << QUrl::fromLocalFile(dir + SaveUtils::mainQmlFileName());
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

        QPixmap icon = QPixmap::fromImage(QImage::fromData(SaveUtils::icon(newToolPath)));
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

void ToolManager::removeTool(const QString& toolPath)
{
    for (auto tree : s_toolboxTreeList) {
        tree->clearSelection();
        tree->setCurrentItem(nullptr);
        for (int i = 0; i < tree->topLevelItemCount(); i++) {
            auto tli = tree->topLevelItem(i);
            for (int j = 0; j < tli->childCount(); j++) {
                auto ci = tli->child(j);
                if (SaveUtils::toParentDir(tree->urls(ci).first().
                                toLocalFile()) == toolPath) {
                    tree->removeUrls(ci);
                    delete tli->takeChild(j);
                    if (tli->childCount() <= 0)
                        delete tree->takeTopLevelItem(i);
                    QDir(toolPath).removeRecursively();
                    emit tree->itemSelectionChanged();
                }
            }
        }
    }
}

void ToolManager::addToolboxTree(ToolboxTree* toolboxTree)
{
    s_toolboxTreeList << toolboxTree;
    fillTree(toolboxTree);
}

void ToolManager::exposeTools()
{
    if (ProjectManager::uid().isEmpty())
        return;

    for (ToolboxTree* tree : s_toolboxTreeList) {
        tree->clear();
        tree->clearUrls();
    }

    if (currentProjectHasToolsInstalled()) {
        for (const QString& toolDirName : QDir(toolsDir()).entryList(QDir::AllDirs | QDir::NoDotAndDotDot))
            addTool(toolsDir() + '/' + toolDirName, false);
    } else {
        for (const QString& toolName : QDir(TOOLS_SOURCE_DIRECTORY).entryList(QDir::Files)) {
            const QString& toolPath = QString(TOOLS_SOURCE_DIRECTORY) + '/' + toolName;
            const QString& newToolPath = toolsDir() + '/' + HashFactory::generate();

            if (!QDir(newToolPath).mkpath(".")) {
                qWarning() << QObject::tr("ToolsManager::exposeTools(): ERROR! 0x01");
                break;
            }

            if (!ZipAsync::unzipSync(toolPath, newToolPath)) {
                qWarning() << QObject::tr("ToolsManager::exposeTools(): ERROR! 0x02");
                break;
            }


            if (!addTool(newToolPath, false)) {
                qWarning() << QObject::tr("ToolsManager::exposeTools(): ERROR! 0x03");
                break;
            }
        }
    }
}

void ToolManager::newTool()
{
    addTool(DIR_QRC_CONTROL, true, true);
}

void ToolManager::resetTools()
{
    if (!currentProjectHasToolsInstalled())
        return;

    for (ToolboxTree* tree : s_toolboxTreeList) {
        tree->clearSelection();
        tree->setCurrentItem(nullptr);
        tree->clear();
        tree->clearUrls();
        emit tree->itemSelectionChanged();
    }

    QDir(toolsDir()).removeRecursively();
    exposeTools();
}

