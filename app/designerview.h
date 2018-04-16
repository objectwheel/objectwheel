#ifndef FORMVIEW_H
#define FORMVIEW_H

#include <QGraphicsView>

class DesignerScene;
class QMenu;
class QAction;

class DesignerView : public QGraphicsView
{
    Q_OBJECT
public:
    explicit DesignerView(DesignerScene* scene, QWidget* parent = 0);
    DesignerScene* scene() const;

public slots:
    void reset();

protected:
    void resizeEvent(QResizeEvent* event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;

private slots:
    void onUndoAction();
    void onRedoAction();
    void onCutAction();
    void onCopyAction();
    void onPasteAction();
    void onDeleteAction();
    void onSelectAllAction();
    void onMoveUpAction();
    void onMoveDownAction();
    void onMoveRightAction();
    void onMoveLeftAction();
    void onSendBackAction();
    void onBringFrontAction();

private:
    QMenu* m_menu;
    QAction* m_sendBackAct;
    QAction* m_bringFrontAct;
    QAction* m_undoAct;
    QAction* m_redoAct;
    QAction* m_cutAct;
    QAction* m_copyAct;
    QAction* m_pasteAct;
    QAction* m_selectAllAct;
    QAction* m_deleteAct;
    QAction* m_moveUpAct;
    QAction* m_moveDownAct;
    QAction* m_moveRightAct;
    QAction* m_moveLeftAct;
};

#endif // FORMVIEW_H
