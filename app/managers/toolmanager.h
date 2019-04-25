#ifndef TOOLMANAGER_H
#define TOOLMANAGER_H

#include <QtGlobal>
#include <QUrl>

class QJsonObject;
class ToolboxTree;

class ToolManager final
{
    Q_DISABLE_COPY(ToolManager)

public:
    static void discharge();
    static void newTool();
    static void resetTools();
    static void exposeTools();
    static void removeTool(const QString& toolPath);
    static void addToolboxTree(ToolboxTree* toolboxTree);
    static bool addTool(const QString& toolPath, const bool select, const bool qrc = false);

    static QString toolsDir();
    static QStringList categories();

private:
    static void fillTree(ToolboxTree* tree);
    static bool addToTree(const QString& toolPath, ToolboxTree* tree);

private:
    ToolManager() {}

private:
    static QList<ToolboxTree*> s_toolboxTreeList;
};

#endif // TOOLMANAGER_H
