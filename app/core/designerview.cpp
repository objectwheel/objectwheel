#include <designerview.h>
#include <designerscene.h>
#include <saveutils.h>
#include <controlcreationmanager.h>
#include <controlremovingmanager.h>
#include <controlpropertymanager.h>
#include <controlpreviewingmanager.h>
#include <QMenu>

// TODO: Improve copy-paste positioning. "Pasting into a sub control and" "positioning wherever you
// right click".. implement those 2 features in future

namespace {

struct CopyPaste final
{
    enum ActionType { Invalid, Copy, Cut };

    CopyPaste() = delete;
    CopyPaste(const CopyPaste&) = delete;
    CopyPaste &operator=(const CopyPaste&) = delete;

    static inline ActionType actionType()
    { return s_actionType; }
    static inline QList<QPointer<Control>> controls()
    { return s_controls; }
    static inline bool isValid()
    { return s_actionType != Invalid && !s_controls.isEmpty(); }
    static inline void invalidate()
    { s_actionType = Invalid; s_controls.clear(); }
    static inline void setControls(const QList<QPointer<Control>>& value, ActionType actionType)
    { s_controls = value; s_actionType = actionType; }

private:
    static ActionType s_actionType;
    static QList<QPointer<Control>> s_controls;
};
QList<QPointer<Control>> CopyPaste::s_controls;
CopyPaste::ActionType CopyPaste::s_actionType = CopyPaste::Invalid;
}

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
    viewport()->setAutoFillBackground(false);

    m_sendBackAct->setText(tr("Send to Back"));
    m_bringFrontAct->setText(tr("Bring to Front"));
    m_undoAct->setText(tr("Undo"));
    m_undoAct->setShortcut(QKeySequence::Undo);
    m_redoAct->setText(tr("Redo"));
    m_redoAct->setShortcut(QKeySequence::Redo);
    m_cutAct->setText(tr("Cut"));
    m_cutAct->setShortcut(QKeySequence::Cut);
    m_copyAct->setText(tr("Copy"));
    m_copyAct->setShortcut(QKeySequence::Copy);
    m_pasteAct->setText(tr("Paste"));
    m_pasteAct->setShortcut(QKeySequence::Paste);
    m_deleteAct->setText(tr("Delete"));
    m_deleteAct->setShortcut(QKeySequence::Delete);
#if defined(Q_OS_MACOS)
    m_deleteAct->setShortcuts(QList<QKeySequence>() << (Qt::CTRL + Qt::Key_Backspace) << QKeySequence::Delete);
#endif
    m_selectAllAct->setText(tr("Select All"));
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

    connect(m_undoAct, &QAction::triggered, this, &DesignerView::onUndoAction);
    connect(m_redoAct, &QAction::triggered, this, &DesignerView::onRedoAction);
    connect(m_cutAct, &QAction::triggered, this, &DesignerView::onCutAction);
    connect(m_copyAct, &QAction::triggered, this, &DesignerView::onCopyAction);
    connect(m_pasteAct, &QAction::triggered, this, &DesignerView::onPasteAction);
    connect(m_deleteAct, &QAction::triggered, this, &DesignerView::onDeleteAction);
    connect(m_selectAllAct, &QAction::triggered, this, &DesignerView::onSelectAllAction);
    connect(m_moveUpAct, &QAction::triggered, this, &DesignerView::onMoveUpAction);
    connect(m_moveDownAct, &QAction::triggered, this, &DesignerView::onMoveDownAction);
    connect(m_moveRightAct, &QAction::triggered, this, &DesignerView::onMoveRightAction);
    connect(m_moveLeftAct, &QAction::triggered, this, &DesignerView::onMoveLeftAction);
    connect(m_sendBackAct, &QAction::triggered, this, &DesignerView::onSendBackAction);
    connect(m_bringFrontAct, &QAction::triggered, this, &DesignerView::onBringFrontAction);

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

void DesignerView::onCutAction()
{
    QList<Control*> controls(scene()->selectedControls());
    controls.removeOne(scene()->currentForm());

    const QList<Control*> copy(controls);
    for (const Control* control : copy) {
        for (Control* childControl : control->childControls())
            controls.removeOne(childControl);
    }

    QList<QPointer<Control>> controlPtrList;
    for (Control* control : controls)
        controlPtrList.append(QPointer<Control>(control));

    CopyPaste::setControls(controlPtrList, CopyPaste::Cut);
}

void DesignerView::onCopyAction()
{
    QList<Control*> controls(scene()->selectedControls());
    controls.removeOne(scene()->currentForm());

    const QList<Control*> copy(controls);
    for (const Control* control : copy) {
        for (Control* childControl : control->childControls())
            controls.removeOne(childControl);
    }

    QList<QPointer<Control>> controlPtrList;
    for (Control* control : controls)
        controlPtrList.append(QPointer<Control>(control));

    CopyPaste::setControls(controlPtrList, CopyPaste::Copy);
}

void DesignerView::onPasteAction()
{
    if (!CopyPaste::isValid())
        return;

    const CopyPaste::ActionType actionType = CopyPaste::actionType();
    const QList<QPointer<Control>>& controls = CopyPaste::controls();

    if (actionType == CopyPaste::Cut)
        CopyPaste::invalidate();

    scene()->clearSelection();

    for (const QPointer<Control>& control : controls) {
        if (control.isNull())
            continue;
        Q_ASSERT(!control->form());

        Control* newControl = nullptr;
        if (actionType == CopyPaste::Cut) {
            ControlPropertyManager::setParent(control, scene()->currentForm(),
                                              ControlPropertyManager::SaveChanges
                                              | ControlPropertyManager::UpdatePreviewer);
            ControlPreviewingManager::scheduleRefresh(scene()->currentForm()->uid());
        } else {
            newControl = ControlCreationManager::createControl(scene()->currentForm(),
                                                               control->dir(),
                                                               control->pos() + QPointF(5, 5));
        }

        if (newControl)
            newControl->setSelected(true);
    }
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
