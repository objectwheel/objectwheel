#include <exposerbackend.h>
#include <control.h>
#include <designerscene.h>
#include <saveutils.h>
#include <projectbackend.h>
#include <filemanager.h>

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

        const auto& cpaths = SaveUtils::childrenPaths(path);
        for (const auto& child : cpaths) {
            auto pcontrol = pmap.value(dname(dname(child)));
            auto control = new Control(child + separator() + DIR_THIS + separator() + "main.qml");
            control->setParentItem(pcontrol);
            pmap[child] = control;
        }
    }
}

Control* ExposerBackend::exposeControl(const QString& rootPath, QString suid) const
{
    auto control = new Control(rootPath + separator() + DIR_THIS + separator() + "main.qml");

    QMap<QString, Control*> pmap;
    pmap[rootPath] = control;
    for (auto child : SaveUtils::childrenPaths(rootPath, suid)) {
        auto pcontrol = pmap.value(dname(dname(child)));
        auto control = new Control(child + separator() + DIR_THIS + separator() + "main.qml");
        control->setParentItem(pcontrol);
        pmap[child] = control;
    }

    return control;
}