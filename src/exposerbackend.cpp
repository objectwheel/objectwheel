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
    auto fpaths = SaveUtils::formPaths(ProjectBackend::instance()->dir());

    qreal j = 0;
    for (auto path : fpaths) {
        auto form = new Form(path + separator() + DIR_THIS + separator() + "main.qml");

        if (SaveUtils::isMain(path))
            form->setMain(true);

        m_designerScene->addForm(form);

        QMap<QString, Control*> pmap;
        pmap[path] = form;

        int i = 1;
        const auto& pths = SaveUtils::childrenPaths(path);
        for (auto child : pths) {
            auto pcontrol = pmap.value(dname(dname(child)));
            auto control = new Control(child + separator() + DIR_THIS + separator() + "main.qml");
            control->setParentItem(pcontrol);
            pmap[child] = control;
            i++;
        }
        j++;
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