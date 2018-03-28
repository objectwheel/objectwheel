#include <form.h>
#include <fit.h>
#include <designerscene.h>

#include <QGraphicsSceneMouseEvent>
#include <QPainter>

Form::Form(const QString& url, Form* parent) : Control(url, parent)
  , m_main(false)
{
    setFlag(ItemIsMovable, false);
}

bool Form::main() const
{
    return m_main;
}

QRectF Form::frameGeometry() const
{
    return QRectF(QPointF(-size().width() / 2.0, -size().height() / 2.0), size());
}

void Form::setMain(bool value)
{
    m_main = value;
}

void Form::resizeEvent(QGraphicsSceneResizeEvent* event)
{
    Control::resizeEvent(event);
    centralize();
}

void Form::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    event->ignore();
}

void Form::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor("#F0F4F7"));
    painter->drawRect(rect());

    Control::paint(painter, option, widget);

    if (!isSelected() && !scene()->showOutlines()) {
        QPen pen;
        pen.setWidthF(fit::fx(1));
        pen.setStyle(Qt::DotLine);
        pen.setColor("#808487");

        painter->setPen(pen);
        painter->setBrush(Qt::transparent);
        painter->drawRect(rect().adjusted(0.5, 0.5, -0.5, -0.5));
    }
}