#include <resizer.h>
#include <controlpropertymanager.h>
#include <designerscene.h>
#include <utilityfunctions.h>

#include <QGraphicsSceneMouseEvent>
#include <QPainter>

Resizer::Resizer(Placement placement, Control* parent) : QGraphicsItem(parent)
  , m_placement(placement)
{
    setVisible(false);
    setFlag(ItemClipsToShape);
    setFlag(ItemIgnoresTransformations);
    setAcceptedMouseButtons(Qt::LeftButton);
    setZValue(std::numeric_limits<int>::max());
    updateCursor();
}

QList<Resizer*> Resizer::init(Control* control)
{
    QList<Resizer*> resizers;
    for (int i = 0; i < 8; ++i)
        resizers.append(new Resizer(Resizer::Placement(i), control));
    return resizers;
}

DesignerScene* Resizer::scene() const
{
    return static_cast<DesignerScene*>(QGraphicsItem::scene());
}

Control* Resizer::parentControl() const
{
    return static_cast<Control*>(parentItem());
}

QRectF Resizer::boundingRect() const
{
    return {-3.0, -3.0, 6.0, 6.0};
}

void Resizer::updateCursor()
{
    switch (m_placement) {
    case Resizer::Top:
        setCursor(Qt::SizeVerCursor);
        break;
    case Resizer::Right:
        setCursor(Qt::SizeHorCursor);
        break;
    case Resizer::Bottom:
        setCursor(Qt::SizeVerCursor);
        break;
    case Resizer::Left:
        setCursor(Qt::SizeHorCursor);
        break;
    case Resizer::TopLeft:
        setCursor(Qt::SizeFDiagCursor);
        break;
    case Resizer::TopRight:
        setCursor(Qt::SizeBDiagCursor);
        break;
    case Resizer::BottomRight:
        setCursor(Qt::SizeFDiagCursor);
        break;
    case Resizer::BottomLeft:
        setCursor(Qt::SizeBDiagCursor);
        break;
    }
}

void Resizer::updatePosition()
{
    const QRectF& parentRect = parentControl()->rect();
    switch (m_placement) {
    case Resizer::Top:
        setPos(UtilityFunctions::topCenter(parentRect));
        break;
    case Resizer::Right:
        setPos(UtilityFunctions::rightCenter(parentRect));
        break;
    case Resizer::Bottom:
        setPos(UtilityFunctions::bottomCenter(parentRect));
        break;
    case Resizer::Left:
        setPos(UtilityFunctions::leftCenter(parentRect));
        break;
    case Resizer::TopLeft:
        setPos(parentRect.topLeft());
        break;
    case Resizer::TopRight:
        setPos(parentRect.topRight());
        break;
    case Resizer::BottomRight:
        setPos(parentRect.bottomRight());
        break;
    case Resizer::BottomLeft:
        setPos(parentRect.bottomLeft());
        break;
    }
}

QRectF Resizer::calculateParentGeometry(const QPointF& snapPos)
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

void Resizer::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    m_dragStartPoint = event->pos();
}

void Resizer::mouseReleaseEvent(QGraphicsSceneMouseEvent*)
{
    parentControl()->setResized(false);
}

void Resizer::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    const QPointF& diff = event->pos() - m_dragStartPoint;
    if (!parentControl()->resized() && diff.manhattanLength() < scene()->startDragDistance())
        return;

    parentControl()->setResized(true);
    const QPointF& snapPos = scene()->snapPosition(parentControl()->mapToParent(mapToParent(diff)));
    const QRectF& geometry = calculateParentGeometry(snapPos);

    ControlPropertyManager::Options option = ControlPropertyManager::SaveChanges
            | ControlPropertyManager::UpdateRenderer
            | ControlPropertyManager::CompressedCall;

    if (parentControl()->form()) {
        ControlPropertyManager::setSize(parentControl(), geometry.size(), option);
        ControlPropertyManager::setPos(parentControl(), geometry.topLeft(), ControlPropertyManager::NoOption);
    } else {
        ControlPropertyManager::setGeometry(parentControl(), geometry, option);
    }
}

void Resizer::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
{
    painter->setBrush(Qt::white);
    painter->setPen(scene()->pen());
    painter->drawRect(boundingRect().adjusted(0, 0, -0.5, -0.5));
}
