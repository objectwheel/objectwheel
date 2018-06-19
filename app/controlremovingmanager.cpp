#include <controlremovingmanager.h>
#include <form.h>
#include <designerscene.h>
#include <savemanager.h>
#include <controlpreviewingmanager.h>

DesignerScene* ControlRemovingManager::s_designerScene = nullptr;
ControlRemovingManager* ControlRemovingManager::s_instance = nullptr;

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

void ControlRemovingManager::init(DesignerScene* designerScene)
{
    s_designerScene = designerScene;
}

void ControlRemovingManager::removeForm(Form* form)
{
    if (!form || !form->form() || form->main())
        return;

    if (!s_designerScene->forms().contains(form))
        return;

    for (Control* childControl : form->childControls())
        emit instance()->controlAboutToBeRemoved(childControl);

    emit instance()->formAboutToBeRemoved(form);

    SaveManager::removeForm(form);
    s_designerScene->removeForm(form);
}

void ControlRemovingManager::removeControl(Control* control)
{
    if (!control || control->form())
        return;

    for (Control* childControl : control->childControls())
        emit instance()->controlAboutToBeRemoved(childControl);

    emit instance()->controlAboutToBeRemoved(control);

    ControlPreviewingManager::removeCache(control->uid());

    control->parentControl()->refresh();
    control->setRefreshingDisabled(true);
    control->blockSignals(true);

    SaveManager::removeControl(control);
    s_designerScene->removeControl(control);
}
