#include <controlexposingmanager.h>
#include <form.h>
#include <designerscene.h>
#include <saveutils.h>
#include <projectmanager.h>
#include <filemanager.h>
#include <savemanager.h>
#include <controlpreviewingmanager.h>

DesignerScene* ControlExposingManager::s_designerScene = nullptr;

void ControlExposingManager::init(DesignerScene* designerScene)
{
    s_designerScene = designerScene;
}

void ControlExposingManager::exposeProject()
{
    const auto& fpaths = SaveUtils::formPaths(ProjectManager::dir());

    for (const auto& path : fpaths) {
        auto form = new Form(path + separator() + DIR_THIS + separator() + "main.qml");

        if (SaveUtils::isMain(path))
            form->setMain(true);

        s_designerScene->addForm(form);
        form->centralize();
        form->refresh();

        QMap<QString, Control*> pmap;
        pmap[path] = form;

        for (const auto& child : SaveUtils::childrenPaths(path)) {
            auto pcontrol = pmap.value(dname(dname(child)));
            auto control = new Control(child + separator() + DIR_THIS + separator() + "main.qml");
            control->blockSignals(true);
            control->setParentItem(pcontrol);
            control->blockSignals(false);
            control->refresh();
            pmap[child] = control;
        }
    }
}

Form* ControlExposingManager::exposeForm(const QString& rootPath)
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

        pmap[child] = control;
    }

    return form;
}

Control* ControlExposingManager::exposeControl(const QString& rootPath, const QPointF& pos, QString sourceSuid,
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

        pmap[child] = ccontrol;
    }

    return control;
}
