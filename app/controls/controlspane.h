#ifndef CONTROLSPANE_H
#define CONTROLSPANE_H

#include <QWidget>

class ControlsTree;
class LineEdit;

class ControlsPane final : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(ControlsPane)

public:
    explicit ControlsPane(QWidget* parent = nullptr);

    ControlsTree* controlsTree() const;
    LineEdit* searchEdit() const;
    QSize sizeHint() const override;

private:
    ControlsTree* m_controlsTree;
    LineEdit* m_searchEdit;
};

#endif // CONTROLSPANE_H
