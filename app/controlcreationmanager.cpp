#include <controlcreationmanager.h>
#include <form.h>
#include <designerscene.h>
#include <saveutils.h>
#include <projectmanager.h>
#include <filemanager.h>
#include <savemanager.h>
#include <controlpreviewingmanager.h>

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
    ControlPreviewingManager::setDisabled(true);

    auto form = new Form(rootPath + separator() + DIR_THIS + separator() + "main.qml");

    if (SaveUtils::isMain(rootPath))
        form->setMain(true);

    SaveManager::addForm(form);
    s_designerScene->addForm(form);

    ControlPreviewingManager::setDisabled(false);
    form->refresh();

    QMap<QString, Control*> pmap;
    pmap[form->dir()] = form;
    for (const auto& child : SaveUtils::childrenPaths(form->dir())) {
        auto pcontrol = pmap.value(dname(dname(child)));

        ControlPreviewingManager::setDisabled(true);
        auto control = new Control(child + separator() + DIR_THIS + separator() + "main.qml");
        control->setParentItem(pcontrol);
        ControlPreviewingManager::setDisabled(false);
        control->refresh();

        emit instance()->controlCreated(control);

        pmap[child] = control;
    }

    emit instance()->formCreated(form);

    return form;
}

Control* ControlCreationManager::createControl(const QString& rootPath, const QPointF& pos, QString sourceSuid,
                                               Control* parentControl, QString destinationPath,
                                               QString destinationSuid)
{
    ControlPreviewingManager::setDisabled(true);
    auto control = new Control(rootPath + separator() + DIR_THIS + separator() + "main.qml");

    SaveManager::addControl(control, parentControl, destinationSuid, destinationPath);
    control->setParentItem(parentControl);
    control->setPos(pos);

    SaveUtils::setX(control->dir(), pos.x());
    SaveUtils::setY(control->dir(), pos.y());

    ControlPreviewingManager::setDisabled(false);
    control->refresh();

    QMap<QString, Control*> pmap;
    pmap[control->dir()] = control;
    for (const auto& child : SaveUtils::childrenPaths(control->dir(), sourceSuid)) {
        auto pcontrol = pmap.value(dname(dname(child)));

        ControlPreviewingManager::setDisabled(true);
        auto ccontrol = new Control(child + separator() + DIR_THIS + separator() + "main.qml");
        ccontrol->setParentItem(pcontrol);
        control->setPos(pos);

        SaveUtils::setX(control->dir(), control->x());
        SaveUtils::setY(control->dir(), control->y());

        ControlPreviewingManager::setDisabled(false);
        ccontrol->refresh();

        emit instance()->controlCreated(control);

        pmap[child] = ccontrol;
    }

    emit instance()->controlCreated(control);

    return control;
}

