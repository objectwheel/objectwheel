#ifndef TOOLSBACKEND_H
#define TOOLSBACKEND_H

#include <QtGlobal>
#include <QUrl>

class QJsonObject;
class ToolboxTree;

class ToolsBackend final
{
    Q_DISABLE_COPY(ToolsBackend)

public:
    struct ChangeSet {
        QString name;
        QString toolPath;
        QString category;
        QString iconPath;
    };

public:
    static void reset();
    static void newTool();
    static void resetTools();
    static void downloadTools(const QUrl& url = QUrl());
    static void removeTool(const QString& toolPath);
    static void changeTool(const ChangeSet& changeSet);
    static void addToolboxTree(ToolboxTree* toolboxTree);
    static bool addTool(const QString& toolPath, const bool select, const bool qrc = false);

    static QString toolsDir();
    static QStringList categories();

private:
    static void fillTree(ToolboxTree* tree);
    static bool addToTree(const QString& toolPath, ToolboxTree* tree);

private:
    ToolsBackend() {}

private:
    static QList<ToolboxTree*> s_toolboxTreeList;
};

#endif // TOOLSBACKEND_H
