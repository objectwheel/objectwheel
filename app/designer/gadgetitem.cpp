#include <gadgetitem.h>
#include <QGraphicsSceneMouseEvent>

GadgetItem::GadgetItem(DesignerItem* parent) : DesignerItem(parent)
  , m_brush(Qt::white)
{
    QPen pen(Qt::black, 1, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin);
    pen.setCosmetic(true);
    setPen(pen);
    setVisible(false);
    setFlag(ItemIgnoresTransformations);
}

QPen GadgetItem::pen() const
{
    return m_pen;
}

void GadgetItem::setPen(const QPen& pen)
{
    if (m_pen != pen) {
        m_pen = pen;
        update();
    }
}

QBrush GadgetItem::brush() const
{
    return m_brush;
}

void GadgetItem::setBrush(const QBrush& brush)
{
    if (m_brush != brush) {
        m_brush = brush;
        update();
    }
}

QPointF GadgetItem::dragDistanceVector() const
{
    return m_dragDistanceVector;
}

void GadgetItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    DesignerItem::mousePressEvent(event);
    event->accept();
}

void GadgetItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    DesignerItem::mouseMoveEvent(event);
    m_dragDistanceVector = event->pos() - mousePressPoint();
}

void GadgetItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    DesignerItem::mouseReleaseEvent(event);
    m_dragDistanceVector = QPointF();
}
