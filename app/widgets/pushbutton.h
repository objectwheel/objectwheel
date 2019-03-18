#ifndef PUSHBUTTON_H
#define PUSHBUTTON_H

#include <QPushButton>

class PushButton final : public QPushButton
{
    Q_OBJECT
    Q_DISABLE_COPY(PushButton)

public:
    explicit PushButton(QWidget* parent = nullptr);
    QSize sizeHint() const override;

private:
    void paintEvent(QPaintEvent*) override;
};

#endif // PUSHBUTTON_H