#include <controlcreationmanager.h>
#include <form.h>
#include <designerscene.h>
#include <saveutils.h>
#include <savemanager.h>
#include <controlrenderingmanager.h>
#include <controlpropertymanager.h>

DesignerScene* ControlCreationManager::s_designerScene = nullptr;

ControlCreationManager::ControlCreationManager(QObject* parent) : QObject(parent)
{
}

void ControlCreationManager::init(DesignerScene* designerScene)
{
    s_designerScene = designerScene;
}

Form* ControlCreationManager::createForm(const QString& formRootPath)
{
    const QString& newFormRootPath = SaveManager::addForm(formRootPath);
    if (newFormRootPath.isEmpty()) {
        qWarning("ControlCreationManager::createForm: Failed.");
        return nullptr;
    }

    auto form = new Form(newFormRootPath);

    s_designerScene->addForm(form);
    s_designerScene->setCurrentForm(form);

    SaveManager::setupFormConnections(form->dir());

    // NOTE: We don't have to call ControlPropertyManager::setParent, since there is no valid
    // parent concept for forms in Designer; forms are directly put into DesignerScene

    ControlPropertyManager::setIndex(form, form->siblings().size(), ControlPropertyManager::SaveChanges);
    ControlRenderingManager::scheduleFormCreation(form->dir());

    // NOTE: We don't have to worry about possible child controls since createForm is only
    // called from FormsPane

    auto conn = new QMetaObject::Connection;
    *conn = connect(ControlPropertyManager::instance(), &ControlPropertyManager::renderInfoChanged,
                    ControlPropertyManager::instance(), [conn] (Control* ctrl, bool) {
        if (ctrl == s_designerScene->currentForm()) {
            QObject::disconnect(*conn);
            delete conn;
            s_designerScene->shrinkSceneRect();
        }
    }, Qt::QueuedConnection);

    return form;
}

Control* ControlCreationManager::createControl(Control* targetParentControl, const QString& controlRootPath, const QPointF& pos)
{
    const QString& newControlRootPath = SaveManager::addControl(controlRootPath, targetParentControl->dir());
    if (newControlRootPath.isEmpty()) {
        qWarning("ControlCreationManager::createControl: Failed.");
        return nullptr;
    }

    auto control = new Control(newControlRootPath);
    ControlPropertyManager::setParent(control, targetParentControl, ControlPropertyManager::NoOption);
    // Since all the controls are "non-gui" at first, this
    // will only set DesignPosition property in design.meta
    // and actual designer position of the control. So we
    // gotta do another setPos after the first render
    // info update function for "gui" controls, because first
    // render info update will either set a zero position for
    // actual designer position if there is no initial position
    // in the main.qml file, or it will set the initial position
    // info in the main.qml file to actual designer position.
    // Therefore we need to do another setPos right after first
    // render info update, in this way, we will set the pos in
    // main.qml file and also correct render engine position,
    // and correct actual designer position of the control because
    // it is corrupted by the render engine lately.
    ControlPropertyManager::setPos(control, pos, ControlPropertyManager::SaveChanges);
    ControlPropertyManager::setIndex(control, control->siblings().size(), ControlPropertyManager::SaveChanges);
    ControlRenderingManager::scheduleControlCreation(control->dir(), targetParentControl->uid());

    QPointer<Control> ptr(control);
    auto conn = new QMetaObject::Connection;
    *conn = QObject::connect(ControlPropertyManager::instance(), &ControlPropertyManager::renderInfoChanged,
                             [ptr, conn, pos] (Control* ctrl, bool) {
        if (ctrl == ptr.data() || ptr.isNull()) {
            QObject::disconnect(*conn);
            delete conn;
            if (ptr && ptr->gui()) {
                ControlPropertyManager::setPos(ptr.data(), pos,
                                               ControlPropertyManager::SaveChanges |
                                               ControlPropertyManager::UpdateRenderer);
            }
        }
    });

    QMap<QString, Control*> controlTree;
    controlTree.insert(control->dir(), control);
    for (const QString& childPath : SaveUtils::childrenPaths(control->dir())) {
        Control* parentControl = controlTree.value(SaveUtils::toDoubleUp(childPath));
        Q_ASSERT(parentControl);

        auto childControl = new Control(childPath);
        ControlPropertyManager::setParent(childControl, parentControl, ControlPropertyManager::NoOption);
        // For non-gui items; others aren't affected, since
        // render info update is going to happen and set position,
        // but that doesn't happen for non-gui controls, in this
        // way we expose non-gui items into right positions
        ControlPropertyManager::setPos(control, SaveUtils::designPosition(childPath), ControlPropertyManager::NoOption);
        ControlPropertyManager::setIndex(childControl, childControl->siblings().size(), ControlPropertyManager::SaveChanges);
        ControlRenderingManager::scheduleControlCreation(childControl->dir(), parentControl->uid());
        controlTree.insert(childPath, childControl);
    }

    return control;
}