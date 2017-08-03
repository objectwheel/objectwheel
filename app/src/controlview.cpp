#include <controlview.h>
#include <controlscene.h>
#include <control.h>
#include <fit.h>
#include <savemanager.h>

#include <QTimer>
#include <QContextMenuEvent>
#include <QMenu>
#include <QAction>
#include <QApplication>
#include <QDebug>
#include <QClipboard>
#include <QMimeData>
#include <QJsonObject>
#include <QJsonDocument>

#define TOOLBOX_ITEM_KEY "QURBUEFaQVJMSVlJWiBIQUZJWg"

using namespace Fit;

class ControlViewPrivate : public QObject
{
        Q_OBJECT

    public:
        ControlViewPrivate(ControlView* parent);

    private slots:
        void handleUndoAction();
        void handleRedoAction();
        void handleCutAction();
        void handleCopyAction();
        void handlePasteAction();
        void handleDeleteAction();
        void handleSelectAllAction();
        void handleMoveUpAction();
        void handleMoveDownAction();
        void handleMoveRightAction();
        void handleMoveLeftAction();
        void handleSendBackActAction();
        void handleBringFrontActAction();

    public:
        ControlView* parent;
        QMenu menu;
        QAction sendBackAct;
        QAction bringFrontAct;
        QAction undoAct;
        QAction redoAct;
        QAction cutAct;
        QAction copyAct;
        QAction pasteAct;
        QAction selectAllAct;
        QAction deleteAct;
        QAction moveUpAct;
        QAction moveDownAct;
        QAction moveRightAct;
        QAction moveLeftAct;
};

ControlViewPrivate::ControlViewPrivate(ControlView* parent)
    : QObject(parent)
    , parent(parent)
{
    sendBackAct.setText("Send to Back");
    bringFrontAct.setText("Bring to Front");
    undoAct.setText("Undo");
    undoAct.setShortcut(QKeySequence::Undo);
    redoAct.setText("Redo");
    redoAct.setShortcut(QKeySequence::Redo);
    cutAct.setText("Cut");
    cutAct.setShortcut(QKeySequence::Cut);
    copyAct.setText("Copy");
    copyAct.setShortcut(QKeySequence::Copy);
    pasteAct.setText("Paste");
    pasteAct.setShortcut(QKeySequence::Paste);
    deleteAct.setText("Delete");
    deleteAct.setShortcut(QKeySequence::Delete);
    selectAllAct.setText("Select All");
    selectAllAct.setShortcut(QKeySequence::SelectAll);
    moveUpAct.setShortcut(Qt::Key_Up);
    moveDownAct.setShortcut(Qt::Key_Down);
    moveRightAct.setShortcut(Qt::Key_Right);
    moveLeftAct.setShortcut(Qt::Key_Left);

    menu.addAction(&sendBackAct);
    menu.addAction(&bringFrontAct);
    menu.addSeparator();
    menu.addAction(&undoAct);
    menu.addAction(&redoAct);
    menu.addSeparator();
    menu.addAction(&cutAct);
    menu.addAction(&copyAct);
    menu.addAction(&pasteAct);
    menu.addAction(&deleteAct);
    menu.addSeparator();
    menu.addAction(&selectAllAct);

    parent->addAction(&undoAct);
    parent->addAction(&redoAct);
    parent->addAction(&cutAct);
    parent->addAction(&copyAct);
    parent->addAction(&pasteAct);
    parent->addAction(&deleteAct);
    parent->addAction(&selectAllAct);
    parent->addAction(&moveUpAct);
    parent->addAction(&moveDownAct);
    parent->addAction(&moveRightAct);
    parent->addAction(&moveLeftAct);

    connect(&undoAct, SIGNAL(triggered()), SLOT(handleUndoAction()));
    connect(&redoAct, SIGNAL(triggered()), SLOT(handleRedoAction()));
    connect(&cutAct, SIGNAL(triggered()), SLOT(handleCutAction()));
    connect(&copyAct, SIGNAL(triggered()), SLOT(handleCopyAction()));
    connect(&pasteAct, SIGNAL(triggered()), SLOT(handlePasteAction()));
    connect(&deleteAct, SIGNAL(triggered()), SLOT(handleDeleteAction()));
    connect(&selectAllAct, SIGNAL(triggered()), SLOT(handleSelectAllAction()));
    connect(&moveUpAct, SIGNAL(triggered()), SLOT(handleMoveUpAction()));
    connect(&moveDownAct, SIGNAL(triggered()), SLOT(handleMoveDownAction()));
    connect(&moveRightAct, SIGNAL(triggered()), SLOT(handleMoveRightAction()));
    connect(&moveLeftAct, SIGNAL(triggered()), SLOT(handleMoveLeftAction()));
    connect(&sendBackAct, SIGNAL(triggered()), SLOT(handleSendBackActAction()));
    connect(&bringFrontAct, SIGNAL(triggered()), SLOT(handleBringFrontActAction()));
}

