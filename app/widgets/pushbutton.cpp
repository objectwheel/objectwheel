#include <pushbutton.h>
#include <paintutils.h>
#include <interfacesettings.h>
#include <generalsettings.h>

#include <QStylePainter>
#include <QStyleOption>

PushButton::PushButton(QWidget* parent) : QPushButton(parent)
{
    QPalette p(palette());
    PaintUtils::setPanelButtonPaletteDefaults(p, GeneralSettings::interfaceSettings()->theme == "Light");
    setPalette(p);
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
}

void PushButton::paintEvent(QPaintEvent*)
{
    QStylePainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    // Draw background
    QStyleOptionButton opt;
    initStyleOption(&opt);
    PaintUtils::drawPanelButtonBevel(&p, opt);

    // Draw the rest (FIXME: Remove this "&& opt.palette.buttonText().color() != Qt::white")
    if ((isDown() || isChecked()) && opt.palette.buttonText().color() != Qt::white)
        opt.palette.setColor(QPalette::ButtonText, opt.palette.buttonText().color().darker());
    opt.state |= isDown() ? QStyle::State_On : QStyle::State_Off;
    opt.rect = style()->subElementRect(QStyle::SE_PushButtonContents, &opt, this);
    p.drawControl(QStyle::CE_PushButtonLabel, opt);
}

QSize PushButton::minimumSizeHint() const
{
    return sizeHint();
}

QSize PushButton::sizeHint() const
{
    return QSize(QPushButton::sizeHint().width(), 24);
}