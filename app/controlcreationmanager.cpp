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

// FIXME: createControl and createForm should use DesignerScene to deal with
// GUI side control adding operations
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
// BUG   ControlPreviewingManager::setDisabled(true);

    auto form = new Form(rootPath + separator() + DIR_THIS + separator() + "main.qml");

    if (SaveUtils::isMain(rootPath))
        form->setMain(true);

    SaveManager::addForm(form);
    s_designerScene->addForm(form);
    // NOTE: We don't have to call ControlPropertyManager::setParent, since there is no valid
    // parent concept for forms in Designer; fors are directly put into DesignerScene

//    ControlPreviewingManager::setDisabled(false);
//    form->refresh();
//    ControlPreviewingManager::scheduleFormCreation(control->dir(), parentControl->uid());

    // NOTE: We don't have to worry about possible child controls since createForm is only
    // called from FormsPane

    emit instance()->formCreated(form);

    return form;
}

// FIXME:
Control* ControlCreationManager::createControl(const QString& rootPath, const QPointF& pos, QString sourceSuid,
                                               Control* parentControl, QString destinationPath,
                                               QString destinationSuid)
{
//    BUG ControlPreviewingManager::setDisabled(true);
    auto control = new Control(rootPath + separator() + DIR_THIS + separator() + "main.qml");

    SaveManager::addControl(control, parentControl, destinationSuid, destinationPath);
    ControlPropertyManager::setParent(control, parentControl, false, false);
    ControlPropertyManager::setPos(control, pos, false, false);

    SaveUtils::setX(control->dir(), pos.x());
    SaveUtils::setY(control->dir(), pos.y());

//    ControlPreviewingManager::setDisabled(false);
    ControlPreviewingManager::scheduleControlCreation(control->dir(), parentControl->uid());

    QMap<QString, Control*> pmap;
    pmap[control->dir()] = control;
    for (const auto& child : SaveUtils::childrenPaths(control->dir(), sourceSuid)) {
        auto pcontrol = pmap.value(SaveUtils::toParentDir(child));

//        ControlPreviewingManager::setDisabled(true);
        auto ccontrol = new Control(child + separator() + DIR_THIS + separator() + "main.qml");
        ControlPropertyManager::setParent(ccontrol, pcontrol, false, false);
//        control->setPos(pos); FIXME
//        ControlPropertyManager::setPos(control, pos, false, false); FIXME


//        SaveUtils::setX(control->dir(), control->x()); FIXME
//        SaveUtils::setY(control->dir(), control->y()); FIXME

//        ControlPreviewingManager::setDisabled(false);
        ControlPreviewingManager::scheduleControlCreation(ccontrol->dir(), pcontrol->uid());

        emit instance()->controlCreated(control);

        pmap[child] = ccontrol;
    }

    emit instance()->controlCreated(control);

    return control;
}

