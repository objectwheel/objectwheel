#include <resizer.h>
#include <fit.h>
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QCursor>
#include <control.h>

#define SIZE (fit::fx(6.0))
#define COLOR (Qt::white)
#define OUTLINE_COLOR ("#202427")
#define MAX_Z_VALUE (99999)

bool Resizer::_resizing = false;

Resizer::Resizer(QGraphicsWidget* parent)
    : QGraphicsWidget(parent)
    , _placement(Top)
    , _disabled(false)
{
    setVisible(false);
    setAcceptedMouseButtons(Qt::LeftButton);
    setAcceptHoverEvents(true);
}

bool Resizer::disabled() const
{
    return _disabled;
}

void Resizer::setDisabled(bool disabled)
{
    _disabled = disabled;
}

bool Resizer::resizing()
{
    return _resizing;
}

Resizer::Placement Resizer::placement() const
{
    return _placement;
}

void Resizer::setPlacement(Placement placement)
{
    _placement = placement;

    switch (_placement) {
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

void Resizer::correct()
{
    auto parent = (QGraphicsWidget*)parentItem();

    if (!parent)
        return;

    switch (placement()) {
        case Resizer::Top:
            setPos(parent->size().width() / 2.0 - SIZE / 2.0,
              - SIZE / 2.0 + 0.5);
            break;

        case Resizer::Right:
            setPos(parent->size().width() - SIZE / 2.0 - 0.5,
              parent->size().height() / 2.0 - SIZE / 2.0);
            break;

        case Resizer::Bottom:
            setPos(parent->size().width() / 2.0 - SIZE / 2.0,
              parent->size().height() - SIZE / 2.0 - 0.5);
            break;

        case Resizer::Left:
            setPos(- SIZE / 2.0 + 0.5,
              parent->size().height() / 2.0 - SIZE / 2.0);
            break;

        case Resizer::TopLeft:
            setPos(- SIZE / 2.0 + 0.5, - SIZE / 2.0 + 0.5);
            break;

        case Resizer::TopRight:
            setPos(parent->size().width() - SIZE / 2.0 - 0.5,
              - SIZE / 2.0 + 0.5);
            break;

        case Resizer::BottomRight:
            setPos(parent->size().width() - SIZE / 2.0 - 0.5,
              parent->size().height() - SIZE / 2.0 - 0.5);
            break;

        case Resizer::BottomLeft:
            setPos( - SIZE / 2.0 + 0.5,
              parent->size().height() - SIZE / 2.0 - 0.5);
            break;
    }
    setZValue(MAX_Z_VALUE);
}

void Resizer::paint(QPainter* painter, const QSogi*, QWidget*)
{
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setPen(OUTLINE_COLOR);
    painter->setBrush(COLOR);
    painter->drawRoundedRect(boundingRect().
      adjusted(0.5, 0.5, -0.5, -0.5),
      SIZE / 4.0, SIZE / 4.0);
}

void Resizer::mousePressEvent(QGsme* event)
{
    QGraphicsItem::mousePressEvent(event);
    event->accept();
    _resizing = true;
}

void Resizer::mouseMoveEvent(QGsme* event)
{
    QGraphicsItem::mouseMoveEvent(event);
    auto parent = static_cast<Control*>(parentItem());

    if (!parent || _disabled || !_resizing)
        return;

    qreal diff_x, diff_y;
    auto startSize = parent->size();
    switch (_placement) {
        case Top:
            diff_y = event->lastPos().y() - event->pos().y();
            if (parent->form()) diff_y *= 2.0;
            parent->setGeometry(parent->geometry().
              adjusted(0, -diff_y, 0, 0));
            break;

        case Right:
            diff_x = event->pos().x() - event->lastPos().x();
            if (parent->form()) diff_x *= 2.0;
            parent->setGeometry(parent->geometry().
              adjusted(0, 0, diff_x, 0));
            break;

        case Bottom:
            diff_y = event->pos().y() - event->lastPos().y();
            if (parent->form()) diff_y *= 2.0;
            parent->setGeometry(parent->geometry().
              adjusted(0, 0, 0, diff_y));
            break;

        case Left:
            diff_x = event->lastPos().x() - event->pos().x();
            if (parent->form()) diff_x *= 2.0;
            parent->setGeometry(parent->geometry().
              adjusted(-diff_x, 0, 0, 0));
            break;

        case TopLeft:
            diff_x = event->lastPos().x() - event->pos().x();
            diff_y = event->lastPos().y() - event->pos().y();
            if (parent->form()) diff_x *= 2.0;
            if (parent->form()) diff_y *= 2.0;
            parent->setGeometry(parent->geometry().
              adjusted(-diff_x, -diff_y, 0, 0));
            break;

        case TopRight:
            diff_x = event->pos().x() - event->lastPos().x();
            diff_y = event->lastPos().y() - event->pos().y();
            if (parent->form()) diff_x *= 2.0;
            if (parent->form()) diff_y *= 2.0;
            parent->setGeometry(parent->geometry().
              adjusted(0, -diff_y, diff_x, 0));
            break;

        case BottomRight:
            diff_x = event->pos().x() - event->lastPos().x();
            diff_y = event->pos().y() - event->lastPos().y();
            if (parent->form()) diff_x *= 2.0;
            if (parent->form()) diff_y *= 2.0;
            parent->setGeometry(parent->geometry().
              adjusted(0, 0, diff_x, diff_y));
            break;

        case BottomLeft:
            diff_x = event->lastPos().x() - event->pos().x();
            diff_y = event->pos().y() - event->lastPos().y();
            if (parent->form()) diff_x *= 2.0;
            if (parent->form()) diff_y *= 2.0;
            parent->setGeometry(parent->geometry().
              adjusted(-diff_x, 0, 0, diff_y));
            break;
    }

    if (parent->size().width() < SIZE ||
        parent->size().height() < SIZE) {
        parent->resize(startSize);
    }
}

void Resizer::mouseReleaseEvent(QGsme* event)
{
    QGraphicsItem::mouseReleaseEvent(event);
    _resizing = false;
}

QRectF Resizer::boundingRect() const
{
    return QRectF(0, 0, SIZE, SIZE);
}
