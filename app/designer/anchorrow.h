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
class QLabel;

class AnchorRow final : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(AnchorRow)

public:
    explicit AnchorRow(AnchorLine::Type sourceLineType, QWidget* parent = nullptr);

    AnchorLine::Type targetLineType() const;
    void setTargetLineType(AnchorLine::Type targetLineType);

    qreal marginOffset() const;
    void setMarginOffset(qreal marginOffset);

    QList<Control*> targetControlList() const;
    void setTargetControlList(const QList<Control*>& targetControlList);

    Control* currentTargetControl() const;
    void setCurrentTargetControl(const Control* control);

    void clear();

public:
    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;

private slots:
    void onTargetLineTypeChange();
    void onTargetButtonClick(QAbstractButton* button, bool checked);

signals:
    void targetLineTypeChanged();
    void targetLineTypeActivated();
    void marginOffsetEditingFinished();
    void targetControlActivated();

private:
    const AnchorLine::Type m_sourceLineType;
    AnchorLine::Type m_targetLineType;
    QBoxLayout* m_layout;
    QLabel* m_sourceIcon;
    QLabel* m_arrowIcon;
    ButtonGroup* m_targetButtonGroup;
    QToolButton* m_targetLineButton1;
    QToolButton* m_targetLineButton2;
    QToolButton* m_targetLineButton3;
    QDoubleSpinBox* m_marginOffsetSpinBox;
    QComboBox* m_targetControlComboBox;
};

#endif // ANCHORROW_H
