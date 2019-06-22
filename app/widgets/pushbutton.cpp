#include <pushbutton.h>
#include <paintutils.h>
#include <utilityfunctions.h>

#include <QStylePainter>
#include <QStyleOption>

PushButton::PushButton(QWidget* parent) : QPushButton(parent)
{
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

    QStyleOptionButton option;
    initStyleOption(&option);
    if (!UtilityFunctions::hasHover(this)) // FIXME: This is a workaround for QTBUG-44400
        option.state &= ~QStyle::State_MouseOver;

    // Draw background
    if (!isFlat())
        PaintUtils::drawPanelButtonBevel(&painter, option);

    // Draw label
    option.rect = style()->subElementRect(QStyle::SE_PushButtonContents, &option, this);
    painter.drawControl(QStyle::CE_PushButtonLabel, option);
}
