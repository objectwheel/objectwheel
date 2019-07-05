#include <resizer.h>
#include <control.h>
#include <controlpropertymanager.h>
#include <designerscene.h>
#include <utilityfunctions.h>
#include <designersettings.h>
#include <scenesettings.h>

#include <QCursor>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>

Resizer::Resizer(Placement placement, Control* parent) : QGraphicsItem(parent)
  , m_placement(placement)
{
    setVisible(false);
    setAcceptedMouseButtons(Qt::LeftButton);
    setZValue(std::numeric_limits<qreal>::max());
    setFlag(ItemIsMovable);
    setFlag(ItemSendsGeometryChanges);
    setFlag(ItemClipsToShape);
    setFlag(ItemIgnoresTransformations);
    setFlag(ItemIsFocusable);
    setFlag(ItemIsSelectable);
//    updateCursor();
}

DesignerScene* Resizer::scene() const
{
    return static_cast<DesignerScene*>(QGraphicsItem::scene());
}

Control* Resizer::parentControl() const
{
    return static_cast<Control*>(parentItem());
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
    using namespace UtilityFunctions;
//    const QRectF& parentRect = parentControl()->rect();
//    switch (m_placement) {
//    case Resizer::Top:
//        setPos(topCenter(parentRect));
//        break;
//    case Resizer::Right:
//        setPos(rightCenter(parentRect));
//        break;
//    case Resizer::Bottom:
//        setPos(bottomCenter(parentRect));
//        break;
//    case Resizer::Left:
//        setPos(leftCenter(parentRect));
//        break;
//    case Resizer::TopLeft:
//        setPos(parentRect.topLeft());
//        break;
//    case Resizer::TopRight:
//        setPos(parentRect.topRight());
//        break;
//    case Resizer::BottomRight:
//        setPos(parentRect.bottomRight());
//        break;
//    case Resizer::BottomLeft:
//        setPos(parentRect.bottomLeft());
//        break;
//    }
}

QVariant Resizer::itemChange(GraphicsItemChange change, const QVariant& value)
{
    /*const SceneSettings* settings = DesignerSettings::sceneSettings();
    if (change == ItemPositionChange && scene() && settings->snappingEnabled) {
        const QPointF& newPos = value.toPointF();
        qreal xV = qRound(newPos.x() / settings->gridSize) * settings->gridSize;
        qreal yV = qRound(newPos.y() / settings->gridSize) * settings->gridSize;
        return QPointF(xV, yV);
    } else */{
        return QGraphicsItem::itemChange(change, value);
    }
}

QList<Resizer*> Resizer::init(Control* control)
{
    QList<Resizer*> resizers;
    for (int i = 0; i < 8; ++i)
        resizers.append(new Resizer(Resizer::Placement(i), control));
    return resizers;
}

QRectF Resizer::boundingRect() const
{
    return {-3.0, -3.0, 6.0, 6.0};
}

void Resizer::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
{
    painter->setBrush(Qt::white);
    painter->setPen(scene()->highlightPen());
    painter->setRenderHint(QPainter::Antialiasing, false);
    painter->drawRect(boundingRect().adjusted(0, 0, -0.5, -0.5));
}
