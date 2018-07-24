#include <designerview.h>
#include <designerscene.h>
#include <css.h>
#include <saveutils.h>
#include <parserutils.h>
#include <controlcreationmanager.h>
#include <controlremovingmanager.h>
#include <controlpropertymanager.h>

#include <QMenu>
#include <QAction>
#include <QScrollBar>
#include <QMimeData>
#include <QGuiApplication>
#include <QClipboard>

extern const char* TOOL_KEY;

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
    verticalScrollBar()->setStyleSheet(CSS_SCROLLBAR);
    horizontalScrollBar()->setStyleSheet(CSS_SCROLLBAR_H);

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

void DesignerView::sweep()
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
    QList<QUrl> urls;
    QByteArray controls;
    QDataStream dstream(&controls, QIODevice::WriteOnly);
    auto mimeData = new QMimeData;
    auto clipboard = QGuiApplication::clipboard();
    auto selectedControls = scene()->selectedControls();
    selectedControls.removeOne(scene()->currentForm());
    mimeData->setData("objectwheel/uid", scene()->currentForm()->uid().toUtf8());
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

void DesignerView::onCopyAction()
{
    QList<QUrl> urls;
    auto mimeData = new QMimeData;
    auto clipboard = QGuiApplication::clipboard();
    auto selectedControls = scene()->selectedControls();
    selectedControls.removeOne(scene()->currentForm());
    mimeData->setData("objectwheel/uid", scene()->currentForm()->uid().toUtf8());

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

void DesignerView::onPasteAction()
{
    auto clipboard = QGuiApplication::clipboard();
    auto mimeData = clipboard->mimeData();
    auto currentForm = scene()->currentForm();
    QString sourceSuid = mimeData->data("objectwheel/uid");
    if (!mimeData->hasUrls() || !mimeData->hasText() ||
            mimeData->text() != TOOL_KEY || sourceSuid.isEmpty())
        return;

    QList<Control*> controls;
    for (auto url : mimeData->urls()) {
        auto control = ControlCreationManager::createControl(
                    url.toLocalFile(),
                    QPointF(ParserUtils::x(SaveUtils::toUrl(url.toLocalFile())) + 5, ParserUtils::y(SaveUtils::toUrl(url.toLocalFile())) + 5),
                    sourceSuid,
                    currentForm,
                    currentForm->dir(),
                    currentForm->uid()
                    );

        controls << control;

        if (url == mimeData->urls().last()) {
            scene()->clearSelection();
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

                ControlRemovingManager::removeControls(cutControls);
            }
        }

        // BUG: Do we really need this?
        //        ControlPreviewingManager::scheduleRefresh(control->uid());
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
