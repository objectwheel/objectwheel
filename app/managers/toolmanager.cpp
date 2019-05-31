#include <toolmanager.h>
#include <projectmanager.h>
#include <saveutils.h>
#include <toolboxtree.h>
#include <zipasync.h>
#include <hashfactory.h>
#include <QDir>

ToolboxTree* ToolManager::s_toolboxTree = nullptr;

ToolManager::ToolManager(QObject* parent) : QObject(parent)
{
}

void ToolManager::init(ToolboxTree* toolboxTree)
{
    s_toolboxTree = toolboxTree;
}

void ToolManager::initTools(const QString& projectDir)
{
    if (!QFileInfo::exists(projectDir))
        return;

    static const QString toolsSourceDir = ":/tools";
    for (const QString& toolName : QDir(toolsSourceDir).entryList(QDir::Files)) {
        const QString& toolPath = toolsSourceDir + '/' + toolName;
        const QString& newToolPath = SaveUtils::toProjectToolsDir(projectDir) + '/' + QFileInfo(toolName).baseName();

        if (!QDir(newToolPath).mkpath("m")) {
            qWarning("ToolsManager::exposeTools(): ERROR! 0x01");
            return;
        }

        if (!ZipAsync::unzipSync(toolPath, newToolPath)) {
            qWarning("ToolsManager::exposeTools(): ERROR! 0x02");
            return;
        }

        QFile::rename(SaveUtils::toControlThisDir(newToolPath) + '/' + SaveUtils::controlMetaFileName(),
                      SaveUtils::toControlMetaDir(newToolPath) + '/' + SaveUtils::controlMetaFileName());
    }
}

void ToolManager::exposeTools()
{
//    if (!ProjectManager::isStarted()) {
//        qWarning("ToolsManager::exposeTools: Project is not started yet");
//        return;
//    }

//    const QString& toolsDir = SaveUtils::toProjectToolsDir(ProjectManager::dir());

//    for (const QString& toolDirName
//         : QDir(toolsDir).entryList(QDir::AllDirs | QDir::NoDotAndDotDot)) {
//        const QString& toolPath = toolsDir + '/' + toolDirName;
//        if (SaveUtils::isControlValid(toolPath)) {
//            QList<QUrl> urls;
//            QString category = SaveUtils::controlToolCategory(toolPath);
//            QString name = SaveUtils::controlToolName(toolPath);

//            urls << QUrl::fromLocalFile(toolPath);

//            if (category.isEmpty())
//                category = QStringLiteral("Others");

//            if (name.isEmpty())
//                name = QStringLiteral("Tool");

//            QTreeWidgetItem* topItem = s_toolboxTree->categoryItem(category);
//            if (!topItem) {
//                topItem = new QTreeWidgetItem;
//                topItem->setText(0, category);
//                s_toolboxTree->addTopLevelItem(topItem);
//                topItem->setExpanded(true);
//            }

//            QPixmap icon = QPixmap::fromImage(QImage::fromData(SaveUtils::controlIcon(toolPath)));
//            icon.setDevicePixelRatio(s_toolboxTree->devicePixelRatioF());
//            QTreeWidgetItem* item = new QTreeWidgetItem;
//            item->setText(0, name);
//            item->setIcon(0, QIcon(icon));
//            topItem->addChild(item);
//            s_toolboxTree->addUrls(item, urls);
//        }
//    }
}
