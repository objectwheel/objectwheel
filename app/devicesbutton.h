#ifndef DEVICESBUTTON_H
#define DEVICESBUTTON_H

#include <QPushButton>

class DevicesButton : public QPushButton
{
    Q_OBJECT
public:
    explicit DevicesButton(QWidget* parent = nullptr);

private:
    void paintEvent(QPaintEvent*) override;
    QSize sizeHint() const override;

private:
    QPixmap m_devicesPixmap;
};

#endif // DEVICESBUTTON_H