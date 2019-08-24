#ifndef ANCHORROW_H
#define ANCHORROW_H

#include <QWidget>
#include <anchorline.h>

class QBoxLayout;
class QToolButton;
class QComboBox;
class QDoubleSpinBox;

class AnchorRow final : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(AnchorRow)

public:
    explicit AnchorRow(QWidget* parent = nullptr);

    AnchorLine::Type sourceLineType() const;
    void setSourceLineType(AnchorLine::Type sourceLineType);

    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;

private slots:
    void onSourceLineTypeChange();
    void onSourceButtonCheckedChange();

signals:
    void sourceLineTypeChanged();

private:
    AnchorLine::Type m_sourceLineType;
    QBoxLayout* m_layout;
    QToolButton* m_sourceLineButton;
    QToolButton* m_targetLineButton1;
    QToolButton* m_targetLineButton2;
    QToolButton* m_targetLineButton3;
    QDoubleSpinBox* m_marginOffsetSpinBox;
    QComboBox* m_targetControlComboBox;
};

#endif // ANCHORROW_H
