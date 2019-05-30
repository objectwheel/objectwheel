#include <controlcreationmanager.h>
#include <form.h>
#include <designerscene.h>
#include <saveutils.h>
#include <savemanager.h>
#include <controlpreviewingmanager.h>
#include <controlpropertymanager.h>

DesignerScene* ControlCreationManager::s_designerScene = nullptr;

ControlCreationManager::ControlCreationManager(QObject* parent) : QObject(parent)
{
}

void ControlCreationManager::init(DesignerScene* designerScene)
{
    s_designerScene = designerScene;
}

Form* ControlCreationManager::createForm(const QString& formRootPath)
{
    const QString& newFormRootPath = SaveManager::addForm(formRootPath);
    if (newFormRootPath.isEmpty()) {
        qWarning("ControlCreationManager::createForm: Failed.");
        return nullptr;
    }

    auto form = new Form(newFormRootPath);

    s_designerScene->addForm(form);

    SaveManager::setupFormConnections(form->dir());

    // NOTE: We don't have to call ControlPropertyManager::setParent, since there is no valid
    // parent concept for forms in Designer; forms are directly put into DesignerScene

    ControlPreviewingManager::scheduleFormCreation(form->dir());

    // NOTE: We don't have to worry about possible child controls since createForm is only
    // called from FormsPane

    return form;
}

Control* ControlCreationManager::createControl(Control* targetParentControl, const QString& controlRootPath, const QPointF& pos)
{
    const QString& newControlRootPath = SaveManager::addControl(controlRootPath, targetParentControl->dir());
    if (newControlRootPath.isEmpty()) {
        qWarning("ControlCreationManager::createControl: Failed.");
        return nullptr;
    }

    auto control = new Control(newControlRootPath);
    ControlPropertyManager::setParent(control, targetParentControl, ControlPropertyManager::NoOption);
    ControlPropertyManager::setPos(control, pos, ControlPropertyManager::SaveChanges);
    ControlPreviewingManager::scheduleControlCreation(control->dir(), targetParentControl->uid());

    QMap<QString, Control*> controlTree;
    controlTree.insert(control->dir(), control);
    for (const QString& childPath : SaveUtils::childrenPaths(control->dir())) {
        Control* parentControl = controlTree.value(SaveUtils::toDoubleUp(childPath));
        Q_ASSERT(parentControl);

        auto childControl = new Control(childPath);
        ControlPropertyManager::setParent(childControl, parentControl, ControlPropertyManager::NoOption);
        ControlPreviewingManager::scheduleControlCreation(childControl->dir(), parentControl->uid());
        controlTree.insert(childPath, childControl);
    }

    return control;
}