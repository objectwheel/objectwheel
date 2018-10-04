#include <runpanebutton.h>
#include <paintutils.h>
#include <utilityfunctions.h>

#include <QPainter>
#include <QStyleOption>

RunPaneButton::RunPaneButton(QWidget* parent) : QAbstractButton(parent)
{
}

void RunPaneButton::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    // Draw background
    QStyleOptionButton opt;
    opt.initFrom(this);
    opt.state |= isDown() ? QStyle::State_Sunken : QStyle::State_Raised;
    PaintUtils::drawMacStyleButtonBackground(&p, opt, this);

    // Draw icon
    const QPixmap& pix = UtilityFunctions::pixmap(this, icon(), iconSize());
    p.drawPixmap(UtilityFunctions::verticalAlignedRect(iconSize(), rect(), -1), pix, pix.rect());
}

QSize RunPaneButton::minimumSizeHint() const
{
    return QSize(39, 24);
}

QSize RunPaneButton::sizeHint() const
{
    return QSize(39, 24);
}
