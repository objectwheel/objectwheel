#include <controlproductionmanager.h>
#include <form.h>
#include <designerscene.h>
#include <saveutils.h>
#include <parserutils.h>
#include <savemanager.h>
#include <controlrenderingmanager.h>
#include <controlpropertymanager.h>
#include <filesystemutils.h>
#include <QTemporaryDir>

ControlProductionManager* ControlProductionManager::s_instance = nullptr;
DesignerScene* ControlProductionManager::s_designerScene = nullptr;

ControlProductionManager::ControlProductionManager(QObject* parent) : QObject(parent)
{
    s_instance = this;
}

ControlProductionManager::~ControlProductionManager()
{
    s_instance = nullptr;
}

ControlProductionManager* ControlProductionManager::instance()
{
    return s_instance;
}

void ControlProductionManager::init(DesignerScene* designerScene)
{
    s_designerScene = designerScene;
}

Form* ControlProductionManager::produceForm(const QString& formRootPath, const QString& module)
{
    const QString& newFormRootPath = SaveManager::addForm(formRootPath);
    if (newFormRootPath.isEmpty()) {
        qWarning("ControlProductionManager::produceForm: Failed.");
        return nullptr;
    }

    auto form = new Form;
    form->setDir(newFormRootPath);
    form->setModule(module);
    form->setUid(SaveUtils::controlUid(form->dir()));
    ControlPropertyManager::setId(form, ParserUtils::id(form->dir()), ControlPropertyManager::NoOption);

    s_designerScene->addForm(form);
    s_designerScene->setCurrentForm(form);

    SaveManager::setupFormConnections(form->dir());

    // NOTE: We don't have to call ControlPropertyManager::setParent, since there is no valid
    // parent concept for forms in Designer; forms are directly put into DesignerScene

    ControlPropertyManager::setIndex(form, form->siblings().size(), ControlPropertyManager::SaveChanges);
    connect(ControlRenderingManager::instance(), &ControlRenderingManager::renderDone,
            form, &Control::setRenderInfo);
    connect(form, &Control::doubleClicked,
            form, [=] (Qt::MouseButtons b) { ControlPropertyManager::instance()->doubleClicked(form, b); });
    connect(form, &Control::renderInfoChanged,
            form, [=] (bool c) { ControlPropertyManager::instance()->renderInfoChanged(form, c); });
    // Made it Qt::QueuedConnection in order to prevent
    // mouseUngrabEvent to call beingDraggedChanged or
    // beingResizedChanged, thus preventing
    // applyGeometryCorrection to be called before
    // dropControl called, since it resets geometry correction
    // otherwise may setSceneRect() in DesignerScene
    // triggers and extends scene rect
    connect(form, &Control::beingDraggedChanged,
            form, &Control::syncGeometry, Qt::QueuedConnection);
    connect(form, &Control::beingResizedChanged,
            form, &Control::syncGeometry, Qt::QueuedConnection);
    ControlRenderingManager::scheduleFormCreation(form->dir(), form->module());

    // NOTE: We don't have to worry about possible child controls since produceForm is only
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

    emit instance()->controlProduced(form);

    return form;
}

