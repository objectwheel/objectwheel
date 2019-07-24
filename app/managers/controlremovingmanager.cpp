#include <controlremovingmanager.h>
#include <form.h>
#include <designerscene.h>
#include <savemanager.h>
#include <controlrenderingmanager.h>
#include <QDebug>

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

void ControlRemovingManager::removeForm(Form* form)
{
    if (!form || !form->form())
        return;

    DesignerScene* scene = form->scene();
    Q_ASSERT(scene);

    if (!scene->forms().contains(form))
        return;

    for (Control* childControl : form->childControls())
        emit instance()->controlAboutToBeRemoved(childControl);

    emit instance()->controlAboutToBeRemoved(form);

    ControlRenderingManager::scheduleFormDeletion(form->uid());

    SaveManager::removeForm(form->dir());
    scene->removeForm(form);
}

void ControlRemovingManager::removeControl(Control* control)
{
    if (!control || control->form())
        return;

    DesignerScene* scene = control->scene();
    Q_ASSERT(scene);

    for (Control* childControl : control->childControls())
        emit instance()->controlAboutToBeRemoved(childControl);

    emit instance()->controlAboutToBeRemoved(control);

    ControlRenderingManager::scheduleControlDeletion(control->uid());

    SaveManager::removeControl(control->dir());

    scene->removeControl(control);
}

void ControlRemovingManager::removeControls(const QList<Control*>& controls)
{
    for (const Control* control : controls) {
        if (control->form()) {
            qWarning() << "ControlRemovingManager::removeControls() can't remove forms.";
            return;
        }
    }

    QList<Control*> finalList(controls);
    for (const Control* control : controls) {
        for (Control* childControl : control->childControls())
            finalList.removeOne(childControl);
    }

    for (Control* control : finalList)
        removeControl(control);
}
