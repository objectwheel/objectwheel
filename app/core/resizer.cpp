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
        setPos(parentWidth / 2 - length / 2, - length / 2 + 0.5);
        break;
    case Resizer::Right:
        setPos(parentWidth - length / 2 - 0.5, parentHeight / 2 - length / 2);
        break;
    case Resizer::Bottom:
        setPos(parentWidth / 2 - length / 2, parentHeight - length / 2 - 0.5);
        break;
    case Resizer::Left:
        setPos(-length / 2 + 0.5, parentHeight / 2 - length / 2);
        break;
    case Resizer::TopLeft:
        setPos(-length / 2 + 0.5, - length / 2 + 0.5);
        break;
    case Resizer::TopRight:
        setPos(parentWidth - length / 2 - 0.5, - length / 2 + 0.5);
        break;
    case Resizer::BottomRight:
        setPos(parentWidth - length / 2 - 0.5, parentHeight - length / 2 - 0.5);
        break;
    case Resizer::BottomLeft:
        setPos(-length / 2 + 0.5, parentHeight - length / 2 - 0.5);
        break;
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
    Control* control = parentControl();
    if (!isEnabled() || !control->resized())
        return;

    qreal dx, dy;
    const qreal length = rect().width();
    const QRectF parentGeo = control->geometry();
    const qreal parentWidth = parentGeo.width();
    const qreal parentHeight = parentGeo.height();
    auto adj = [=] (qreal x1, qreal y1, qreal x2, qreal y2) {
        return parentGeo.adjusted(x1, y1, x2, y2);
    };

    ControlPropertyManager::Options opt = ControlPropertyManager::SaveChanges
            | ControlPropertyManager::UpdatePreviewer
            | ControlPropertyManager::CompressedCall;

    switch (m_placement) {
    case Top:
        dy = event->lastPos().y() - event->pos().y();
        if (dy == 0)
            break;
        if (control->form()) {
            dy *= 2.0;
            ControlPropertyManager::setHeight(control, parentHeight + dy, opt);
        } else {
            ControlPropertyManager::setGeometry(control, adj(0, -dy, 0, 0), opt);
        }
        break;

    case Right:
        dx = event->pos().x() - event->lastPos().x();
        if (dx == 0)
            break;
        if (control->form()) {
            dx *= 2.0;
            ControlPropertyManager::setWidth(control, parentWidth + dx, opt);
        } else {
            ControlPropertyManager::setGeometry(control, adj(0, 0, dx, 0), opt);
        }
        break;

    case Bottom:
        dy = event->pos().y() - event->lastPos().y();
        if (dy == 0)
            break;
        if (control->form()) {
            dy *= 2.0;
            ControlPropertyManager::setHeight(control, parentHeight + dy, opt);
        } else {
            ControlPropertyManager::setGeometry(control, adj(0, 0, 0, dy), opt);
        }
        break;

    case Left:
        dx = event->lastPos().x() - event->pos().x();
        if (dx == 0)
            break;
        if (control->form()) {
            dx *= 2.0;
            ControlPropertyManager::setWidth(control, parentWidth + dx, opt);
        } else {
            ControlPropertyManager::setGeometry(control, adj(-dx, 0, 0, 0), opt);
        }
        break;

    case TopLeft:
        dx = event->lastPos().x() - event->pos().x();
        dy = event->lastPos().y() - event->pos().y();
        if (dx == 0 && dy == 0)
            break;
        if (control->form()) {
            dx *= 2.0;
            dy *= 2.0;
            ControlPropertyManager::setWidth(control, parentWidth + dx, opt);
            ControlPropertyManager::setHeight(control, parentHeight + dy, opt);
        } else {
            ControlPropertyManager::setGeometry(control, adj(-dx, -dy, 0, 0), opt);
        }
        break;

    case TopRight:
        dx = event->pos().x() - event->lastPos().x();
        dy = event->lastPos().y() - event->pos().y();
        if (dx == 0 && dy == 0)
            break;
        if (control->form()) {
            dx *= 2.0;
            dy *= 2.0;
            ControlPropertyManager::setWidth(control, parentWidth + dx, opt);
            ControlPropertyManager::setHeight(control, parentHeight + dy, opt);
        } else {
            ControlPropertyManager::setGeometry(control, adj(0, -dy, dx, 0), opt);
        }
        break;

    case BottomRight:
        dx = event->pos().x() - event->lastPos().x();
        dy = event->pos().y() - event->lastPos().y();
        if (dx == 0 && dy == 0)
            break;
        if (control->form()) {
            dx *= 2.0;
            dy *= 2.0;
            ControlPropertyManager::setWidth(control, parentWidth + dx, opt);
            ControlPropertyManager::setHeight(control, parentHeight + dy, opt);
        } else {
            ControlPropertyManager::setGeometry(control, adj(0, 0, dx, dy), opt);
        }
        break;

    case BottomLeft:
        dx = event->lastPos().x() - event->pos().x();
        dy = event->pos().y() - event->lastPos().y();
        if (dx == 0 && dy == 0)
            break;
        if (control->form()) {
            dx *= 2.0;
            dy *= 2.0;
            ControlPropertyManager::setWidth(control, parentWidth + dx, opt);
            ControlPropertyManager::setHeight(control, parentHeight + dy, opt);
        } else {
            ControlPropertyManager::setGeometry(control, adj(-dx, 0, 0, dy), opt);
        }
        break;
    }

    if (parentWidth < length || parentHeight < length)
        ControlPropertyManager::setSize(control, parentGeo.size(), opt);
}
