#include <toolsbackend.h>
#include <projectbackend.h>
#include <saveutils.h>
#include <savebackend.h>
#include <toolboxtree.h>
#include <filemanager.h>
#include <zipper.h>

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

#define DEFAULT_CATEGORY        "Others"
#define DEFAULT_NAME            "Tool"
#define DEFAULT_TOOLS_DIRECTORY "tools"
#define DEFAULT_TOOLS_URL       "qrc:/tools/tools.json"
#define DIR_QRC_CONTROL         ":/resources/qmls/control"

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
    if (ProjectBackend::instance()->dir().isEmpty())
        return false;
    if (!QDir().exists(
        ProjectBackend::instance()->dir() +
        separator() +
        DEFAULT_TOOLS_DIRECTORY
    ))
        return false;
    else
        return true;
}

ToolsBackend* ToolsBackend::instance()
{
    static ToolsBackend instance;
    return &instance;
}

QString ToolsBackend::toolsDir() const
{
    auto projectDir = ProjectBackend::instance()->dir();
    if (projectDir.isEmpty()) return projectDir;
    return projectDir + separator() + DEFAULT_TOOLS_DIRECTORY;
}

QStringList ToolsBackend::categories() const
{
    QStringList categories;

    if (!isProjectFull())
        return categories;

    for (auto dir : lsdir(toolsDir())) {
        auto toolPath = toolsDir() + separator() + dir;
        auto category = SaveUtils::toolCategory(toolPath);
        if (!categories.contains(category))
            categories << category;
    }
    return categories;
}

void ToolsBackend::fillTree(ToolboxTree* tree)
{
    if (!isProjectFull())
        return;

    tree->clear();
    tree->clearUrls();
    for (auto toolDir : lsdir(toolsDir()))
        addToTree(toolsDir() + separator() + toolDir, tree);
}

bool ToolsBackend::addToTree(const QString& toolPath, ToolboxTree* tree)
{
    if (ProjectBackend::instance()->dir().isEmpty() ||
        toolPath.isEmpty() || !SaveUtils::isOwctrl(toolPath))
        return false;

    QList<QUrl> urls;
    auto dir = toolPath + separator() + DIR_THIS + separator();
    auto category = SaveUtils::toolCategory(toolPath);
    auto name = SaveUtils::toolName(toolPath);

    urls << QUrl::fromLocalFile(dir + "main.qml");
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

    QTreeWidgetItem* item = new QTreeWidgetItem;
    item->setText(0, name);
    item->setIcon(0, QIcon(dir + "icon.png"));
    topItem->addChild(item);
    tree->addUrls(item, urls);

    return true;
}

void ToolsBackend::reset()
{
    if (!isProjectFull())
        return;

    for (auto tree : _toolboxTreeList) {
        tree->clearSelection();
        tree->setCurrentItem(nullptr);
        tree->clear();
        tree->clearUrls();
        emit tree->itemSelectionChanged();
    }
}

bool ToolsBackend::addTool(const QString& toolPath, const bool select, const bool qrc)
{
    if (ProjectBackend::instance()->dir().isEmpty() ||
        toolPath.isEmpty() || !SaveUtils::isOwctrl(toolPath))
        return false;

    if (!isProjectFull())
        return false;

    const bool isNewTool = !toolPath.contains(toolsDir());
    QString newToolPath;
    if (isNewTool) {
        newToolPath = toolsDir() + separator() +
          QString::number(SaveUtils::biggestDir(toolsDir()) + 1);

        if (!mkdir(newToolPath))
            return false;

        if (!cp(toolPath, newToolPath, true, qrc))
            return false;

        SaveBackend::instance()->refreshToolUid(newToolPath);
    } else {
        newToolPath = toolPath;
    }

    QList<QUrl> urls;
    auto dir = newToolPath + separator() + DIR_THIS + separator();
    auto category = SaveUtils::toolCategory(newToolPath);
    auto name = SaveUtils::toolName(newToolPath);

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
    if (ProjectBackend::instance()->dir().isEmpty()) return;

    for (auto tree : _toolboxTreeList) {
        tree->clear();
        tree->clearUrls();
    }

    QNetworkAccessManager* manager = new QNetworkAccessManager;
    QNetworkRequest request;
    if (!url.isEmpty()) request.setUrl(url);
    else request.setUrl(QUrl::fromUserInput(DEFAULT_TOOLS_URL));

    QSharedPointer<QEventLoop> loop(new QEventLoop);
    QNetworkReply* reply = manager->get(request);
    QObject::connect(reply, static_cast<void (QNetworkReply::*)
      (QNetworkReply::NetworkError)>(&QNetworkReply::error),
      this, [] { qFatal("downloadTools() : Network Error"); });
    QObject::connect(reply, &QNetworkReply::sslErrors,
      this, [] { qFatal("downloadTools() : Ssl Error"); });
    QObject::connect(reply, &QNetworkReply::finished, this, [=]
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
            QString toolDir = QString::number(SaveUtils::biggestDir(toolsDir()) + 1);
            QDir(toolsDir()).mkpath(toolDir);
            QUrl toolUrl = QUrl::fromUserInput(val.toString());

            QNetworkRequest request;
            request.setUrl(toolUrl);
            request.setRawHeader("User-Agent", "objectwheel 1.0");

            QNetworkReply* toolReply = manager->get(request);
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
    manager->deleteLater();
}

void ToolsBackend::newTool()
{
    addTool(DIR_QRC_CONTROL, true, true);
}

void ToolsBackend::resetTools()
{
    if (!isProjectFull())
        return;

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

