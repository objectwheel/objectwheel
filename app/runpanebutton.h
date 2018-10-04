#ifndef RUNPANEBUTTON_H
#define RUNPANEBUTTON_H

#include <QAbstractButton>

class RunPaneButton : public QAbstractButton
{
    Q_OBJECT
public:
    explicit RunPaneButton(QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent*) override;
    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;
};

#endif // RUNPANEBUTTON_H