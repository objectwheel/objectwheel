#ifndef PUSHBUTTON_H
#define PUSHBUTTON_H

#include <QPushButton>

class PushButton : public QPushButton
{
    Q_OBJECT
public:
    explicit PushButton(QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent*) override;
    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;
};

#endif // PUSHBUTTON_H