Control* ControlProductionManager::produceControl(Control* targetParentControl,
                                               const QString& controlRootPath,
                                               const QString& module,
                                               const QPointF& pos,
                                               const QSizeF& initialSize,
                                               const QPixmap& initialPixmap)
{
    QString newControlRootPath;
    if (controlRootPath == targetParentControl->dir()) {
        QTemporaryDir tmpDir;
        if (tmpDir.isValid()) {
            if (!FileSystemUtils::copy(controlRootPath, tmpDir.path(), true)) {
                qWarning("SaveManager::addControl: Failed. Cannot copy the control to its new root path.");
                return nullptr;
            }
            newControlRootPath = SaveManager::addControl(tmpDir.path(), controlRootPath);
        }
    } else {
        newControlRootPath = SaveManager::addControl(controlRootPath, targetParentControl->dir());
    }

    if (newControlRootPath.isEmpty()) {
        qWarning("ControlProductionManager::produceControl: Failed.");
        return nullptr;
    }

    auto control = new Control;
    control->setDir(newControlRootPath);
    control->setModule(module);
    control->setUid(SaveUtils::controlUid(control->dir()));
    ControlPropertyManager::setId(control, ParserUtils::id(control->dir()), ControlPropertyManager::NoOption);
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
    control->setPixmap(initialPixmap);
    ControlPropertyManager::setPos(control, pos, ControlPropertyManager::SaveChanges);
    ControlPropertyManager::setSize(control, initialSize, ControlPropertyManager::NoOption);
    ControlPropertyManager::setIndex(control, control->siblings().size(), ControlPropertyManager::SaveChanges);
    connect(ControlRenderingManager::instance(), &ControlRenderingManager::renderDone,
            control, &Control::setRenderInfo);
    connect(control, &Control::doubleClicked,
            control, [=] (Qt::MouseButtons b) { ControlPropertyManager::instance()->doubleClicked(control, b); });
    connect(control, &Control::renderInfoChanged,
            control, [=] (bool c) { ControlPropertyManager::instance()->renderInfoChanged(control, c); });
    // Made it Qt::QueuedConnection in order to prevent
    // mouseUngrabEvent to call beingDraggedChanged or
    // beingResizedChanged, thus preventing
    // applyGeometryCorrection to be called before
    // dropControl called, since it resets geometry correction
    // otherwise may setSceneRect() in DesignerScene
    // triggers and extends scene rect
    connect(control, &Control::beingDraggedChanged,
            control, &Control::syncGeometry, Qt::QueuedConnection);
    connect(control, &Control::beingResizedChanged,
            control, &Control::syncGeometry, Qt::QueuedConnection);

    ControlRenderingManager::scheduleControlCreation(control->dir(), control->module(), targetParentControl->uid());

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
    emit instance()->controlProduced(control);

    for (const QString& childPath : SaveUtils::childrenPaths(control->dir())) {
        Control* parentControl = controlTree.value(SaveUtils::toDoubleUp(childPath));
        Q_ASSERT(parentControl);

        auto childControl = new Control;
        childControl->setDir(childPath);
        childControl->setModule(ParserUtils::module(childPath));
        childControl->setUid(SaveUtils::controlUid(childControl->dir()));
        ControlPropertyManager::setId(childControl, ParserUtils::id(childControl->dir()), ControlPropertyManager::NoOption);
        ControlPropertyManager::setParent(childControl, parentControl, ControlPropertyManager::NoOption);
        // For non-gui items; others aren't affected, since
        // render info update is going to happen and set position,
        // but that doesn't happen for non-gui controls, in this
        // way we expose non-gui items into right positions
        ControlPropertyManager::setPos(childControl, SaveUtils::designPosition(childPath), ControlPropertyManager::NoOption);
        ControlPropertyManager::setSize(childControl, SaveUtils::designSize(childPath), ControlPropertyManager::NoOption);
        ControlPropertyManager::setIndex(childControl, childControl->siblings().size(), ControlPropertyManager::SaveChanges);
        connect(ControlRenderingManager::instance(), &ControlRenderingManager::renderDone,
                childControl, &Control::setRenderInfo);
        connect(childControl, &Control::doubleClicked,
                childControl, [=] (Qt::MouseButtons b) { ControlPropertyManager::instance()->doubleClicked(childControl, b); });
        connect(childControl, &Control::renderInfoChanged,
                childControl, [=] (bool c) { ControlPropertyManager::instance()->renderInfoChanged(childControl, c); });
        // Made it Qt::QueuedConnection in order to prevent
        // mouseUngrabEvent to call beingDraggedChanged or
        // beingResizedChanged, thus preventing
        // applyGeometryCorrection to be called before
        // dropControl called, since it resets geometry correction
        // otherwise may setSceneRect() in DesignerScene
        // triggers and extends scene rect
        connect(childControl, &Control::beingDraggedChanged,
                childControl, &Control::syncGeometry, Qt::QueuedConnection);
        connect(childControl, &Control::beingResizedChanged,
                childControl, &Control::syncGeometry, Qt::QueuedConnection);

        ControlRenderingManager::scheduleControlCreation(childControl->dir(), childControl->module(), parentControl->uid());

        controlTree.insert(childPath, childControl);

        emit instance()->controlProduced(childControl);
    }

    return control;
}