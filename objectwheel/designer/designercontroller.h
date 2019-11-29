#ifndef DESIGNERCONTROLLER_H
#define DESIGNERCONTROLLER_H

#include <anchorline.h>
#include <copypaste.h>

class DesignerPane;
class DesignerController final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(DesignerController)

    friend class MainWindow; // For connecting other controller signals to our slots

public:
    explicit DesignerController(DesignerPane* designerPane, QObject* parent = nullptr);

public slots:
    void charge();
    void discharge();

private slots:
    void onSceneSettingsChange();
    void onContextMenuRequest(const QPoint& scenePos);
    void onControlDoubleClick(Control* control, Qt::MouseButtons buttons);
    void onRenderEngineConnectionStatusChange(bool connected);

    void onAnchorClear();
    void onAnchorSourceControlActivation();
    void onAnchor(AnchorLine::Type sourceLineType, const AnchorLine& targetLine);
    void onAnchorFill(Control* control);
    void onAnchorCenter(Control* control, bool overlay);
    void onAnchorMarginOffsetEdit(AnchorLine::Type sourceLineType, qreal marginOffset);
    void onAnchorMarginsEdit(qreal margins);
    void onAnchorAlignmentActivation(bool aligned);
    void onAnchorEditorActivation(Control* sourceControl, Control* targetControl);

    void onRefreshButtonClick();
    void onShrinkSceneButtonClick();
    void onClearButtonClick();
    void onAnchorsButtonClick();
    void onSnappingButtonClick();
    void onGridViewButtonClick();
    void onGuidelinesButtonClick();
    void onSceneSettingsButtonClick();
    void onThemeSettingsButtonClick();
    void onZoomLevelComboBoxActivation(const QString& currentText);
    void onRendererStatusButtonClick();

    void onInvertSelectionActionTrigger();
    void onSelectAllActionTrigger();
    void onSendBackActionTrigger();
    void onBringFrontActionTrigger();
    void onEditAnchorsActionTrigger();
    void onViewSourceCodeActionTrigger();
    void onGoToSlotActionTrigger();
    void onCutActionTrigger();
    void onCopyActionTrigger();
    void onPasteActionTrigger();
    void onDeleteActionTrigger();
    void onMoveLeftActionTrigger();
    void onMoveRightActionTrigger();
    void onMoveUpActionTrigger();
    void onMoveDownActionTrigger();

private:
    QList<Control*> selectedAncestorControls() const;
    QList<Control*> movableSelectedAncestorControls(const QList<Control*>& selectedControls) const;

signals:
    void viewSourceCodeTriggered(Control* control);
    void goToSlotTriggered(Control* control, const QString& signal);
    void projectThemeActivated(const QString& theme);
    void projectThemeActivated1(const QString& theme);

private:
    DesignerPane* m_designerPane;
    CopyPaste m_copyPaste;
};

#endif // DESIGNERCONTROLLER_H
