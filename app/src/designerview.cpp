#include <designerview.h>
#include <designerscene.h>
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

class DesignerViewPrivate : public QObject
{
        Q_OBJECT

    public:
        DesignerViewPrivate(DesignerView* parent);

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
        DesignerView* parent;
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

DesignerViewPrivate::DesignerViewPrivate(DesignerView* parent)
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

void DesignerViewPrivate::handleUndoAction()
{
    //TODO
}

void DesignerViewPrivate::handleRedoAction()
{
    //TODO
}

void DesignerViewPrivate::handleCutAction()
{
    // TODO
//    QJsonObject pr;
//    QList<QUrl> urls;
//    auto mimeData = new QMimeData;
//    auto clipboard = QApplication::clipboard();
//    auto scene = static_cast<DesignerScene*>(parent->scene());

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

void DesignerViewPrivate::handleCopyAction()
{
    //TODO
}

void DesignerViewPrivate::handlePasteAction()
{
    //TODO
}

void DesignerViewPrivate::handleDeleteAction()
{
    auto scene = static_cast<DesignerScene*>(parent->scene());
    for (auto control : scene->selectedControls()) {
        for (auto childControl : control->childControls())
            scene->removeControl(childControl);
        scene->removeControl(control);
    }
}

void DesignerViewPrivate::handleSelectAllAction()
{
    auto currentPage = ((DesignerScene*)parent->scene())->currentPage();
    for (auto control : currentPage->childControls())
        control->setSelected(true);
}

void DesignerViewPrivate::handleMoveUpAction()
{
    auto scene = static_cast<DesignerScene*>(parent->scene());
    for (auto control : scene->selectedControls())
        control->moveBy(0, - fit(1));
}

void DesignerViewPrivate::handleMoveDownAction()
{
    auto scene = static_cast<DesignerScene*>(parent->scene());
    for (auto control : scene->selectedControls())
        control->moveBy(0, fit(1));
}

void DesignerViewPrivate::handleMoveRightAction()
{
    auto scene = static_cast<DesignerScene*>(parent->scene());
    for (auto control : scene->selectedControls())
        control->moveBy(fit(1), 0);
}

void DesignerViewPrivate::handleMoveLeftAction()
{
    auto scene = static_cast<DesignerScene*>(parent->scene());
    for (auto control : scene->selectedControls())
        control->moveBy(- fit(1), 0);
}

void DesignerViewPrivate::handleSendBackActAction()
{
    auto scene = static_cast<DesignerScene*>(parent->scene());
    for (auto control : scene->selectedControls())
        control->setZValue(scene->currentPage()->lowerZValue() == MAX_Z_VALUE
                           ? 0 : scene->currentPage()->lowerZValue() - 1);
}

void DesignerViewPrivate::handleBringFrontActAction()
{
    auto scene = static_cast<DesignerScene*>(parent->scene());
    for (auto control : scene->selectedControls())
        control->setZValue(scene->currentPage()->higherZValue() == -MAX_Z_VALUE
                           ? 0 : scene->currentPage()->higherZValue() + 1);
}

DesignerView::DesignerView(QGraphicsScene* scene, QWidget* parent)
    : QGraphicsView(scene, parent)
    , _d(new DesignerViewPrivate(this))
{
}

void DesignerView::resizeEvent(QResizeEvent* event)
{
    QGraphicsView::resizeEvent(event);

    auto _scene = static_cast<DesignerScene*>(scene());
    auto currentPage = _scene->currentPage();
    if (currentPage)
        currentPage->centralize();
}

void DesignerView::contextMenuEvent(QContextMenuEvent* event)
{
    QGraphicsView::contextMenuEvent(event);

    auto scene = static_cast<DesignerScene*>(this->scene());
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

#include "designerview.moc"
