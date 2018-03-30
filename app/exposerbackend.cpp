#include <exposerbackend.h>
#include <form.h>
#include <designerscene.h>
#include <saveutils.h>
#include <projectbackend.h>
#include <filemanager.h>
#include <savebackend.h>
#include <previewerbackend.h>

ExposerBackend::ExposerBackend() : m_designerScene(nullptr)
{
}

ExposerBackend* ExposerBackend::instance()
{
    static ExposerBackend instance;
    return &instance;
}

void ExposerBackend::init(DesignerScene* designerScene)
{
    m_designerScene = designerScene;
}

void ExposerBackend::exposeProject() const
{
    const auto& fpaths = SaveUtils::formPaths(ProjectBackend::instance()->dir());

    for (const auto& path : fpaths) {
        auto form = new Form(path + separator() + DIR_THIS + separator() + "main.qml");

        if (SaveUtils::isMain(path))
            form->setMain(true);

        m_designerScene->addForm(form);

        QMap<QString, Control*> pmap;
        pmap[path] = form;

        for (const auto& child : SaveUtils::childrenPaths(path)) {
            auto pcontrol = pmap.value(dname(dname(child)));
            auto control = new Control(child + separator() + DIR_THIS + separator() + "main.qml");
            control->setParentItem(pcontrol);
            pmap[child] = control;
        }
    }
}

Form* ExposerBackend::exposeForm(const QString& rootPath) const
{
    PreviewerBackend::instance()->setDisabled(true);

    auto form = new Form(rootPath + separator() + DIR_THIS + separator() + "main.qml");

    if (SaveUtils::isMain(rootPath))
        form->setMain(true);

    SaveBackend::instance()->addForm(form);
    m_designerScene->addForm(form);

    PreviewerBackend::instance()->setDisabled(false);
    form->refresh();

    QMap<QString, Control*> pmap;
    pmap[form->dir()] = form;
    for (const auto& child : SaveUtils::childrenPaths(form->dir())) {
        auto pcontrol = pmap.value(dname(dname(child)));

        PreviewerBackend::instance()->setDisabled(true);
        auto control = new Control(child + separator() + DIR_THIS + separator() + "main.qml");
        control->setParentItem(pcontrol);
        PreviewerBackend::instance()->setDisabled(false);
        control->refresh();

        pmap[child] = control;
    }

    return form;
}

Control* ExposerBackend::exposeControl(const QString& rootPath, const QPointF& pos, QString sourceSuid, Control* parentControl, QString destinationPath, QString destinationSuid) const
{
    PreviewerBackend::instance()->setDisabled(true);
    auto control = new Control(rootPath + separator() + DIR_THIS + separator() + "main.qml");

    SaveBackend::instance()->addControl(control, parentControl, destinationSuid, destinationPath);
    control->setParentItem(parentControl);
    control->setPos(pos);

    SaveUtils::setX(control->dir(), control->x());
    SaveUtils::setY(control->dir(), control->y());

    PreviewerBackend::instance()->setDisabled(false);
    control->refresh();

    QMap<QString, Control*> pmap;
    pmap[control->dir()] = control;
    for (const auto& child : SaveUtils::childrenPaths(control->dir(), sourceSuid)) {
        auto pcontrol = pmap.value(dname(dname(child)));

        PreviewerBackend::instance()->setDisabled(true);
        auto ccontrol = new Control(child + separator() + DIR_THIS + separator() + "main.qml");
        ccontrol->setParentItem(pcontrol);
        control->setPos(pos);

        SaveUtils::setX(control->dir(), control->x());
        SaveUtils::setY(control->dir(), control->y());

        PreviewerBackend::instance()->setDisabled(false);
        ccontrol->refresh();

        pmap[child] = ccontrol;
    }

    return control;
}