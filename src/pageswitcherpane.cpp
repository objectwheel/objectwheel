#include <pageswitcherpane.h>
#include <QPainter>

PageSwitcherPane::PageSwitcherPane(QWidget *parent) : QWidget(parent)
{
}

void PageSwitcherPane::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), "#465059");
    painter.setPen("#1e2326");
    painter.drawLine(QRectF(rect()).topRight() + QPointF(-0.5, 0.5), QRectF(rect()).bottomRight() + QPointF(-0.5, -0.5));
}
