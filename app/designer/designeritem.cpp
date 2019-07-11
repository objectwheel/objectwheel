#include <designeritem.h>
#include <designerscene.h>
#include <QGraphicsSceneMouseEvent>

DesignerItem::DesignerItem(Control* parent) : QGraphicsObject(parent)
  , m_pen(Qt::white)
  , m_brush(DesignerScene::outlineColor())
  , m_dragStarted(false)
{
    setVisible(false);
    setFlag(ItemClipsToShape);
    setFlag(ItemIgnoresTransformations);
    setAcceptedMouseButtons(Qt::LeftButton);
    setZValue(std::numeric_limits<int>::max());
}

Control* DesignerItem::parentControl() const
{
    return static_cast<Control*>(parentItem());
}

DesignerScene* DesignerItem::scene() const
{
    return static_cast<DesignerScene*>(QGraphicsItem::scene());
}

QPen DesignerItem::pen() const
{
    return m_pen;
}

void DesignerItem::setPen(const QPen& pen)
{
    m_pen = pen;
    update();
}

QBrush DesignerItem::brush() const
{
    return m_brush;
}

void DesignerItem::setBrush(const QBrush& brush)
{
    m_brush = brush;
    update();
}

bool DesignerItem::dragStarted() const
{
    return m_dragStarted;
}

QPointF DesignerItem::snapPosition() const
{
    return m_snapPosition;
}

void DesignerItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event)
{
    event->ignore();
    emit doubleClicked();
}

void DesignerItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    Q_UNUSED(event)
    m_dragStartPoint = event->pos();
}

void DesignerItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    const QPointF& diff = event->pos() - m_dragStartPoint;
    if (m_dragStarted || diff.manhattanLength() >= scene()->startDragDistance()) {
        m_dragStarted = true;
        m_snapPosition = scene()->snapPosition(parentControl()->mapToParent(mapToParent(diff)));
    }
}

void DesignerItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    Q_UNUSED(event)
    m_dragStarted = false;
}
