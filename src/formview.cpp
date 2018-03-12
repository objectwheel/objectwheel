#include <formview.h>
#include <formscene.h>

#include <QMenu>
#include <QAction>
#include <QScrollBar>

FormView::FormView(FormScene* scene, QWidget* parent) : QGraphicsView(scene, parent)
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
    verticalScrollBar()->setStyleSheet(CSS::ScrollBar);
    horizontalScrollBar()->setStyleSheet(CSS::ScrollBarH);

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
    m_deleteAct->setShortcut(Qt::Key_Control | Qt::Key_Backspace); //TODO: Test this
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
    connect(m_sendBackAct, SIGNAL(triggered()), SLOT(onSendBackActAction()));
    connect(m_bringFrontAct, SIGNAL(triggered()), SLOT(onBringFrontActAction()));

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

FormScene* FormView::scene() const
{
    return static_cast<FormScene*>(QGraphicsView::scene());
}

void FormView::resizeEvent(QResizeEvent* event)
{
    QGraphicsView::resizeEvent(event);

    auto mainForm = scene()->mainForm();
    if (mainForm)
        mainForm->centralize();
}

void FormView::contextMenuEvent(QContextMenuEvent* event)
{
    QGraphicsView::contextMenuEvent(event);

    auto selectedControls = scene()->selectedControls();
    selectedControls.removeOne(scene()->mainForm());

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
        for (auto sc : selectedControls) {
            if (sc->gui() == false) {
                m_sendBackAct->setDisabled(true);
                m_bringFrontAct->setDisabled(true);
            }
        }
    }
    m_menu->exec(event->globalPos());
}

void FormView::onUndoAction()
{
    //TODO
}

void FormView::onRedoAction()
{
    //TODO
}

void FormView::onCutAction()
{
    QList<QUrl> urls;
    QByteArray controls;
    QDataStream dstream(&controls, QIODevice::WriteOnly);
    auto mimeData = new QMimeData;
    auto clipboard = QApplication::clipboard();
    auto selectedControls = scene()->selectedControls();
    selectedControls.removeOne(scene()->mainForm());
    mimeData->setData("objectwheel/uid", scene()->mainForm()->uid().toUtf8());
    mimeData->setData("objectwheel/cut", "1");

    for (auto control : selectedControls)
        for (auto ctrl : selectedControls)
            if (control->childControls().contains(ctrl))
                selectedControls.removeAll(ctrl);

    for (auto control : selectedControls) {
        urls << QUrl::fromLocalFile(control->dir());
        dstream << (quint64)control;
    }

    mimeData->setData("objectwheel/dstreamsize", QString::number(selectedControls.size()).toUtf8());
    mimeData->setData("objectwheel/dstream", controls);
    mimeData->setUrls(urls);
    mimeData->setText(TOOL_KEY);
    clipboard->setMimeData(mimeData);
}

void FormView::onCopyAction()
{
    QList<QUrl> urls;
    auto mimeData = new QMimeData;
    auto clipboard = QApplication::clipboard();
    auto selectedControls = scene()->selectedControls();
    selectedControls.removeOne(scene()->mainForm());
    mimeData->setData("objectwheel/uid", scene()->mainForm()->uid().toUtf8());

    for (auto control : selectedControls)
        for (auto ctrl : selectedControls)
            if (control->childControls().contains(ctrl))
                selectedControls.removeAll(ctrl);

    for (auto control : selectedControls)
        urls << QUrl::fromLocalFile(control->dir());

    mimeData->setUrls(urls);
    mimeData->setText(TOOL_KEY);
    clipboard->setMimeData(mimeData);
}

void FormView::onPasteAction()
{
    auto clipboard = QApplication::clipboard();
    auto mimeData = clipboard->mimeData();
    auto mainForm = dW->formScene()->mainForm();
    QString uid = mimeData->data("objectwheel/uid");
    if (!mimeData->hasUrls() || !mimeData->hasText() ||
        mimeData->text() != TOOL_KEY || uid.isEmpty())
        return;

    QList<Control*> controls;
    for (auto url : mimeData->urls()) {
        auto control = SaveBackend::instance()->exposeControl(url.toLocalFile(), uid);
        SaveBackend::instance()->addControl(control, mainForm, mainForm->uid(), mainForm->dir());
        control->setParentItem(mainForm);
        controls << control;

        control->setPos(control->pos() + QPoint(fit::fx(5), fit::fx(5)));
        if (url == mimeData->urls().last()) {
            dW->formScene()->clearSelection();
            for (auto control : controls)
                control->setSelected(true);

            if (!mimeData->data("objectwheel/cut").isEmpty()) {
                QDataStream dstream(mimeData->data("objectwheel/dstream"));
                int size = QString(mimeData->data("objectwheel/dstreamsize")).toInt();
                QList<Control*> cutControls;
                for (int i = 0; i < size; i++) {
                    quint64 buff;
                    dstream >> buff;
                    cutControls << (Control*)buff;
                }

                for (auto control : cutControls) {
                    scene()->removeControl(control);
                    SaveBackend::instance()->removeControl(control);
                }
            }
        }

        for (auto childControl : control->childControls())
            childControl->refresh();
    }
}

void FormView::onDeleteAction()
{ //FIXME: Do not delete if docs are open within QML Editor
    auto selectedControls = scene()->selectedControls();
    selectedControls.removeOne(scene()->mainForm());
    for (auto control : selectedControls) {
       scene()->removeControl(control);
       SaveBackend::instance()->removeControl(control);
    }
}

void FormView::onSelectAllAction()
{
    auto mainForm = scene()->mainForm();
    for (auto control : mainForm->childControls())
        control->setSelected(true);
}

void FormView::onMoveUpAction()
{
    auto selectedControls = scene()->selectedControls();
    selectedControls.removeOne(scene()->mainForm());
    for (auto control : selectedControls)
        control->moveBy(0, - fit::fx(1));
}

void FormView::onMoveDownAction()
{
    auto selectedControls = scene()->selectedControls();
    selectedControls.removeOne(scene()->mainForm());
    for (auto control : selectedControls)
        control->moveBy(0, fit::fx(1));
}

void FormView::onMoveRightAction()
{
    auto selectedControls = scene()->selectedControls();
    selectedControls.removeOne(scene()->mainForm());
    for (auto control : selectedControls)
        control->moveBy(fit::fx(1), 0);
}

void FormView::onMoveLeftAction()
{
    auto selectedControls = scene()->selectedControls();
    selectedControls.removeOne(scene()->mainForm());
    for (auto control : selectedControls)
        control->moveBy(- fit::fx(1), 0);
}

void FormView::onSendBackAction()
{
    auto selectedControls = scene()->selectedControls();
    selectedControls.removeOne(scene()->mainForm());
    for (auto control : selectedControls)
        control->setZValue(scene()->mainForm()->lowerZValue() - 1);
}

void FormView::onBringFrontAction()
{
    auto selectedControls = scene()->selectedControls();
    selectedControls.removeOne(scene()->mainForm());
    for (auto control : selectedControls)
        control->setZValue(scene()->mainForm()->higherZValue() + 1);
}