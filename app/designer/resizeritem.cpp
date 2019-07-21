#include <resizeritem.h>
#include <utilityfunctions.h>
#include <QPainter>

ResizerItem::ResizerItem(Placement placement, DesignerItem* parent) : ToolItem(parent)
  , m_placement(placement)
{
    setRect(-3, -3, 6, 6);
    updateCursor();
}

void ResizerItem::updateCursor()
{
    switch (m_placement) {
    case ResizerItem::Top:
        setCursor(Qt::SizeVerCursor);
        break;
    case ResizerItem::Right:
        setCursor(Qt::SizeHorCursor);
        break;
    case ResizerItem::Bottom:
        setCursor(Qt::SizeVerCursor);
        break;
    case ResizerItem::Left:
        setCursor(Qt::SizeHorCursor);
        break;
    case ResizerItem::TopLeft:
        setCursor(Qt::SizeFDiagCursor);
        break;
    case ResizerItem::TopRight:
        setCursor(Qt::SizeBDiagCursor);
        break;
    case ResizerItem::BottomRight:
        setCursor(Qt::SizeFDiagCursor);
        break;
    case ResizerItem::BottomLeft:
        setCursor(Qt::SizeBDiagCursor);
        break;
    }
}

void ResizerItem::updatePosition()
{
    if (parentItem() == 0)
        return;

    const QRectF& parentRect = parentItem()->rect();

    switch (m_placement) {
    case ResizerItem::Top:
        setPos(UtilityFunctions::topCenter(parentRect));
        break;
    case ResizerItem::Right:
        setPos(UtilityFunctions::rightCenter(parentRect));
        break;
    case ResizerItem::Bottom:
        setPos(UtilityFunctions::bottomCenter(parentRect));
        break;
    case ResizerItem::Left:
        setPos(UtilityFunctions::leftCenter(parentRect));
        break;
    case ResizerItem::TopLeft:
        setPos(parentRect.topLeft());
        break;
    case ResizerItem::TopRight:
        setPos(parentRect.topRight());
        break;
    case ResizerItem::BottomRight:
        setPos(parentRect.bottomRight());
        break;
    case ResizerItem::BottomLeft:
        setPos(parentRect.bottomLeft());
        break;
    }
}

void ResizerItem::setParentGeometry(const QPointF& dragDistance)
{
    if (parentItem() == 0)
        return;

    QRectF geometry = parentItem()->geometry();

    switch (m_placement) {
    case Top:
        geometry.setTop(dragDistance.y());
        break;
    case Right:
        geometry.setRight(dragDistance.x());
        break;
    case Bottom:
        geometry.setBottom(dragDistance.y());
        break;
    case Left:
        geometry.setLeft(dragDistance.x());
        break;
    case TopLeft:
        geometry.setTopLeft(dragDistance);
        break;
    case TopRight:
        geometry.setTopRight(dragDistance);
        break;
    case BottomRight:
        geometry.setBottomRight(dragDistance);
        break;
    case BottomLeft:
        geometry.setBottomLeft(dragDistance);
        break;
    }

    parentItem()->setGeometry(geometry);
}

void ResizerItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    ToolItem::mouseMoveEvent(event);

    if (parentItem() && dragAccepted()) {
        parentItem()->setBeingDragged(true);
        parentItem()->setBeingResized(true);
        setParentGeometry(parentItem()->mapToParent(mapToParent(dragDistanceVector())));
    }
}

void ResizerItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    if (parentItem() && dragAccepted()) {
        parentItem()->setBeingDragged(false);
        parentItem()->setBeingResized(false);
    }

    ToolItem::mouseReleaseEvent(event); // Clears dragAccepted state
}

void ResizerItem::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
{
    painter->setPen(pen());
    painter->setBrush(brush());
    painter->drawRect(rect().adjusted(0, 0, -0.5, -0.5));
}
