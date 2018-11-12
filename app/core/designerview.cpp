#include <designerview.h>
#include <designerscene.h>
#include <saveutils.h>
#include <controlcreationmanager.h>
#include <controlremovingmanager.h>
#include <controlpropertymanager.h>
#include <QMenu>

DesignerView::DesignerView(DesignerScene* scene, QWidget* parent) : QGraphicsView(scene, parent)
  , m_menu(new QMenu(this))
  , m_sendBackAct(new QAction(this))
  , m_bringFrontAct(new QAction(this))
  , m_undoAct(new QAction(this))
  , m_redoAct(new QAction(this))
  , m_cutAct(new QAction(this))
  , m_copyAct(new QAction(this))
  , m_pasteAct(new QAction(this))
  , m_selectAllAct(new QAction(this))
  , m_deleteAct(new QAction(this))
  , m_moveUpAct(new QAction(this))
  , m_moveDownAct(new QAction(this))
  , m_moveRightAct(new QAction(this))
  , m_moveLeftAct(new QAction(this))
{
    setStyleSheet("DesignerView { background: transparent }");

    m_sendBackAct->setText("Send to Back");
    m_bringFrontAct->setText("Bring to Front");
    m_undoAct->setText("Undo");
    m_undoAct->setShortcut(QKeySequence::Undo);
    m_redoAct->setText("Redo");
    m_redoAct->setShortcut(QKeySequence::Redo);
    m_cutAct->setText("Cut");
    m_cutAct->setShortcut(QKeySequence::Cut);
    m_copyAct->setText("Copy");
    m_copyAct->setShortcut(QKeySequence::Copy);
    m_pasteAct->setText("Paste");
    m_pasteAct->setShortcut(QKeySequence::Paste);
    m_deleteAct->setText("Delete");
    m_deleteAct->setShortcut(QKeySequence::Delete);
#if defined(Q_OS_MACOS)
    m_deleteAct->setShortcuts(QList<QKeySequence>() << (Qt::CTRL + Qt::Key_Backspace) << QKeySequence::Delete);
#endif
    m_selectAllAct->setText("Select All");
    m_selectAllAct->setShortcut(QKeySequence::SelectAll);
    m_moveUpAct->setShortcut(Qt::Key_Up);
    m_moveDownAct->setShortcut(Qt::Key_Down);
    m_moveRightAct->setShortcut(Qt::Key_Right);
    m_moveLeftAct->setShortcut(Qt::Key_Left);

    m_menu->addAction(m_sendBackAct);
    m_menu->addAction(m_bringFrontAct);
    m_menu->addSeparator();
    m_menu->addAction(m_undoAct);
    m_menu->addAction(m_redoAct);
    m_menu->addSeparator();
    m_menu->addAction(m_cutAct);
    m_menu->addAction(m_copyAct);
    m_menu->addAction(m_pasteAct);
    m_menu->addAction(m_deleteAct);
    m_menu->addSeparator();
    m_menu->addAction(m_selectAllAct);

    connect(m_undoAct, SIGNAL(triggered()), SLOT(onUndoAction()));
    connect(m_redoAct, SIGNAL(triggered()), SLOT(onRedoAction()));
    connect(m_cutAct, SIGNAL(triggered()), SLOT(onCutAction()));
    connect(m_copyAct, SIGNAL(triggered()), SLOT(onCopyAction()));
    connect(m_pasteAct, SIGNAL(triggered()), SLOT(onPasteAction()));
    connect(m_deleteAct, SIGNAL(triggered()), SLOT(onDeleteAction()));
    connect(m_selectAllAct, SIGNAL(triggered()), SLOT(onSelectAllAction()));
    connect(m_moveUpAct, SIGNAL(triggered()), SLOT(onMoveUpAction()));
    connect(m_moveDownAct, SIGNAL(triggered()), SLOT(onMoveDownAction()));
    connect(m_moveRightAct, SIGNAL(triggered()), SLOT(onMoveRightAction()));
    connect(m_moveLeftAct, SIGNAL(triggered()), SLOT(onMoveLeftAction()));
    connect(m_sendBackAct, SIGNAL(triggered()), SLOT(onSendBackAction()));
    connect(m_bringFrontAct, SIGNAL(triggered()), SLOT(onBringFrontAction()));

    addAction(m_undoAct);
    addAction(m_redoAct);
    addAction(m_cutAct);
    addAction(m_copyAct);
    addAction(m_pasteAct);
    addAction(m_deleteAct);
    addAction(m_selectAllAct);
    addAction(m_moveUpAct);
    addAction(m_moveDownAct);
    addAction(m_moveRightAct);
    addAction(m_moveLeftAct);
}

