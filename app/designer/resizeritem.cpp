#include <resizeritem.h>
#include <designerscene.h>
#include <utilityfunctions.h>
#include <QPainter>

ResizerItem::ResizerItem(Placement placement, DesignerItem* parent) : DesignerItem(parent)
  , m_placement(placement)
{
    setFlag(ItemIgnoresTransformations);
    setZValue(std::numeric_limits<int>::max());
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

QRectF ResizerItem::calculateParentGeometry(const QPointF& snapPos)
{
    QRectF geometry(parentItem()->geometry());
    switch (m_placement) {
    case Top:
        geometry.setTop(snapPos.y());
        break;
    case Right:
        geometry.setRight(snapPos.x());
        break;
    case Bottom:
        geometry.setBottom(snapPos.y());
        break;
    case Left:
        geometry.setLeft(snapPos.x());
        break;
    case TopLeft:
        geometry.setTopLeft(snapPos);
        break;
    case TopRight:
        geometry.setTopRight(snapPos);
        break;
    case BottomRight:
        geometry.setBottomRight(snapPos);
        break;
    case BottomLeft:
        geometry.setBottomLeft(snapPos);
        break;
    }
    return geometry;
}

void ResizerItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    DesignerItem::mouseMoveEvent(event);

    if (!dragDistanceExceeded())
        return;

    Q_ASSERT(parentItem() && parentItem()->controlCast());

    parentItem()->controlCast()->setResized(true);

    scene()->snapPosition(parentItem()->mapToParent(mapToParent(diff)));
    const QRectF& geometry = calculateParentGeometry(snapPosition());
    ControlPropertyManager::Options option = ControlPropertyManager::SaveChanges
            | ControlPropertyManager::UpdateRenderer
            | ControlPropertyManager::CompressedCall;

    if (parentItem()->controlCast()->form()) {
        ControlPropertyManager::setSize(parentItem()->controlCast(), geometry.size(), option);
        ControlPropertyManager::setPos(parentItem()->controlCast(), geometry.topLeft(),
                                       ControlPropertyManager::NoOption);
    } else {
        ControlPropertyManager::setGeometry(parentItem()->controlCast(), geometry, option);
    }
}

void ResizerItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    Q_ASSERT(parentItem() && parentItem()->controlCast());
    DesignerItem::mouseReleaseEvent(event);
    parentItem()->controlCast()->setResized(false);
}

void ResizerItem::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
{
    painter->setPen(pen());
    painter->setBrush(brush());
    painter->drawRect(rect().adjusted(0, 0, -0.5, -0.5));
}
