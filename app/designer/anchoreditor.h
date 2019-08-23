#ifndef ANCHOREDITOR_H
#define ANCHOREDITOR_H

#include <QWidget>

class Control;
class AnchorEditor final : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(AnchorEditor)

public:
    explicit AnchorEditor(QWidget* parent = nullptr);

    Control* sourceControl() const;
    void setSourceControl(Control* sourceControl);

    Control* targetControl() const;
    void setTargetControl(Control* targetControl);

private:
    Control* m_sourceControl;
    Control* m_targetControl;
};

#endif // ANCHOREDITOR_H