DesignerScene* DesignerView::scene() const
{
    return static_cast<DesignerScene*>(QGraphicsView::scene());
}

void DesignerView::resizeEvent(QResizeEvent* event)
{
    QGraphicsView::resizeEvent(event);
    scene()->centralize();
}

void DesignerView::contextMenuEvent(QContextMenuEvent* event)
{
    QGraphicsView::contextMenuEvent(event);

    auto selectedControls = scene()->selectedControls();
    selectedControls.removeOne(scene()->currentForm());

    if (selectedControls.isEmpty()) {
        m_sendBackAct->setDisabled(true);
        m_bringFrontAct->setDisabled(true);
        m_cutAct->setDisabled(true);
        m_copyAct->setDisabled(true);
        m_deleteAct->setDisabled(true);
    } else {
        m_sendBackAct->setDisabled(false);
        m_bringFrontAct->setDisabled(false);
        m_cutAct->setDisabled(false);
        m_copyAct->setDisabled(false);
        m_deleteAct->setDisabled(false);
    }
    m_menu->exec(event->globalPos());
}

void DesignerView::discharge()
{
    update();
}

void DesignerView::onUndoAction()
{
    //TODO
}

void DesignerView::onRedoAction()
{
    //TODO
}

namespace {
struct CopyPaste final
{
    enum ActionType { Invalid, Copy, Cut };

    CopyPaste() = delete;
    CopyPaste(const CopyPaste&) = delete;
    CopyPaste &operator=(const CopyPaste&) = delete;

    static inline QString suid()
    { return s_suid; }
    static inline ActionType actionType()
    { return s_actionType; }
    static inline QList<QPointer<Control>> controls()
    { return s_controls; }
    static inline bool isValid()
    { return s_actionType != Invalid && !s_controls.isEmpty() && !s_suid.isEmpty(); }
    static inline void invalidate()
    { s_actionType = Invalid; s_controls.clear(); s_suid.clear(); }
    static inline void setControls(const QList<QPointer<Control>>& value, const QString& suid, ActionType actionType)
    { s_controls = value; s_suid = suid; s_actionType = actionType; }

private:
    static QString s_suid;
    static ActionType s_actionType;
    static QList<QPointer<Control>> s_controls;
};
QString CopyPaste::s_suid;
QList<QPointer<Control>> CopyPaste::s_controls;
CopyPaste::ActionType CopyPaste::s_actionType = CopyPaste::Invalid;
}

void DesignerView::onCutAction()
{
    QList<Control*> controlsForRemoval;
    QList<Control*> controls(scene()->selectedControls());

    controlsForRemoval.append(scene()->currentForm());
    for (Control* control : controls) {
        for (Control* ctrl : controls) {
            if (control->childControls().contains(ctrl))
                controlsForRemoval.append(ctrl);
        }
    }

    for (Control* control : controlsForRemoval)
        controls.removeOne(control);

    QList<QPointer<Control>> controlPtrList;
    for (Control* control : controls)
        controlPtrList.append(QPointer<Control>(control));

    CopyPaste::setControls(controlPtrList, scene()->currentForm()->uid(), CopyPaste::Cut);
}

