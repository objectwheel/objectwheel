#ifndef ANCHOREDITOR_H
#define ANCHOREDITOR_H

#include <QDialog>
#include <anchorline.h>

class QBoxLayout;
class AnchorRow;
class QDialogButtonBox;
class Control;
class QComboBox;
class QDoubleSpinBox;
class QCheckBox;
class DesignerScene;

class AnchorEditor final : public QDialog
{
    Q_OBJECT
    Q_DISABLE_COPY(AnchorEditor)

public:
    explicit AnchorEditor(DesignerScene* scene, QWidget* parent = nullptr);

    Control* sourceControl() const;
    void setSourceControl(Control* sourceControl);

    Control* primaryTargetControl() const;
    void setPrimaryTargetControl(Control* primaryTargetControl);

    void refresh(bool delayed = true);

    void clear();

public slots:
    void onMarginOffsetEditingFinish(AnchorRow* row);
    void onTargetControlActivate(AnchorRow* row);
    void onTargetLineTypeActivate(AnchorRow* row);

private:
    void refreshNow();

signals:
    void sourceControlActivated();
    void alignmentActivated(bool align);
    void marginsEdited(qreal marginOffset);
    void marginOffsetEdited(AnchorLine::Type sourceLineType, qreal marginOffset);
    void anchored(AnchorLine::Type sourceLineType, const AnchorLine& targetLine);
    void filled(Control* control);
    void centered(Control* control, bool overlay);
    void cleared();

private:
    const DesignerScene* m_scene;
    bool m_refreshScheduled;
    QPointer<Control> m_sourceControl;
    QPointer<Control> m_primaryTargetControl;
    QBoxLayout* m_layout;
    QComboBox* m_sourceControlComboBox;
    QDoubleSpinBox* m_marginsSpinBox;
    AnchorRow* m_leftRow;
    AnchorRow* m_rightRow;
    AnchorRow* m_topRow;
    AnchorRow* m_bottomRow;
    AnchorRow* m_fillRow;
    AnchorRow* m_horizontalCenterRow;
    AnchorRow* m_verticalCenterRow;
    AnchorRow* m_centerInRow;
    QCheckBox* m_alignWhenCenteredCheckBox;
    QDialogButtonBox* m_dialogButtonBox;
};

#endif // ANCHOREDITOR_H
