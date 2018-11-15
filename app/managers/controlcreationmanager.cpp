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

Form* ControlCreationManager::createForm(const QString& rootPath)
{
    auto form = new Form(SaveUtils::toUrl(rootPath));

    if (SaveUtils::isMain(rootPath))
        form->setMain(true);

    SaveManager::addForm(form);
    s_designerScene->addForm(form);

    if (form->id() != SaveUtils::id(form->dir()))
        SaveUtils::setProperty(form->dir(), TAG_ID, form->id());

    SaveManager::setupFormGlobalConnections(form);

    // NOTE: We don't have to call ControlPropertyManager::setParent, since there is no valid
    // parent concept for forms in Designer; fors are directly put into DesignerScene

    ControlPreviewingManager::scheduleFormCreation(form->dir());

    // NOTE: We don't have to worry about possible child controls since createForm is only
    // called from FormsPane

    return form;
}

Control* ControlCreationManager::createControl(const QString& rootPath, const QPointF& pos, QString sourceSuid,
                                               Control* parentControl, QString destinationPath,
                                               QString destinationSuid)
{
    auto control = new Control(SaveUtils::toUrl(rootPath));
//    SaveManager::addControl(control, parentControl, destinationSuid, destinationPath);

    if (control->id() != SaveUtils::id(control->dir()))
        SaveUtils::setProperty(control->dir(), TAG_ID, control->id());

    ControlPropertyManager::setParent(control, parentControl, ControlPropertyManager::NoOption);
    ControlPropertyManager::setPos(control, pos, ControlPropertyManager::SaveChanges);
    ControlPreviewingManager::scheduleControlCreation(control->dir(), parentControl->uid());
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
        if (childControl->id() != SaveUtils::id(childControl->dir()))
            SaveUtils::setProperty(childControl->dir(), TAG_ID, childControl->id());

        ControlPropertyManager::setParent(childControl, parentControl, ControlPropertyManager::NoOption);
        ControlPreviewingManager::scheduleControlCreation(childControl->dir(), parentControl->uid());
        emit instance()->controlCreated(childControl);

        controlTree.insert(childPath, childControl);
    }

    return control;
}