void DesignerView::onCopyAction()
{
    QList<Control*> controlsForRemoval;
    QList<Control*> controls(scene()->selectedControls());

    controlsForRemoval.append(scene()->currentForm());
    for (Control* control : controls) {
        for (Control* ctrl : controls) {
            if (control->childControls().contains(ctrl))
                controlsForRemoval.append(ctrl);
        }
    }

    for (Control* control : controlsForRemoval)
        controls.removeOne(control);

    QList<QPointer<Control>> controlPtrList;
    for (Control* control : controls)
        controlPtrList.append(QPointer<Control>(control));

    CopyPaste::setControls(controlPtrList, scene()->currentForm()->uid(), CopyPaste::Copy);
}

// FIXME:
void DesignerView::onPasteAction()
{
    if (!CopyPaste::isValid())
        return;

    const QString& suid = CopyPaste::suid();
    const CopyPaste::ActionType actionType = CopyPaste::actionType();
    const QList<QPointer<Control>>& controls = CopyPaste::controls();
    Form* currentForm = scene()->currentForm();

    if (actionType == CopyPaste::Cut)
        CopyPaste::invalidate();

    scene()->clearSelection();

    for (const QPointer<Control>& control : controls) {
        if (control.isNull())
            continue;
        Control* newControl = ControlCreationManager::createControl(control->dir(),
                                                                    control->pos() + QPointF(5, 5),
                                                                    suid, currentForm,
                                                                    currentForm->dir(),
                                                                    currentForm->uid());
        newControl->setSelected(true);
    }

    QList<Control*> controlsToRemove;
    for (const QPointer<Control>& control : controls) {
        if (control)
            controlsToRemove.append(control.data());
    }

    if (actionType == CopyPaste::Cut)
        ControlRemovingManager::removeControls(controlsToRemove);
}

void DesignerView::onDeleteAction()
{ //FIXME: Do not delete if docs are open within QML Editor
    QList<Control*> selectedControls = scene()->selectedControls();
    selectedControls.removeAll(scene()->currentForm());

    ControlRemovingManager::removeControls(selectedControls);
}

void DesignerView::onSelectAllAction()
{
    auto currentForm = scene()->currentForm();
    for (auto control : currentForm->childControls())
        control->setSelected(true);
}

void DesignerView::onMoveUpAction()
{
    auto selectedControls = scene()->selectedControls();
    selectedControls.removeOne(scene()->currentForm());
    for (auto control : selectedControls)
        control->moveBy(0, - 1);
}

void DesignerView::onMoveDownAction()
{
    auto selectedControls = scene()->selectedControls();
    selectedControls.removeOne(scene()->currentForm());
    for (auto control : selectedControls)
        control->moveBy(0, 1);
}

void DesignerView::onMoveRightAction()
{
    auto selectedControls = scene()->selectedControls();
    selectedControls.removeOne(scene()->currentForm());
    for (auto control : selectedControls)
        control->moveBy(1, 0);
}

void DesignerView::onMoveLeftAction()
{
    auto selectedControls = scene()->selectedControls();
    selectedControls.removeOne(scene()->currentForm());
    for (auto control : selectedControls)
        control->moveBy(- 1, 0);
}

void DesignerView::onSendBackAction()
{
    auto selectedControls = scene()->selectedControls();
    selectedControls.removeOne(scene()->currentForm());
    for (auto control : selectedControls) {
        ControlPropertyManager::setZ(control, scene()->currentForm()->lowerZValue() - 1,
                                     ControlPropertyManager::SaveChanges
                                     | ControlPropertyManager::UpdatePreviewer);
    }
}

void DesignerView::onBringFrontAction()
{
    auto selectedControls = scene()->selectedControls();
    selectedControls.removeOne(scene()->currentForm());
    for (auto control : selectedControls) {
        ControlPropertyManager::setZ(control, scene()->currentForm()->higherZValue() + 1,
                                     ControlPropertyManager::SaveChanges
                                     | ControlPropertyManager::UpdatePreviewer);
    }
}
