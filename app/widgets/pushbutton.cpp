#include <pushbutton.h>
#include <paintutils.h>

#include <QStylePainter>
#include <QStyleOption>

PushButton::PushButton(QWidget* parent) : QPushButton(parent)
{
    PaintUtils::setPanelButtonPaletteDefaults(this);
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
}

QSize PushButton::sizeHint() const
{
    return QSize(QPushButton::sizeHint().width(), 24);
}

void PushButton::paintEvent(QPaintEvent*)
{
    QStylePainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Draw background
    QStyleOptionButton option;
    initStyleOption(&option);
    PaintUtils::drawPanelButtonBevel(&painter, option);

    // Draw the rest (FIXME: Remove this "&& option.palette.buttonText().color() != Qt::white")
    if ((isDown() || isChecked()) && option.palette.buttonText().color() != Qt::white)
        option.palette.setColor(QPalette::ButtonText, option.palette.buttonText().color().darker());

    // qfusionstyle.cpp selects QIcon::Active icon when State_HasFocus is present
    option.state &= ~QStyle::State_HasFocus;
    if (isDown() || isChecked())
        option.state |= QStyle::State_HasFocus;
    option.rect = style()->subElementRect(QStyle::SE_PushButtonContents, &option, this);
    painter.drawControl(QStyle::CE_PushButtonLabel, option);
}
