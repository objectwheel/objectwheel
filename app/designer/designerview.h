#ifndef DESIGNERVIEW_H
#define DESIGNERVIEW_H

#include <QGraphicsView>

class QmlCodeEditorWidget;
class DesignerScene;
class QToolBar;
class QToolButton;
class QComboBox;
class Control;
class SignalChooserDialog;
class QMenu;
class AnchorEditor;

class DesignerView final : public QGraphicsView
{
    Q_OBJECT
    Q_DISABLE_COPY(DesignerView)

public:
    explicit DesignerView(QmlCodeEditorWidget* qmlCodeEditorWidget, QWidget* parent = nullptr);

    DesignerScene* scene() const;

public slots:
    void discharge();
    void refresh() { onRefreshButtonClick(); }
    void onControlDoubleClick(Control*, Qt::MouseButtons);
    void onInspectorItemDoubleClick(Control*);
    void onAssetsFileOpen(const QString& relativePath, int line, int column);
    void onDesignsFileOpen(Control* control, const QString& relativePath, int line, int column);
    void onControlSelectionChange(const QList<Control*>& selectedControls);

private slots:
    void onFitButtonClick();
    void onUndoButtonClick();
    void onRedoButtonClick();
    void onClearButtonClick();
    void onRefreshButtonClick();
    void onOutlineButtonTrigger(QAction* action);
    void onSnappingButtonClick(bool value);
    void onZoomLevelChange(const QString& text);

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

signals:
    void hideDockWidgetTitleBars(bool);

private:
    AnchorEditor* m_anchorEditor;
    SignalChooserDialog* m_signalChooserDialog;
    QmlCodeEditorWidget* m_qmlCodeEditorWidget;
    QToolBar* m_toolBar;
    QToolButton* m_undoButton;
    QToolButton* m_redoButton;
    QToolButton* m_clearButton;
    QToolButton* m_refreshButton;
    QToolButton* m_snappingButton;
    QToolButton* m_fitButton;
    QToolButton* m_outlineButton;
    QToolButton* m_hideDockWidgetTitleBarsButton;
    QComboBox* m_zoomlLevelCombobox;

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

#endif // DESIGNERVIEW_H
