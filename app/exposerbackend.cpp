#include <exposerbackend.h>
#include <form.h>
#include <designerscene.h>
#include <saveutils.h>
#include <projectbackend.h>
#include <filemanager.h>
#include <savebackend.h>

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
    auto form = new Form(rootPath + separator() + DIR_THIS + separator() + "main.qml");

    if (SaveUtils::isMain(rootPath))
        form->setMain(true);

    QMap<QString, Control*> pmap;
    pmap[rootPath] = form;
    for (const auto& child : SaveUtils::childrenPaths(rootPath)) {
        auto pcontrol = pmap.value(dname(dname(child)));
        auto control = new Control(child + separator() + DIR_THIS + separator() + "main.qml");
        control->setParentItem(pcontrol);
        pmap[child] = control;
    }

    SaveBackend::instance()->addForm(form);
    m_designerScene->addForm(form);

    return form;
}

Control* ExposerBackend::exposeControl(const QString& rootPath, QString sourceSuid, Control* parentControl, QString destinationPath, QString destinationSuid) const
{
    auto control = new Control(rootPath + separator() + DIR_THIS + separator() + "main.qml");

    QMap<QString, Control*> pmap;
    pmap[rootPath] = control;
    for (const auto& child : SaveUtils::childrenPaths(rootPath, sourceSuid)) {
        auto pcontrol = pmap.value(dname(dname(child)));
        auto control = new Control(child + separator() + DIR_THIS + separator() + "main.qml");
        control->setParentItem(pcontrol);
        pmap[child] = control;
    }

    SaveBackend::instance()->addControl(control, parentControl, destinationSuid, destinationPath);
    control->setParentItem(parentControl);

    return control;
}