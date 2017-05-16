#include <editmodeindicator.h>
#include <fit.h>
#include <QPainter>
#include <QGraphicsDropShadowEffect>

using namespace Fit;

QGraphicsDropShadowEffect eff;

void EditModeIndicator::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QColor color(m_on ? "#6BB64B" : "#8A604B");
    QLinearGradient grad(width() / 2.0, 0, width()/2.0, height());
    grad.setColorAt(0, color);
    grad.setColorAt(1, color.darker(115));
    painter.setBrush(grad);
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(rect(),fit(3), fit(3));

    painter.setPen(QPen("#3E4651"));
    painter.setBrush(QBrush("#3E4651"));
    painter.drawRect(0, 0, fit(6), height());

    QFont f;
    f.setPixelSize(fit(12));
    painter.setFont(f);
    painter.setPen(Qt::white);
    painter.drawText(rect().adjusted(fit(9), 0, 0, 0), m_on ? "Edit: On" : "Edit: Off", Qt::AlignVCenter | Qt::AlignLeft);
}

EditModeIndicator::EditModeIndicator(QWidget *parent) : QWidget(parent)
{
    resize(fit(62), fit(18));
    eff.setOffset(0, fit(1));
    eff.setBlurRadius(fit(3));
    eff.setColor("#60000000");
    setGraphicsEffect(&eff);
}
