#include <formview.h>
#include <formscene.h>
#include <control.h>
#include <fit.h>
#include <savemanager.h>
#include <designmanager.h>
#include <filemanager.h>

#include <QTimer>
#include <QContextMenuEvent>
#include <QMenu>
#include <QAction>
#include <QApplication>
#include <QDebug>
#include <QClipboard>
#include <QMimeData>
#include <QJsonDocument>
#include <QJsonValue>
#include <QJsonObject>

#define TOOLBOX_ITEM_KEY "QURBUEFaQVJMSVlJWiBIQUZJWg"

using namespace Fit;

class FormViewPrivate : public QObject
{
        Q_OBJECT

    public:
        FormViewPrivate(FormView* parent);

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
        FormView* parent;
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

FormViewPrivate::FormViewPrivate(FormView* parent)
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

void FormViewPrivate::handleUndoAction()
{
    //TODO
}

void FormViewPrivate::handleRedoAction()
{
    //TODO
}

void FormViewPrivate::handleCutAction()
{
    QList<QUrl> urls;
    QByteArray controls;
    QDataStream dstream(&controls, QIODevice::WriteOnly);
    auto mimeData = new QMimeData;
    auto clipboard = QApplication::clipboard();
    auto scene = DesignManager::formScene();
    auto selectedControls = scene->selectedControls();
    selectedControls.removeOne(scene->mainControl());
    mimeData->setData("objectwheel/uid", scene->mainControl()->uid().toUtf8());
    mimeData->setData("objectwheel/cut", "1");
    mimeData->setData("objectwheel/fscene", "");

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
    mimeData->setText(TOOLBOX_ITEM_KEY);
    clipboard->setMimeData(mimeData);
}

void FormViewPrivate::handleCopyAction()
{
    QList<QUrl> urls;
    auto mimeData = new QMimeData;
    auto clipboard = QApplication::clipboard();
    auto scene = DesignManager::formScene();
    auto selectedControls = scene->selectedControls();
    selectedControls.removeOne(scene->mainControl());
    mimeData->setData("objectwheel/uid", scene->mainControl()->uid().toUtf8());

    for (auto control : selectedControls)
        for (auto ctrl : selectedControls)
            if (control->childControls().contains(ctrl))
                selectedControls.removeAll(ctrl);

    for (auto control : selectedControls)
        urls << QUrl::fromLocalFile(control->dir());

    mimeData->setUrls(urls);
    mimeData->setText(TOOLBOX_ITEM_KEY);
    clipboard->setMimeData(mimeData);
}

void FormViewPrivate::handlePasteAction()
{
    auto clipboard = QApplication::clipboard();
    auto mimeData = clipboard->mimeData();
    auto mainControl = DesignManager::formScene()->mainForm();
    QString uid = mimeData->data("objectwheel/uid");
    if (!mimeData->hasUrls() || !mimeData->hasText() ||
        mimeData->text() != TOOLBOX_ITEM_KEY || uid.isEmpty())
        return;

    QList<Control*> controls;
    for (auto url : mimeData->urls()) {
        auto control = SaveManager::exposeControl(url.toLocalFile(), FormGui, uid);
        SaveManager::addControl(control, mainControl, mainControl->uid(), mainControl->dir());
        control->setParentItem(mainControl);
        control->refresh();
        controls << control;

        control->setPos(control->pos() + QPoint(fit(5), fit(5)));
        if (url == mimeData->urls().last()) {
            DesignManager::formScene()->clearSelection();
            for (auto control : controls)
                control->setSelected(true);

            if (!mimeData->data("objectwheel/cut").isEmpty()) {
                ControlScene* scene;
                if (mimeData->data("objectwheel/fscene").isEmpty())
                    scene = DesignManager::formScene();
                else
                    scene = DesignManager::controlScene();

                QDataStream dstream(mimeData->data("objectwheel/dstream"));
                int size = QString(mimeData->data("objectwheel/dstreamsize")).toInt();
                QList<Control*> cutControls;
                for (int i = 0; i < size; i++) {
                    quint64 buff;
                    dstream >> buff;
                    cutControls << (Control*)buff;
                }

                for (auto control : cutControls) {
                    scene->removeControl(control);
                    SaveManager::removeControl(control);
                }
            }
        }

        for (auto childControl : control->childControls())
            childControl->refresh();
    }
}

void FormViewPrivate::handleDeleteAction()
{
    auto scene = static_cast<FormScene*>(parent->scene());
    auto selectedControls = scene->selectedControls();
    selectedControls.removeOne(scene->mainControl());
    for (auto control : selectedControls) {
       scene->removeControl(control);
       SaveManager::removeControl(control);
    }
}

void FormViewPrivate::handleSelectAllAction()
{
    auto mainForm = ((FormScene*)parent->scene())->mainForm();
    for (auto control : mainForm->childControls())
        control->setSelected(true);
}

void FormViewPrivate::handleMoveUpAction()
{
    auto scene = static_cast<FormScene*>(parent->scene());
    auto selectedControls = scene->selectedControls();
    selectedControls.removeOne(scene->mainControl());
    for (auto control : selectedControls)
        control->moveBy(0, - fit(1));
}

void FormViewPrivate::handleMoveDownAction()
{
    auto scene = static_cast<FormScene*>(parent->scene());
    auto selectedControls = scene->selectedControls();
    selectedControls.removeOne(scene->mainControl());
    for (auto control : selectedControls)
        control->moveBy(0, fit(1));
}

void FormViewPrivate::handleMoveRightAction()
{
    auto scene = static_cast<FormScene*>(parent->scene());
    auto selectedControls = scene->selectedControls();
    selectedControls.removeOne(scene->mainControl());
    for (auto control : selectedControls)
        control->moveBy(fit(1), 0);
}

void FormViewPrivate::handleMoveLeftAction()
{
    auto scene = static_cast<FormScene*>(parent->scene());
    auto selectedControls = scene->selectedControls();
    selectedControls.removeOne(scene->mainControl());
    for (auto control : selectedControls)
        control->moveBy(- fit(1), 0);
}

void FormViewPrivate::handleSendBackActAction()
{
    auto scene = static_cast<FormScene*>(parent->scene());
    auto selectedControls = scene->selectedControls();
    selectedControls.removeOne(scene->mainControl());
    for (auto control : selectedControls)
        control->setZValue(scene->mainForm()->lowerZValue() - 1);
}

void FormViewPrivate::handleBringFrontActAction()
{
    auto scene = static_cast<FormScene*>(parent->scene());
    auto selectedControls = scene->selectedControls();
    selectedControls.removeOne(scene->mainControl());
    for (auto control : selectedControls)
        control->setZValue(scene->mainForm()->higherZValue() + 1);
}

FormView::FormView(QGraphicsScene* scene, QWidget* parent)
    : QGraphicsView(scene, parent)
    , _d(new FormViewPrivate(this))
{
}

void FormView::resizeEvent(QResizeEvent* event)
{
    QGraphicsView::resizeEvent(event);

    auto _scene = static_cast<FormScene*>(scene());
    auto mainForm = _scene->mainForm();
    if (mainForm)
        mainForm->centralize();
}

void FormView::contextMenuEvent(QContextMenuEvent* event)
{
    QGraphicsView::contextMenuEvent(event);

    auto scene = static_cast<FormScene*>(this->scene());
    auto selectedControls = scene->selectedControls();
    selectedControls.removeOne(scene->mainControl());

    if (selectedControls.isEmpty()) {
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
        for (auto sc : selectedControls) {
            if (sc->gui() == false) {
                _d->sendBackAct.setDisabled(true);
                _d->bringFrontAct.setDisabled(true);
            }
        }
    }
    _d->menu.exec(event->globalPos());
}

#include "formview.moc"
