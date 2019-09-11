#ifndef DESIGNERPANE_H
#define DESIGNERPANE_H

#include <QWidget>

class QMenu;
class QToolBar;
class DesignerView;
class AnchorEditor;
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

    QToolButton* refreshButton() const;
    QToolButton* clearButton() const;
    QToolButton* anchorsButton() const;
    QToolButton* snappingButton() const;
    QToolButton* gridViewButton() const;
    QToolButton* guidelinesButton() const;
    QToolButton* sceneSettingsButton() const;
    QToolButton* themeSettingsButton() const;
    QComboBox* zoomLevelComboBox() const;
    QComboBox* themeComboBox() const;

    QAction* invertSelectionAction() const;
    QAction* selectAllAction() const;
    QAction* refreshFormContentAction() const;
    QAction* sendBackAction() const;
    QAction* bringFrontAction() const;
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

    QToolButton* m_refreshButton;
    QToolButton* m_clearButton;
    QToolButton* m_anchorsButton;
    QToolButton* m_snappingButton;
    QToolButton* m_gridViewButton;
    QToolButton* m_guidelinesButton;
    QToolButton* m_sceneSettingsButton;
    QToolButton* m_themeSettingsButton;
    QComboBox* m_zoomLevelComboBox;
    QComboBox* m_themeComboBox;

    QAction* m_invertSelectionAction;
    QAction* m_selectAllAction;
    QAction* m_refreshFormContentAction;
    QAction* m_sendBackAction;
    QAction* m_bringFrontAction;
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
