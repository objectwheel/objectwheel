#ifndef DESIGNERPANE_H
#define DESIGNERPANE_H

#include <QWidget>

class QMenu;
class QToolBar;
class DesignerView;
class AnchorEditor;
class SignalEditor;
class QToolButton;
class QComboBox;

class DesignerPane final : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(DesignerPane)

public:
    explicit DesignerPane(QWidget* parent = nullptr);

    QMenu* menu() const;
    QToolBar* toolBar() const;
    DesignerView* designerView() const;
    AnchorEditor* anchorEditor() const;
    SignalEditor* signalEditor() const;

    QToolButton* refreshButton() const;
    QToolButton* shrinkSceneButton() const;
    QToolButton* clearButton() const;
    QToolButton* anchorsButton() const;
    QToolButton* snappingButton() const;
    QToolButton* gridViewButton() const;
    QToolButton* guidelinesButton() const;
    QToolButton* sceneSettingsButton() const;
    QToolButton* themeSettingsButton() const;
    QComboBox* zoomLevelComboBox() const;
    QComboBox* themeComboBox() const;
    QComboBox* themeComboBox1() const;
    QToolButton* rendererStatusButton() const;

    QAction* invertSelectionAction() const;
    QAction* selectAllAction() const;
    QAction* refreshAction() const;
    QAction* sendBackAction() const;
    QAction* bringFrontAction() const;
    QAction* editAnchorsAction() const;
    QAction* viewSourceCodeAction() const;
    QAction* goToSlotAction() const;
    QAction* cutAction() const;
    QAction* copyAction() const;
    QAction* pasteAction() const;
    QAction* deleteAction() const;
    QAction* deleteAllAction() const;
    QAction* moveLeftAction() const;
    QAction* moveRightAction() const;
    QAction* moveUpAction() const;
    QAction* moveDownAction() const;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

private:
    QMenu* m_menu;
    QToolBar* m_toolBar;
    DesignerView* m_designerView;
    AnchorEditor* m_anchorEditor;
    SignalEditor* m_signalEditor;

    QToolButton* m_refreshButton;
    QToolButton* m_shrinkSceneButton;
    QToolButton* m_clearButton;
    QToolButton* m_anchorsButton;
    QToolButton* m_snappingButton;
    QToolButton* m_gridViewButton;
    QToolButton* m_guidelinesButton;
    QToolButton* m_sceneSettingsButton;
    QToolButton* m_themeSettingsButton;
    QComboBox* m_zoomLevelComboBox;
    QComboBox* m_themeComboBox;
    QComboBox* m_themeComboBox1;
    QToolButton* m_rendererStatusButton;

    QAction* m_invertSelectionAction;
    QAction* m_selectAllAction;
    QAction* m_refreshAction;
    QAction* m_sendBackAction;
    QAction* m_bringFrontAction;
    QAction* m_editAnchorsAction;
    QAction* m_viewSourceCodeAction;
    QAction* m_goToSlotAction;
    QAction* m_cutAction;
    QAction* m_copyAction;
    QAction* m_pasteAction;
    QAction* m_deleteAction;
    QAction* m_deleteAllAction;
    QAction* m_moveLeftAction;
    QAction* m_moveRightAction;
    QAction* m_moveUpAction;
    QAction* m_moveDownAction;
};

#endif // DESIGNERPANE_H
