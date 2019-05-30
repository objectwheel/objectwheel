#ifndef TOOLMANAGER_H
#define TOOLMANAGER_H

#include <QStringList>

class ToolboxTree;

class ToolManager final
{
public:
    static void clear();
    static void exposeTools();
    static void initTools(const QString& projectDir);
    static void addToolboxTree(ToolboxTree* toolboxTree);
    static bool addToolToTrees(const QString& toolPath, const bool select, const bool qrc = false);
    static QStringList categories();

private:
    static void fillTree(ToolboxTree* tree);
    static bool addToTree(const QString& toolPath, ToolboxTree* tree);

private:
    ToolManager() = delete;
    ToolManager(const ToolManager&) = delete;
    ToolManager&operator=(const ToolManager&) = delete;

private:
    static QList<ToolboxTree*> s_toolboxTreeList;
};

#endif // TOOLMANAGER_H
