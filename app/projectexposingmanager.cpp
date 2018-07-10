#include <projectexposingmanager.h>
#include <saveutils.h>
#include <projectmanager.h>
#include <filemanager.h>
#include <control.h>
#include <form.h>
#include <designerscene.h>
#include <controlpropertymanager.h>

DesignerScene* ProjectExposingManager::s_designerScene = nullptr;

ProjectExposingManager::ProjectExposingManager(QObject* parent) : QObject(parent)
{
}

void ProjectExposingManager::init(DesignerScene* designerScene)
{
    s_designerScene = designerScene;
}

void ProjectExposingManager::exposeProject()
{
    for (const QString& formPath : SaveUtils::formPaths(ProjectManager::dir())) {
        auto form = new Form(SaveUtils::toUrl(formPath));

        if (SaveUtils::isMain(formPath))
            form->setMain(true);

        s_designerScene->addForm(form);

        QMap<QString, Control*> controlTree;
        controlTree.insert(formPath, form);

        for (const QString& childPath : SaveUtils::childrenPaths(formPath)) {
            Control* parentControl = controlTree.value(SaveUtils::toParentDir(childPath));
            Q_ASSERT(parentControl);

            auto control = new Control(SaveUtils::toUrl(childPath));
            ControlPropertyManager::setParent(control, parentControl, false, false);
            controlTree.insert(childPath, control);
        }
    }
}
