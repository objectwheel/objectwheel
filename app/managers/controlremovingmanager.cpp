#include <controlremovingmanager.h>
#include <designerscene.h>
#include <savemanager.h>
#include <controlrenderingmanager.h>
#include <form.h>

ControlRemovingManager* ControlRemovingManager::s_instance = nullptr;
DesignerScene* ControlRemovingManager::s_designerScene = nullptr;

ControlRemovingManager::ControlRemovingManager(QObject* parent) : QObject(parent)
{
    s_instance = this;
}

ControlRemovingManager::~ControlRemovingManager()
{
    s_instance = nullptr;
}

ControlRemovingManager* ControlRemovingManager::instance()
{
    return s_instance;
}

void ControlRemovingManager::init(DesignerScene* scene)
{
    s_designerScene = scene;
}

void ControlRemovingManager::removeControl(Control* control, bool removeFromDatabaseAlso)
{
    if (control == 0)
        return;

    for (Control* childControl : control->childControls())
        emit instance()->controlAboutToBeRemoved(childControl);

    emit instance()->controlAboutToBeRemoved(control);

    if (control->type() == Form::Type) {
        ControlRenderingManager::scheduleFormDeletion(control->uid());
        if (removeFromDatabaseAlso)
            SaveManager::removeForm(control->dir());
        bool inUse = control == s_designerScene->currentForm();
        s_designerScene->removeForm(static_cast<Form*>(control)); // deletes control
        if (inUse && !s_designerScene->forms().isEmpty())
            s_designerScene->setCurrentForm(s_designerScene->forms().first());
    } else {
        ControlRenderingManager::scheduleControlDeletion(control->uid());
        if (removeFromDatabaseAlso)
            SaveManager::removeControl(control->dir());
        s_designerScene->removeControl(control); // deletes control
    }
}

void ControlRemovingManager::removeControls(const QList<Control*>& controls, bool removeFromDatabaseAlso)
{
    QList<Control*> finalList(controls);
    for (const Control* control : controls) {
        if (control->type() == Form::Type)
            continue;
        const QList<Control*>& childControls = control->childControls();
        for (Control* childControl : childControls)
            finalList.removeOne(childControl);
    }

    for (Control* control : finalList) {
        if (control->type() != Form::Type)
            removeControl(control, removeFromDatabaseAlso);
    }
}
