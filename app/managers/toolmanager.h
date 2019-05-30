#ifndef TOOLMANAGER_H
#define TOOLMANAGER_H

#include <QObject>

class ToolboxTree;

class ToolManager final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(ToolManager)

    friend class ApplicationCore; // For constructor, init
    friend class ProjectManager; // For initTools, exposeTools

private:
    explicit ToolManager(QObject* parent = nullptr);
    static void init(ToolboxTree* tree);
    static void initTools(const QString& projectDir);
    static void exposeTools();

private:
    static ToolboxTree* s_toolboxTree;
};

#endif // TOOLMANAGER_H
