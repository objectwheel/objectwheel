#include <resizeritem.h>
#include <controlpropertymanager.h>
#include <designerscene.h>
#include <utilityfunctions.h>
#include <QPainter>

ResizerItem::ResizerItem(Placement placement, Control* parent) : DesignerItem(parent)
  , m_placement(placement)
{
    updateCursor();
}

QList<ResizerItem*> ResizerItem::init(Control* control)
{
    QList<ResizerItem*> resizers;
    for (int i = 0; i < 8; ++i)
        resizers.append(new ResizerItem(ResizerItem::Placement(i), control));
    return resizers;
}

QRectF ResizerItem::boundingRect() const
{
    return {-3.0, -3.0, 6.0, 6.0};
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
    const QRectF& parentRect = parentControl()->rect();
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
    QRectF geometry(parentControl()->geometry());
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

    if (!dragStarted())
        return;

    parentControl()->setResized(true);

    const QRectF& geometry = calculateParentGeometry(snapPosition());
    ControlPropertyManager::Options option = ControlPropertyManager::SaveChanges
            | ControlPropertyManager::UpdateRenderer
            | ControlPropertyManager::CompressedCall;

    if (parentControl()->form()) {
        ControlPropertyManager::setSize(parentControl(), geometry.size(), option);
        ControlPropertyManager::setPos(parentControl(), geometry.topLeft(),
                                       ControlPropertyManager::NoOption);
    } else {
        ControlPropertyManager::setGeometry(parentControl(), geometry, option);
    }
}

void ResizerItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    DesignerItem::mouseReleaseEvent(event);
    parentControl()->setResized(false);
}

void ResizerItem::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
{
    painter->setPen(pen());
    painter->setBrush(brush());
    painter->drawRect(boundingRect().adjusted(0, 0, -0.5, -0.5));
}
