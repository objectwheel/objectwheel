#include <toolitem.h>
#include <QGraphicsSceneMouseEvent>

ToolItem::ToolItem(DesignerItem* parent) : DesignerItem(parent)
  , m_brush(Qt::white)
{
    QPen pen(Qt::black, 1, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin);
    pen.setCosmetic(true);
    setPen(pen);
    setVisible(false);
    setFlag(ItemIgnoresTransformations);
    setZValue(std::numeric_limits<int>::max());
}

QPen ToolItem::pen() const
{
    return m_pen;
}

void ToolItem::setPen(const QPen& pen)
{
    m_pen = pen;
    update();
}

QBrush ToolItem::brush() const
{
    return m_brush;
}

void ToolItem::setBrush(const QBrush& brush)
{
    m_brush = brush;
    update();
}

QPointF ToolItem::dragDistanceVector() const
{
    return m_dragDistanceVector;
}

void ToolItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    DesignerItem::mouseMoveEvent(event);
    m_dragDistanceVector = event->pos() - mousePressPoint();
}

void ToolItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    DesignerItem::mouseReleaseEvent(event);
    m_dragDistanceVector = QPointF();
}
