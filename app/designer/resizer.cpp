#include <resizer.h>
#include <control.h>
#include <controlpropertymanager.h>

#include <QCursor>
#include <QGraphicsSceneMouseEvent>

Resizer::Resizer(Placement placement, Control* parent) : QGraphicsRectItem(0, 0, 5, 5, parent)
{
    setVisible(false);
    setBrush(Qt::white);
    setPlacement(placement);
    setFlag(QGraphicsItem::ItemClipsToShape);
    setAcceptedMouseButtons(Qt::LeftButton);
    setZValue(std::numeric_limits<qreal>::max());
}

Control* Resizer::parentControl() const
{
    return static_cast<Control*>(parentItem());
}

Resizer::Placement Resizer::placement() const
{
    return m_placement;
}

void Resizer::setPlacement(Resizer::Placement placement)
{
    m_placement = placement;
    updateCursor();
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
    const qreal length = rect().width();
    const qreal parentWidth = parentControl()->size().width();
    const qreal parentHeight = parentControl()->size().height();

    switch (m_placement) {
    case Resizer::Top:
        setPos(parentWidth / 2 - length / 2, -length / 2);
        break;
    case Resizer::Right:
        setPos(parentWidth - length / 2, parentHeight / 2 - length / 2);
        break;
    case Resizer::Bottom:
        setPos(parentWidth / 2 - length / 2, parentHeight - length / 2);
        break;
    case Resizer::Left:
        setPos(-length / 2, parentHeight / 2 - length / 2);
        break;
    case Resizer::TopLeft:
        setPos(-length / 2, -length / 2);
        break;
    case Resizer::TopRight:
        setPos(parentWidth - length / 2, -length / 2);
        break;
    case Resizer::BottomRight:
        setPos(parentWidth - length / 2, parentHeight - length / 2);
        break;
    case Resizer::BottomLeft:
        setPos(-length / 2, parentHeight - length / 2);
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

    if (parentControl()->form()) {
        *dx *= 2.0;
        *dy *= 2.0;
    }
}

void Resizer::mousePressEvent(QGraphicsSceneMouseEvent*)
{
    parentControl()->setResized(true);
}

void Resizer::mouseReleaseEvent(QGraphicsSceneMouseEvent*)
{
    parentControl()->setResized(false);
}

void Resizer::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    if (!isEnabled())
        return;

    if (!parentControl()->resized())
        return;

    qreal dx, dy;
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
