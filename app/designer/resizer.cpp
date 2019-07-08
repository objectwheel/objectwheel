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
#include <QTimer>
#include <QDebug>

Resizer::Resizer(Placement placement, Control* parent) : QGraphicsItem(parent)
  , m_collectiveDx(0)
  , m_collectiveDy(0)
  , m_placement(placement)
{
    setVisible(false);
    setFlag(ItemClipsToShape);
    setFlag(ItemIgnoresTransformations);
    setAcceptedMouseButtons(Qt::LeftButton);
    setZValue(std::numeric_limits<qreal>::max());
    updateCursor();
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
    const QRectF& parentRect = parentControl()->rect();
    prepareGeometryChange();
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

void Resizer::calculatePositionDifference(const QGraphicsSceneMouseEvent* event, qreal* dx, qreal* dy)
{
    *dx = 0, *dy = 0;

    switch (m_placement) {
    case Top:
        *dy = event->lastPos().y() - event->pos().y();
        break;
    case Right:
        *dx = event->pos().x() - event->lastPos().x();
        break;
    case Bottom:
        *dy = event->pos().y() - event->lastPos().y();
        break;
    case Left:
        *dx = event->lastPos().x() - event->pos().x();
        break;
    case TopLeft:
        *dx = event->lastPos().x() - event->pos().x();
        *dy = event->lastPos().y() - event->pos().y();
        break;
    case TopRight:
        *dx = event->pos().x() - event->lastPos().x();
        *dy = event->lastPos().y() - event->pos().y();
        break;
    case BottomRight:
        *dx = event->pos().x() - event->lastPos().x();
        *dy = event->pos().y() - event->lastPos().y();
        break;
    case BottomLeft:
        *dx = event->lastPos().x() - event->pos().x();
        *dy = event->pos().y() - event->lastPos().y();
        break;
    }
}

void Resizer::mousePressEvent(QGraphicsSceneMouseEvent*)
{
    m_collectiveDx = 0;
    m_collectiveDy = 0;
    parentControl()->setResized(true);
}

void Resizer::mouseReleaseEvent(QGraphicsSceneMouseEvent*)
{
    m_collectiveDx = 0;
    m_collectiveDy = 0;
    parentControl()->setResized(false);
}

void Resizer::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    if (!isEnabled())
        return;

    if (!parentControl()->resized())
        return;

    qreal dx, dy;
    const SceneSettings* settings = DesignerSettings::sceneSettings();
    const qreal parentWidth = parentControl()->size().width();
    const qreal parentHeight = parentControl()->size().height();
    const auto& shift = [this] (qreal x1, qreal y1, qreal x2, qreal y2) {
        return parentControl()->geometry().adjusted(x1, y1, x2, y2);
    };

    ControlPropertyManager::Options option = ControlPropertyManager::SaveChanges
            | ControlPropertyManager::UpdateRenderer
            | ControlPropertyManager::CompressedCall;

    calculatePositionDifference(event, &dx, &dy);

    if (dx == 0 && dy == 0)
        return;

    if (parentWidth + dx < 10 || parentHeight + dy < 10)
        return;

//    m_collectiveDy += dy;
//    m_collectiveDx += dx;

//    qreal closestX = qRound((parentControl()->x() - m_collectiveDx) / settings->gridSize) * settings->gridSize;
//    qreal closestY = qRound((parentControl()->y() - m_collectiveDy) / settings->gridSize) * settings->gridSize;

//    if (closestX == parentControl()->x() && closestY == parentControl()->y())
//        return;

//    if (closestX != parentControl()->x()) {
//        dx = parentControl()->x() - closestX;
//        m_collectiveDx -= dx;
//    }

//    if (closestY != parentControl()->y()) {
//        dy = parentControl()->y() - closestY;
//        m_collectiveDy -= dy;
//    }

    switch (m_placement) {
    case Top:
        if (parentControl()->form())
            ControlPropertyManager::setHeight(parentControl(), parentHeight + dy, option);
        else
            ControlPropertyManager::setGeometry(parentControl(), shift(0, -dy, 0, 0), option);
        break;
    case Right:
        if (parentControl()->form())
            ControlPropertyManager::setWidth(parentControl(), parentWidth + dx, option);
        else
            ControlPropertyManager::setGeometry(parentControl(), shift(0, 0, dx, 0), option);
        break;
    case Bottom:
        if (parentControl()->form())
            ControlPropertyManager::setHeight(parentControl(), parentHeight + dy, option);
        else
            ControlPropertyManager::setGeometry(parentControl(), shift(0, 0, 0, dy), option);
        break;
    case Left:
        if (parentControl()->form())
            ControlPropertyManager::setWidth(parentControl(), parentWidth + dx, option);
        else
            ControlPropertyManager::setGeometry(parentControl(), shift(-dx, 0, 0, 0), option);
        break;
    case TopLeft:
        if (parentControl()->form())
            ControlPropertyManager::setSize(parentControl(), {parentWidth + dx, parentHeight + dy}, option);
        else
            ControlPropertyManager::setGeometry(parentControl(), shift(-dx, -dy, 0, 0), option);
        break;
    case TopRight:
        if (parentControl()->form())
            ControlPropertyManager::setSize(parentControl(), {parentWidth + dx, parentHeight + dy}, option);
        else
            ControlPropertyManager::setGeometry(parentControl(), shift(0, -dy, dx, 0), option);
        break;
    case BottomRight:
        if (parentControl()->form())
            ControlPropertyManager::setSize(parentControl(), {parentWidth + dx, parentHeight + dy}, option);
        else
            ControlPropertyManager::setGeometry(parentControl(), shift(0, 0, dx, dy), option);
        break;
    case BottomLeft:
        if (parentControl()->form())
            ControlPropertyManager::setSize(parentControl(), {parentWidth + dx, parentHeight + dy}, option);
        else
            ControlPropertyManager::setGeometry(parentControl(), shift(-dx, 0, 0, dy), option);
        break;
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
    painter->fillRect(boundingRect(), Qt::red);
    painter->setBrush(Qt::white);
    painter->setPen(scene()->pen());
    painter->setRenderHint(QPainter::Antialiasing, false);
    painter->drawRect(boundingRect().adjusted(0, 0, -0.5, -0.5));
}

