#ifndef PROJECTEXPOSINGMANAGER_H
#define PROJECTEXPOSINGMANAGER_H

#include <QObject>

class DesignerScene;

class ProjectExposingManager final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(ProjectExposingManager)

    friend class ApplicationCore; // For constructor, init
    friend class ProjectManager; // For exposeProject()

private:
    explicit ProjectExposingManager(QObject* parent = nullptr);
    static void init(DesignerScene* designerScene);
    static void exposeProject();

private:
    static DesignerScene* s_designerScene;
};

#endif // PROJECTEXPOSINGMANAGER_H