#include <exposerbackend.h>
#include <form.h>
#include <designerscene.h>
#include <saveutils.h>
#include <projectbackend.h>
#include <filemanager.h>
#include <savebackend.h>
#include <previewerbackend.h>

DesignerScene* ExposerBackend::s_designerScene = nullptr;

void ExposerBackend::init(DesignerScene* designerScene)
{
    s_designerScene = designerScene;
}

void ExposerBackend::exposeProject()
{
    const auto& fpaths = SaveUtils::formPaths(ProjectBackend::dir());

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

Form* ExposerBackend::exposeForm(const QString& rootPath)
{
    PreviewerBackend::setDisabled(true);

    auto form = new Form(rootPath + separator() + DIR_THIS + separator() + "main.qml");

    if (SaveUtils::isMain(rootPath))
        form->setMain(true);

    SaveBackend::instance()->addForm(form);
    s_designerScene->addForm(form);

    PreviewerBackend::setDisabled(false);
    form->refresh();

    QMap<QString, Control*> pmap;
    pmap[form->dir()] = form;
    for (const auto& child : SaveUtils::childrenPaths(form->dir())) {
        auto pcontrol = pmap.value(dname(dname(child)));

        PreviewerBackend::setDisabled(true);
        auto control = new Control(child + separator() + DIR_THIS + separator() + "main.qml");
        control->setParentItem(pcontrol);
        PreviewerBackend::setDisabled(false);
        control->refresh();

        pmap[child] = control;
    }

    return form;
}

Control* ExposerBackend::exposeControl(const QString& rootPath, const QPointF& pos, QString sourceSuid,
                                       Control* parentControl, QString destinationPath,
                                       QString destinationSuid)
{
    PreviewerBackend::setDisabled(true);
    auto control = new Control(rootPath + separator() + DIR_THIS + separator() + "main.qml");

    SaveBackend::instance()->addControl(control, parentControl, destinationSuid, destinationPath);
    control->setParentItem(parentControl);
    control->setPos(pos);

    SaveUtils::setX(control->dir(), pos.x());
    SaveUtils::setY(control->dir(), pos.y());

    PreviewerBackend::setDisabled(false);
    control->refresh();

    QMap<QString, Control*> pmap;
    pmap[control->dir()] = control;
    for (const auto& child : SaveUtils::childrenPaths(control->dir(), sourceSuid)) {
        auto pcontrol = pmap.value(dname(dname(child)));

        PreviewerBackend::setDisabled(true);
        auto ccontrol = new Control(child + separator() + DIR_THIS + separator() + "main.qml");
        ccontrol->setParentItem(pcontrol);
        control->setPos(pos);

        SaveUtils::setX(control->dir(), control->x());
        SaveUtils::setY(control->dir(), control->y());

        PreviewerBackend::setDisabled(false);
        ccontrol->refresh();

        pmap[child] = ccontrol;
    }

    return control;
}
