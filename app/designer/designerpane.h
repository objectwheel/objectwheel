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

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

private:
    void updateIcons();
    void changeEvent(QEvent* event) override;
    void contextMenuEvent(QContextMenuEvent* event) override;

private:
    QMenu* m_menu;
    QToolBar* m_toolBar;
    DesignerView* m_designerView;
    AnchorEditor* m_anchorEditor;

    QAction* m_toggleSelectionAction;
    QAction* m_selectAllAction;
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

    QToolButton* m_clearButton;
    QToolButton* m_refreshButton;
    QToolButton* m_anchorsButton;
    QToolButton* m_snappingButton;
    QToolButton* m_gridViewButton;
    QToolButton* m_guidelinesButton;
    QToolButton* m_controlOutlineButton;
    QToolButton* m_backgroundTextureButton;
    QToolButton* m_settingsButton;
    QToolButton* m_themeSettingsButton;
    QComboBox* m_zoomLevelComboBox;
    QComboBox* m_themeComboBox;
};

#endif // DESIGNERPANE_H
