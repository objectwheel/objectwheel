#include <controlcreationmanager.h>
#include <form.h>
#include <designerscene.h>
#include <saveutils.h>
#include <projectmanager.h>
#include <filemanager.h>
#include <savemanager.h>
#include <controlpreviewingmanager.h>
#include <controlpropertymanager.h>

DesignerScene* ControlCreationManager::s_designerScene = nullptr;
ControlCreationManager* ControlCreationManager::s_instance = nullptr;

ControlCreationManager::ControlCreationManager(QObject* parent) : QObject(parent)
{
    s_instance = this;
}

ControlCreationManager::~ControlCreationManager()
{
    s_instance = nullptr;
}

ControlCreationManager* ControlCreationManager::instance()
{
    return s_instance;
}

void ControlCreationManager::init(DesignerScene* designerScene)
{
    s_designerScene = designerScene;
}

Form* ControlCreationManager::addCreateForm(const QString& formRootPath)
{
    const QString& newFormRootPath = SaveManager::addForm(formRootPath);
    if (newFormRootPath.isEmpty()) {
        qWarning("ControlCreationManager::createForm: Failed.");
        return nullptr;
    }

    auto form = new Form(SaveUtils::toUrl(newFormRootPath));

    if (SaveUtils::isMain(newFormRootPath))
        form->setMain(true);

    s_designerScene->addForm(form);

    SaveManager::setupFormGlobalConnections(form->dir());

    // NOTE: We don't have to call ControlPropertyManager::setParent, since there is no valid
    // parent concept for forms in Designer; forms are directly put into DesignerScene

    ControlPreviewingManager::scheduleFormCreation(form->dir());

    // NOTE: We don't have to worry about possible child controls since createForm is only
    // called from FormsPane

    return form;
}

Control* ControlCreationManager::addCreateControl(Control* targetParentControl, const QString& controlRootPath, const QPointF& pos)
{
    const QString& newControlRootPath = SaveManager::addControl(controlRootPath, targetParentControl->dir());
    if (newControlRootPath.isEmpty()) {
        qWarning("ControlCreationManager::createControl: Failed.");
        return nullptr;
    }

    auto control = new Control(SaveUtils::toUrl(newControlRootPath));
    ControlPropertyManager::setParent(control, targetParentControl, ControlPropertyManager::NoOption);
    ControlPropertyManager::setPos(control, pos, ControlPropertyManager::SaveChanges);
    ControlPreviewingManager::scheduleControlCreation(control->dir(), targetParentControl->uid());
    emit instance()->controlCreated(control);
    // NOTE: InspectorPane dependency: Only emit after adding into db and then reparenting it.
    // Adding into db is needed because of id refactoring. Reparenting is needed because
    // InspectorPane needs the parent in order to find out parent QTreeWidgetItem to put it underneath.

    QMap<QString, Control*> controlTree;
    controlTree.insert(control->dir(), control);
    for (const QString& childPath : SaveUtils::childrenPaths(control->dir())) {
        Control* parentControl = controlTree.value(SaveUtils::toParentDir(childPath));
        Q_ASSERT(parentControl);

        auto childControl = new Control(SaveUtils::toUrl(childPath));
        ControlPropertyManager::setParent(childControl, parentControl, ControlPropertyManager::NoOption);
        ControlPreviewingManager::scheduleControlCreation(childControl->dir(), parentControl->uid());
        emit instance()->controlCreated(childControl);
        controlTree.insert(childPath, childControl);
    }

    return control;
}