void ControlViewPrivate::handleUndoAction()
{
    //TODO
}

void ControlViewPrivate::handleRedoAction()
{
    //TODO
}

void ControlViewPrivate::handleCutAction()
{
    // TODO
//    QJsonObject pr;
//    QList<QUrl> urls;
//    auto mimeData = new QMimeData;
//    auto clipboard = QApplication::clipboard();
//    auto scene = static_cast<ControlScene*>(parent->scene());

//    for (auto control : scene->selectedControls()) {
//        for (auto childControl : control->childControls()) {
//            pr[childControl->id()] = control->id();
//            urls << childControl->url();
//        }
//        pr[control->id()] = control->parentControl()->id();
//        urls << control->url();
//    }

//    mimeData->setUrls(m_Urls[items[0]]);
//    mimeData->setText(TOOLBOX_ITEM_KEY);
//    mimeData->setHtml(QJsonDocument(pr));
}

void ControlViewPrivate::handleCopyAction()
{
    //TODO
}

void ControlViewPrivate::handlePasteAction()
{
    //TODO
}

void ControlViewPrivate::handleDeleteAction()
{
    auto scene = static_cast<ControlScene*>(parent->scene());
    for (auto control : scene->selectedControls()) {
        ControlScene::removeControl(control);
    }
}

void ControlViewPrivate::handleSelectAllAction()
{
    auto currentControl = ((ControlScene*)parent->scene())->currentControl();
    for (auto control : currentControl->childControls())
        control->setSelected(true);
}

void ControlViewPrivate::handleMoveUpAction()
{
    auto scene = static_cast<ControlScene*>(parent->scene());
    for (auto control : scene->selectedControls())
        control->moveBy(0, - fit(1));
}

void ControlViewPrivate::handleMoveDownAction()
{
    auto scene = static_cast<ControlScene*>(parent->scene());
    for (auto control : scene->selectedControls())
        control->moveBy(0, fit(1));
}

void ControlViewPrivate::handleMoveRightAction()
{
    auto scene = static_cast<ControlScene*>(parent->scene());
    for (auto control : scene->selectedControls())
        control->moveBy(fit(1), 0);
}

void ControlViewPrivate::handleMoveLeftAction()
{
    auto scene = static_cast<ControlScene*>(parent->scene());
    for (auto control : scene->selectedControls())
        control->moveBy(- fit(1), 0);
}

void ControlViewPrivate::handleSendBackActAction()
{
    auto scene = static_cast<ControlScene*>(parent->scene());
    for (auto control : scene->selectedControls())
        control->setZValue(scene->currentControl()->lowerZValue() == MAX_Z_VALUE
                           ? 0 : scene->currentControl()->lowerZValue() - 1);
}

void ControlViewPrivate::handleBringFrontActAction()
{
    auto scene = static_cast<ControlScene*>(parent->scene());
    for (auto control : scene->selectedControls())
        control->setZValue(scene->currentControl()->higherZValue() == -MAX_Z_VALUE
                           ? 0 : scene->currentControl()->higherZValue() + 1);
}

ControlView::ControlView(QGraphicsScene* scene, QWidget* parent)
    : QGraphicsView(scene, parent)
    , _d(new ControlViewPrivate(this))
{
    setViewportUpdateMode(FullViewportUpdate);
}

void ControlView::resizeEvent(QResizeEvent* event)
{
    QGraphicsView::resizeEvent(event);

    auto _scene = static_cast<ControlScene*>(scene());
    auto currentControl = _scene->currentControl();
    if (currentControl)
        currentControl->centralize();
}

void ControlView::contextMenuEvent(QContextMenuEvent* event)
{
    QGraphicsView::contextMenuEvent(event);

    auto scene = static_cast<ControlScene*>(this->scene());
    auto selectedControls = scene->selectedControls();

    if (selectedControls.size() <= 0) {
        _d->sendBackAct.setDisabled(true);
        _d->bringFrontAct.setDisabled(true);
        _d->cutAct.setDisabled(true);
        _d->copyAct.setDisabled(true);
        _d->deleteAct.setDisabled(true);
    } else {
        _d->sendBackAct.setDisabled(false);
        _d->bringFrontAct.setDisabled(false);
        _d->cutAct.setDisabled(false);
        _d->copyAct.setDisabled(false);
        _d->deleteAct.setDisabled(false);
    }
    _d->menu.exec(event->globalPos());
}

#include "controlview.moc"