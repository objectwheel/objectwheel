#ifndef DESIGNERCONTROLLER_H
#define DESIGNERCONTROLLER_H

#include <anchorline.h>
#include <copypaste.h>

class DesignerPane;
class DesignerController final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(DesignerController)

public:
    explicit DesignerController(DesignerPane* designerPane, QObject* parent = nullptr);

public slots:
    void charge();
    void discharge();

private slots:
    void onSceneSettingsChange();
    void onCustomContextMenuRequest(const QPoint& pos);
    void onControlDoubleClick(Control* control, Qt::MouseButtons buttons);

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

    void onInvertSelectionActionTrigger();
    void onSelectAllActionTrigger();
    void onSendBackActionTrigger();
    void onBringFrontActionTrigger();
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
    void codeEditorTriggered(Control* control, const QString& relativeFileName);
    void projectThemeActivated(const QString& theme);

private:
    DesignerPane* m_designerPane;
    CopyPaste m_copyPaste;
};

#endif // DESIGNERCONTROLLER_H
