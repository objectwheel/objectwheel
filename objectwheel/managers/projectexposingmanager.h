#ifndef PROJECTEXPOSINGMANAGER_H
#define PROJECTEXPOSINGMANAGER_H

#include <QObject>

class DesignerScene;
class Control;

class ProjectExposingManager final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(ProjectExposingManager)

    friend class ApplicationCore; // For constructor, init
    friend class ProjectManager; // For exposeProject()

public:
    static ProjectExposingManager* instance();

private:
    explicit ProjectExposingManager(QObject* parent = nullptr);
    ~ProjectExposingManager() override;
    static void init(DesignerScene* designerScene);
    static void exposeProject();
    static void removeProject();

signals:
    void controlExposed(Control* control);

private:
    static ProjectExposingManager* s_instance;
    static DesignerScene* s_designerScene;
};

#endif // PROJECTEXPOSINGMANAGER_H