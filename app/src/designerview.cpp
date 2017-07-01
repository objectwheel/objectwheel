#include <designerview.h>
#include <designerscene.h>
#include <control.h>

#include <QTimer>
#include <QContextMenuEvent>
#include <QMenu>
#include <QAction>
#include <QApplication>
class DesignerViewPrivate : public QObject
{
        Q_OBJECT

    public:
        DesignerViewPrivate(DesignerView* parent);

    public:
        DesignerView* parent;
        QAction sendBackAct;
        QAction bringFrontAct;
        QAction undoAct;
        QAction redoAct;
        QAction cutAct;
        QAction copyAct;
        QAction pasteAct;
        QAction selectAllAct;
        QAction deleteAct;
        QMenu menu;
};

DesignerViewPrivate::DesignerViewPrivate(DesignerView* parent)
    : parent(parent)
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

    selectAllAct.setText("Select All");
    selectAllAct.setShortcut(QKeySequence::SelectAll);

    deleteAct.setText("Delete");
    deleteAct.setShortcut(QKeySequence::Delete);

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
}

DesignerView::DesignerView(QGraphicsScene* scene, QWidget* parent)
    : QGraphicsView(scene, parent)
    , _d(new DesignerViewPrivate(this))
{
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

void DesignerView::resizeEvent(QResizeEvent* event)
{
    QGraphicsView::resizeEvent(event);

    auto _scene = static_cast<DesignerScene*>(scene());
    auto currentPage = _scene->currentPage();
    if (currentPage)
        currentPage->centralize();

    setSceneRect(QRectF(- width() / 2.0, - height() / 2.0, width(), height()));
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
