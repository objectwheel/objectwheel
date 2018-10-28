#include <form.h>
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
    scene()->centralize();
}

void Form::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    // We don't propagate the mouse press event in order to block selection on form.
    // Hence rubber band is activated on form.
    event->ignore();
}

void Form::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    painter->fillRect(rect(), Qt::white);

    Control::paint(painter, option, widget);

    painter->setPen(isSelected() ? "#4BA2FF" : "#b4b4b4");
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(rect().adjusted(0.5, 0.5, -0.5, -0.5));
}