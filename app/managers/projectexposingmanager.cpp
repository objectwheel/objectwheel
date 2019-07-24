#include <projectexposingmanager.h>
#include <saveutils.h>
#include <projectmanager.h>
#include <control.h>
#include <form.h>
#include <designerscene.h>
#include <controlpropertymanager.h>

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
        auto form = new Form(formPath);

        // Since SaveUtils::formPaths sorts out the form paths based
        // on indexes, thus the first form is the one with lower index
        if (!firstForm)
            firstForm = form;

        if (form->id().isEmpty())
            ControlPropertyManager::setId(form, "form", ControlPropertyManager::SaveChanges);

        if (form->id() != SaveUtils::controlId(form->dir()))
            SaveUtils::setProperty(form->dir(), SaveUtils::ControlId, form->id());

        s_designerScene->addForm(form);

        QMap<QString, Control*> controlTree;
        controlTree.insert(formPath, form);
        emit instance()->controlExposed(form);

        for (const QString& childPath : SaveUtils::childrenPaths(formPath)) {
            Control* parentControl = controlTree.value(SaveUtils::toDoubleUp(childPath));
            Q_ASSERT(parentControl);

            auto control = new Control(childPath);
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

            controlTree.insert(childPath, control);

            emit instance()->controlExposed(control);
        }
    }

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
