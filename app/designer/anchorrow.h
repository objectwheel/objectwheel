#ifndef ANCHORROW_H
#define ANCHORROW_H

#include <QWidget>
#include <anchorline.h>

class QBoxLayout;
class QToolButton;
class QComboBox;
class QDoubleSpinBox;
class ButtonGroup;
class QAbstractButton;

class AnchorRow final : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(AnchorRow)

public:
    explicit AnchorRow(QWidget* parent = nullptr);

    bool isActive() const;
    void setActive(bool active);

    AnchorLine::Type sourceLineType() const;
    void setSourceLineType(AnchorLine::Type sourceLineType);

    AnchorLine::Type targetLineType() const;
    void setTargetLineType(AnchorLine::Type targetLineType);

    qreal marginOffset() const;
    void setMarginOffset(qreal marginOffset);

    QList<Control*> targetControlList() const;
    void setTargetControlList(const QList<Control*>& targetControlList);

    Control* currentTargetControl() const;
    void setCurrentTargetControl(const Control* control);

    void clear();

    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;

private slots:
    void onSourceLineTypeChange();
    void onTargetLineTypeChange();
    void onSourceButtonClick();
    void onTargetButtonClick(QAbstractButton* button, bool checked);

signals:
    void sourceLineTypeChanged();
    void targetLineTypeChanged();
    void targetLineTypeActivated();
    void marginOffsetEditingFinished();
    void targetControlActivated();

private:
    AnchorLine::Type m_sourceLineType;
    AnchorLine::Type m_targetLineType;
    QBoxLayout* m_layout;
    QToolButton* m_sourceLineButton;
    ButtonGroup* m_targetButtonGroup;
    QToolButton* m_targetLineButton1;
    QToolButton* m_targetLineButton2;
    QToolButton* m_targetLineButton3;
    QDoubleSpinBox* m_marginOffsetSpinBox;
    QComboBox* m_targetControlComboBox;
};

#endif // ANCHORROW_H
