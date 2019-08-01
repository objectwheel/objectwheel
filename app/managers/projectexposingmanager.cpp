#include <projectexposingmanager.h>
#include <controlpropertymanager.h>
#include <controlremovingmanager.h>
#include <controlrenderingmanager.h>
#include <saveutils.h>
#include <parserutils.h>
#include <projectmanager.h>
#include <control.h>
#include <form.h>
#include <designerscene.h>

ProjectExposingManager* ProjectExposingManager::s_instance = nullptr;
DesignerScene* ProjectExposingManager::s_designerScene = nullptr;

ProjectExposingManager::ProjectExposingManager(QObject* parent) : QObject(parent)
{
    s_instance = this;
}

ProjectExposingManager::~ProjectExposingManager()
{
    s_instance = nullptr;
}

ProjectExposingManager* ProjectExposingManager::instance()
{
    return s_instance;
}

void ProjectExposingManager::init(DesignerScene* designerScene)
{
    s_designerScene = designerScene;
}

void ProjectExposingManager::exposeProject()
{
    Form* firstForm = nullptr;
    QMap<Control*, Control*> firstChilds;

    for (const QString& formPath : SaveUtils::formPaths(ProjectManager::dir())) {
        auto form = new Form;
        form->setDir(formPath);
        form->setUid(SaveUtils::controlUid(form->dir()));
        form->setDevicePixelRatio(ControlRenderingManager::devicePixelRatio());
        ControlPropertyManager::setId(form, ParserUtils::id(form->dir()), ControlPropertyManager::NoOption);
        ControlPropertyManager::setIndex(form, SaveUtils::controlIndex(form->dir()), ControlPropertyManager::NoOption);

        // Since SaveUtils::formPaths sorts out the form paths based
        // on indexes, thus the first form is the one with lower index
        if (!firstForm)
            firstForm = form;

        if (form->id().isEmpty())
            ControlPropertyManager::setId(form, "form", ControlPropertyManager::SaveChanges);

        if (form->id() != SaveUtils::controlId(form->dir()))
            SaveUtils::setProperty(form->dir(), SaveUtils::ControlId, form->id());

        s_designerScene->addForm(form);

        connect(ControlRenderingManager::instance(), &ControlRenderingManager::renderDone,
                form, &Control::setRenderInfo);
        connect(ControlRenderingManager::instance(), &ControlRenderingManager::devicePixelRatioChanged,
                form, &Control::setDevicePixelRatio);
        connect(form, &Control::doubleClicked,
                form, [=] { ControlPropertyManager::instance()->doubleClicked(form); });
        connect(form, &Control::renderInfoChanged,
                form, [=] (bool c) { ControlPropertyManager::instance()->renderInfoChanged(form, c); });

        QMap<QString, Control*> controlTree;
        controlTree.insert(formPath, form);
        emit instance()->controlExposed(form);

        for (const QString& childPath : SaveUtils::childrenPaths(formPath)) {
            Control* parentControl = controlTree.value(SaveUtils::toDoubleUp(childPath));
            Q_ASSERT(parentControl);

            auto control = new Control;
            control->setDir(childPath);
            control->setUid(SaveUtils::controlUid(control->dir()));
            control->setDevicePixelRatio(ControlRenderingManager::devicePixelRatio());
            ControlPropertyManager::setId(control, ParserUtils::id(control->dir()), ControlPropertyManager::NoOption);
            ControlPropertyManager::setIndex(control, SaveUtils::controlIndex(control->dir()), ControlPropertyManager::NoOption);

            // For non-gui items; others aren't affected, since
            // render info update is going to happen and set position,
            // but that doesn't happen for non-gui controls, in this
            // way we expose non-gui items into right positions
            ControlPropertyManager::setPos(control, SaveUtils::designPosition(childPath),
                                           ControlPropertyManager::NoOption);

            // Since SaveUtils::childrenPaths sorts out the siblings based
            // on indexes, thus the first control is the one with lower index
            if (!firstChilds.contains(parentControl))
                firstChilds.insert(parentControl, control);

            if (control->id().isEmpty())
                ControlPropertyManager::setId(control, "control", ControlPropertyManager::SaveChanges);

            if (control->id() != SaveUtils::controlId(control->dir()))
                SaveUtils::setProperty(control->dir(), SaveUtils::ControlId, control->id());

            ControlPropertyManager::setParent(control, parentControl, ControlPropertyManager::NoOption);

            connect(ControlRenderingManager::instance(), &ControlRenderingManager::renderDone,
                    control, &Control::setRenderInfo);
            connect(ControlRenderingManager::instance(), &ControlRenderingManager::devicePixelRatioChanged,
                    control, &Control::setDevicePixelRatio);
            connect(control, &Control::doubleClicked,
                    control, [=] { ControlPropertyManager::instance()->doubleClicked(control); });
            connect(control, &Control::renderInfoChanged,
                    control, [=] (bool c) { ControlPropertyManager::instance()->renderInfoChanged(control, c); });

            controlTree.insert(childPath, control);

            emit instance()->controlExposed(control);
        }
    }

    s_designerScene->setCurrentForm(firstForm);

    // Let the SaveManager fix (rebuild) indexes
    // Make sure setIndex is called after controls put upon DesignerScene
    ControlPropertyManager::setIndex(firstForm, 0, ControlPropertyManager::SaveChanges);
    for (Control* firstChild : firstChilds)
        ControlPropertyManager::setIndex(firstChild, 0, ControlPropertyManager::SaveChanges);

    auto conn = new QMetaObject::Connection;
    *conn = connect(ControlPropertyManager::instance(), &ControlPropertyManager::renderInfoChanged,
                    ControlPropertyManager::instance(), [conn] (Control* ctrl, bool) {
            if (ctrl == s_designerScene->currentForm()) {
            QObject::disconnect(*conn);
            delete conn;
            s_designerScene->shrinkSceneRect();
}
}, Qt::QueuedConnection);
}

void ProjectExposingManager::removeProject()
{
    for (Form* form : s_designerScene->forms())
        ControlRemovingManager::removeControl(form, false);
}
