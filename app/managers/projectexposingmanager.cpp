#include <projectexposingmanager.h>
#include <saveutils.h>
#include <projectmanager.h>
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
    int formIndex = 0;
    for (const QString& formPath : SaveUtils::formPaths(ProjectManager::dir())) {
        auto form = new Form(formPath);

        if (form->id().isEmpty())
            ControlPropertyManager::setId(form, "form", ControlPropertyManager::SaveChanges);

        if (form->id() != SaveUtils::controlId(form->dir()))
            SaveUtils::setProperty(form->dir(), SaveUtils::ControlId, form->id());

        // FIXME
        if (form->index() != formIndex++) {
            SaveUtils::setProperty(form->dir(), SaveUtils::ControlIndex, form->id());
        }

        s_designerScene->addForm(form);

        QMap<QString, Control*> controlTree;
        controlTree.insert(formPath, form);

        for (const QString& childPath : SaveUtils::childrenPaths(formPath)) {
            Control* parentControl = controlTree.value(SaveUtils::toDoubleUp(childPath));
            Q_ASSERT(parentControl);

            auto control = new Control(childPath);

            if (control->id().isEmpty())
                ControlPropertyManager::setId(control, "control", ControlPropertyManager::SaveChanges);

            if (control->id() != SaveUtils::controlId(control->dir()))
                SaveUtils::setProperty(control->dir(), SaveUtils::ControlId, control->id());

            ControlPropertyManager::setParent(control, parentControl, ControlPropertyManager::NoOption);
            controlTree.insert(childPath, control);
        }
    }
}
