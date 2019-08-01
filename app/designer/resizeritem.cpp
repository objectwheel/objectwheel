#include <resizeritem.h>
#include <utilityfunctions.h>
#include <QPainter>

ResizerItem::ResizerItem(Placement placement, DesignerItem* parent) : GadgetItem(parent)
  , m_placement(placement)
{
    setRect(-3, -3, 6, 6);
    updateCursor();
}

DesignerItem* ResizerItem::targetItem() const
{
    return m_targetItem;
}

void ResizerItem::setTargetItem(DesignerItem* targetItem)
{
    if (m_targetItem != targetItem) {
        if (m_targetItem)
            m_targetItem->disconnect(this);
        m_targetItem = targetItem;
        if (m_targetItem) {
            connect(m_targetItem, &DesignerItem::resizableChanged,
                    this, [=] { if (dragAccepted() && !m_targetItem->resizable()) ungrabMouse(); });
        }
        updatePosition();
    }
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
    if (targetItem() == 0)
        return;

    const QRectF& targetRect = targetItem()->mapRectToScene(targetItem()->rect());

    switch (m_placement) {
    case ResizerItem::Top:
        setPos(UtilityFunctions::topCenter(targetRect));
        break;
    case ResizerItem::Right:
        setPos(UtilityFunctions::rightCenter(targetRect));
        break;
    case ResizerItem::Bottom:
        setPos(UtilityFunctions::bottomCenter(targetRect));
        break;
    case ResizerItem::Left:
        setPos(UtilityFunctions::leftCenter(targetRect));
        break;
    case ResizerItem::TopLeft:
        setPos(targetRect.topLeft());
        break;
    case ResizerItem::TopRight:
        setPos(targetRect.topRight());
        break;
    case ResizerItem::BottomRight:
        setPos(targetRect.bottomRight());
        break;
    case ResizerItem::BottomLeft:
        setPos(targetRect.bottomLeft());
        break;
    }
}

void ResizerItem::updateTargetGeometry()
{
    if (targetItem() == 0)
        return;

    qreal dx = dragDistanceVector().x();
    qreal dy = dragDistanceVector().y();
    QRectF geometry = targetItem()->geometry();

    switch (m_placement) {
    case Top:
        geometry.adjust(0, dy, 0, 0);
        break;
    case Right:
        geometry.adjust(0, 0, dx, 0);
        break;
    case Bottom:
        geometry.adjust(0, 0, 0, dy);
        break;
    case Left:
        geometry.adjust(dx, 0, 0, 0);
        break;
    case TopLeft:
        geometry.adjust(dx, dy, 0, 0);
        break;
    case TopRight:
        geometry.adjust(0, dy, dx, 0);
        break;
    case BottomRight:
        geometry.adjust(0, 0, dx, dy);
        break;
    case BottomLeft:
        geometry.adjust(dx, 0, 0, dy);
        break;
    }

    targetItem()->setGeometry(geometry);
}

void ResizerItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    GadgetItem::mouseMoveEvent(event);

    if (dragAccepted() && targetItem() && targetItem()->resizable()) {
        if (!targetItem()->beingResized()) {
            targetItem()->setBeingResized(true);
            targetItem()->setBeingDragged(true); // Because pos() may also change
        }
        updateTargetGeometry();
    }
}

void ResizerItem::mouseUngrabEvent(QEvent* event)
{
    if (dragAccepted() && targetItem() && targetItem()->beingResized()) {
        targetItem()->setBeingDragged(false);
        targetItem()->setBeingResized(false);
    }

    GadgetItem::mouseUngrabEvent(event); // Clears dragAccepted state
}

void ResizerItem::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
{
    painter->setPen(pen());
    painter->setBrush(brush());
    painter->drawRect(rect().adjusted(0, 0, -0.5, -0.5));
}
