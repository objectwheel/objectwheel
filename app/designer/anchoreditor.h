#ifndef ANCHOREDITOR_H
#define ANCHOREDITOR_H

#include <QWidget>
#include <anchorline.h>

class QBoxLayout;
class AnchorRow;
class QDialogButtonBox;
class Control;
class QComboBox;
class QDoubleSpinBox;
class QCheckBox;
class DesignerScene;

class AnchorEditor final : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(AnchorEditor)

public:
    explicit AnchorEditor(DesignerScene* scene, QWidget* parent = nullptr);

    void activate(Control* source, Control* target);

public slots:
    void onMarginOffsetEditingFinish(AnchorRow* row);
    void onTargetControlActivate(AnchorRow* row);
    void onTargetLineTypeActivate(AnchorRow* row);

signals:
    void marginOffsetEdited(const AnchorLine& sourceLine, qreal marginOffset);
    void anchored(const AnchorLine& sourceLine, const AnchorLine& targetLine);
    void cleared();

private:
    const DesignerScene* m_scene;
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
