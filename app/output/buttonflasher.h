#ifndef BUTTONFLASHER_H
#define BUTTONFLASHER_H

#include <applicationstyle.h>
#include <QVariantAnimation>

class QAbstractButton;
class ButtonFlasher final : private ApplicationStyle
{
    Q_OBJECT
    Q_DISABLE_COPY(ButtonFlasher)

public:
    explicit ButtonFlasher(QAbstractButton* parent);

    void flash(int timeout = 500, int repeat = 3);

private:
    void drawControl(QStyle::ControlElement element, const QStyleOption* option,
                     QPainter* painter, const QWidget* widget) const override;

private:
    QVariantAnimation m_colorOpacity;
    QAbstractButton* m_button;
};

#endif // BUTTONFLASHER_H
