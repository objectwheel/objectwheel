#include <pushbutton.h>
#include <paintutils.h>

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

    // Draw background
    if (!isFlat())
        PaintUtils::drawPanelButtonBevel(&painter, option);

    // Draw label
    option.rect = style()->subElementRect(QStyle::SE_PushButtonContents, &option, this);
    painter.drawControl(QStyle::CE_PushButtonLabel, option);
}
