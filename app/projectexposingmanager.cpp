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
    const auto& fpaths = SaveUtils::formPaths(ProjectManager::dir());

    for (const auto& path : fpaths) {
        auto form = new Form(path + separator() + DIR_THIS + separator() + "main.qml");

        if (SaveUtils::isMain(path))
            form->setMain(true);

        s_designerScene->addForm(form);
        form->centralize();
        //       BUG form->refresh();

        QMap<QString, Control*> pmap;
        pmap[path] = form;

        for (const auto& child : SaveUtils::childrenPaths(path)) {
            auto pcontrol = pmap.value(dname(dname(child)));
            auto control = new Control(child + separator() + DIR_THIS + separator() + "main.qml");
            ControlPropertyManager::setParent(control, pcontrol, false, false);

            //       BUG control->refresh();
            pmap[child] = control;
        }
    }
}
