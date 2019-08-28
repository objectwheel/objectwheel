#ifndef ANCHORROW_H
#define ANCHORROW_H

#include <QWidget>
#include <anchorline.h>

class QBoxLayout;
class QComboBox;
class QDoubleSpinBox;
class ButtonGroup;
class QAbstractButton;
class QPushButton;

class AnchorRow final : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(AnchorRow)

public:
    explicit AnchorRow(AnchorLine::Type sourceLineType, QWidget* parent = nullptr);

    AnchorLine::Type sourceLineType() const;

    AnchorLine::Type targetLineType() const;
    void setTargetLineType(AnchorLine::Type targetLineType);

    qreal marginOffset() const;
    void setMarginOffset(qreal marginOffset);

    QList<Control*> targetControlList() const;
    void setTargetControlList(const QList<Control*>& targetControlList);

    Control* targetControl() const;
    void setTargetControl(const Control* control);

    bool fillCenterModeEnabled() const;
    void setFillCenterModeEnabled(bool fillCenterModeEnabled, Control* targetControl = nullptr);

    bool overlayModeEnabled() const;
    void setOverlayModeEnabled(bool overlayModeEnabled);

    void setSourceButtonChecked(bool checked);
    void clear();

private slots:
    void onTargetLineTypeChange();
    void onSourceButtonCheckedChange(bool checked);
    void onTargetButtonClick(QAbstractButton* button, bool checked);

signals:
    void targetLineTypeChanged();
    void targetLineTypeActivated();
    void marginOffsetEditingFinished();
    void targetControlActivated();
    void sourceButtonClicked(bool checked);

private:
    const AnchorLine::Type m_sourceLineType;
    AnchorLine::Type m_targetLineType;
    AnchorLine::Type m_targetLineTypeBackup;
    Control* m_targetControlBackup;
    bool m_fillCenterModeEnabled;
    bool m_overlayModeEnabled;
    QBoxLayout* m_layout;
    QPushButton* m_sourceButton;
    QComboBox* m_targetControlComboBox;
    QDoubleSpinBox* m_marginOffsetSpinBox;
    ButtonGroup* m_targetButtonGroup;
    QPushButton* m_targetLineButton1;
    QPushButton* m_targetLineButton2;
    QPushButton* m_targetLineButton3;
};

#endif // ANCHORROW_H
