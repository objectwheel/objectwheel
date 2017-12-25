#include <toolsbackend.h>
#include <filemanager.h>
#include <zipper.h>
#include <projectbackend.h>
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

static void flushChangeSet(const ToolsBackend::ChangeSet& changeSet)
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
    if (ProjectBackend::currentProject().isEmpty())
        return false;
    if (!QDir().exists(ProjectBackend::projectDirectory
      (ProjectBackend::currentProject()) + separator() + DEFAULT_TOOLS_DIRECTORY))
        return false;
    else
        return true;
}

//!
//! *********************** [ToolsBackend] ***********************
//!

ToolsBackend* ToolsBackend::instance()
{
    static ToolsBackend instance;
    return &instance;
}

QString ToolsBackend::toolsDir() const
{
    auto projectDir = ProjectBackend::projectDirectory(ProjectBackend::currentProject());
    if (projectDir.isEmpty()) qFatal("ToolsBackend : Error occurred");
    return projectDir + separator() + DEFAULT_TOOLS_DIRECTORY;
}

QStringList ToolsBackend::categories() const
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

bool ToolsBackend::addTool(const QString& toolPath, const bool select, const bool qrc)
{
    if (ProjectBackend::currentProject().isEmpty() ||
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

void ToolsBackend::changeTool(const ChangeSet& changeSet)
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

void ToolsBackend::removeTool(const QString& toolPath)
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

void ToolsBackend::addToolboxTree(ToolboxTree* toolboxTree)
{
    _toolboxTreeList << toolboxTree;
    fillTree(toolboxTree);
}

void ToolsBackend::downloadTools(const QUrl& url)
{
    if (ProjectBackend::currentProject().isEmpty()) return;
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

void ToolsBackend::createNewTool()
{
    addTool(DIR_QRC_CONTROL, true, true);
}

void ToolsBackend::resetTools()
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

