#include <resizer.h>
#include <control.h>
#include <controlpropertymanager.h>

#include <QPainter>
#include <QCursor>
#include <QGraphicsSceneMouseEvent>

#define MAX_Z_VALUE   99999
#define COLOR         Qt::white
#define SIZE          6.0
#define OUTLINE_COLOR Qt::black

bool Resizer::m_resizing = false;

Resizer::Resizer(QGraphicsWidget* parent, Placement placement) : QGraphicsWidget(parent)
  , m_placement(placement)
  , m_disabled(true)
{
    setVisible(false);
    setAcceptedMouseButtons(Qt::LeftButton);
    setAcceptHoverEvents(true);

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

bool Resizer::disabled() const
{
    return m_disabled;
}

void Resizer::setDisabled(bool disabled)
{
    m_disabled = disabled;
}

bool Resizer::resizing()
{
    return m_resizing;
}

void Resizer::correct()
{
    switch (m_placement) {
    case Resizer::Top:
        setPos(parentWidget()->size().width() / 2.0 - SIZE / 2.0, - SIZE / 2.0 + 0.5);
        break;

    case Resizer::Right:
        setPos(parentWidget()->size().width() - SIZE / 2.0 - 0.5, parentWidget()->size().height() / 2.0 - SIZE / 2.0);
        break;

    case Resizer::Bottom:
        setPos(parentWidget()->size().width() / 2.0 - SIZE / 2.0, parentWidget()->size().height() - SIZE / 2.0 - 0.5);
        break;

    case Resizer::Left:
        setPos(- SIZE / 2.0 + 0.5, parentWidget()->size().height() / 2.0 - SIZE / 2.0);
        break;

    case Resizer::TopLeft:
        setPos(- SIZE / 2.0 + 0.5, - SIZE / 2.0 + 0.5);
        break;

    case Resizer::TopRight:
        setPos(parentWidget()->size().width() - SIZE / 2.0 - 0.5, - SIZE / 2.0 + 0.5);
        break;

    case Resizer::BottomRight:
        setPos(parentWidget()->size().width() - SIZE / 2.0 - 0.5, parentWidget()->size().height() - SIZE / 2.0 - 0.5);
        break;

    case Resizer::BottomLeft:
        setPos( - SIZE / 2.0 + 0.5, parentWidget()->size().height() - SIZE / 2.0 - 0.5);
        break;
    }

    setZValue(MAX_Z_VALUE);
}

void Resizer::paint(QPainter* painter, const QSogi*, QWidget*)
{
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setPen(OUTLINE_COLOR);
    painter->setBrush(COLOR);
    painter->drawRoundedRect(boundingRect().adjusted(0.5, 0.5, -0.5, -0.5), SIZE / 4.0, SIZE / 4.0);
}

void Resizer::mousePressEvent(QGsme* event)
{
    QGraphicsItem::mousePressEvent(event);
    event->accept();
    m_resizing = true;
    static_cast<Control*>(parentWidget())->setResizing(true);
}

void Resizer::mouseMoveEvent(QGsme* event)
{
    QGraphicsItem::mouseMoveEvent(event);

    auto parent = static_cast<Control*>(parentWidget());

    if (!parent || m_disabled || !m_resizing)
        return;

    qreal diff_x, diff_y;
    auto startSize = parent->size();

    ControlPropertyManager::Options options = ControlPropertyManager::SaveChanges
            | ControlPropertyManager::UpdatePreviewer
            | ControlPropertyManager::CompressedCall;

    switch (m_placement) {
    case Top:
        diff_y = event->lastPos().y() - event->pos().y();
        if (diff_y == 0)
            break;
        if (parent->form()) {
            diff_y *= 2.0;
            ControlPropertyManager::setHeight(parent, parent->size().height() + diff_y, options);
        } else {
            ControlPropertyManager::setGeometry(parent,
                                                parent->geometry().adjusted(0, -diff_y, 0, 0), options);
        }
        break;

    case Right:
        diff_x = event->pos().x() - event->lastPos().x();
        if (diff_x == 0)
            break;
        if (parent->form()) {
            diff_x *= 2.0;
            ControlPropertyManager::setWidth(parent, parent->size().width() + diff_x, options);
        } else {
            ControlPropertyManager::setGeometry(parent,
                                                parent->geometry().adjusted(0, 0, diff_x, 0), options);
        }
        break;

    case Bottom:
        diff_y = event->pos().y() - event->lastPos().y();
        if (diff_y == 0)
            break;
        if (parent->form()) {
            diff_y *= 2.0;
            ControlPropertyManager::setHeight(parent, parent->size().height() + diff_y, options);
        } else {
            ControlPropertyManager::setGeometry(parent,
                                                parent->geometry().adjusted(0, 0, 0, diff_y), options);
        }
        break;

    case Left:
        diff_x = event->lastPos().x() - event->pos().x();
        if (diff_x == 0)
            break;
        if (parent->form()) {
            diff_x *= 2.0;
            ControlPropertyManager::setWidth(parent, parent->size().width() + diff_x, options);
        } else {
            ControlPropertyManager::setGeometry(parent,
                                                parent->geometry().adjusted(-diff_x, 0, 0, 0), options);
        }
        break;

    case TopLeft:
        diff_x = event->lastPos().x() - event->pos().x();
        diff_y = event->lastPos().y() - event->pos().y();
        if (diff_x == 0 && diff_y == 0)
            break;
        if (parent->form()) {
            diff_x *= 2.0;
            diff_y *= 2.0;
            ControlPropertyManager::setWidth(parent, parent->size().width() + diff_x, options);
            ControlPropertyManager::setHeight(parent, parent->size().height() + diff_y, options);
        } else {
            ControlPropertyManager::setGeometry(parent,
                                                parent->geometry().adjusted(-diff_x, -diff_y, 0, 0), options);
        }
        break;

    case TopRight:
        diff_x = event->pos().x() - event->lastPos().x();
        diff_y = event->lastPos().y() - event->pos().y();
        if (diff_x == 0 && diff_y == 0)
            break;
        if (parent->form()) {
            diff_x *= 2.0;
            diff_y *= 2.0;
            ControlPropertyManager::setWidth(parent, parent->size().width() + diff_x, options);
            ControlPropertyManager::setHeight(parent, parent->size().height() + diff_y, options);
        } else {
            ControlPropertyManager::setGeometry(parent,
                                                parent->geometry().adjusted(0, -diff_y, diff_x, 0), options);
        }
        break;

    case BottomRight:
        diff_x = event->pos().x() - event->lastPos().x();
        diff_y = event->pos().y() - event->lastPos().y();
        if (diff_x == 0 && diff_y == 0)
            break;
        if (parent->form()) {
            diff_x *= 2.0;
            diff_y *= 2.0;
            ControlPropertyManager::setWidth(parent, parent->size().width() + diff_x, options);
            ControlPropertyManager::setHeight(parent, parent->size().height() + diff_y, options);
        } else {
            ControlPropertyManager::setGeometry(parent,
                                                parent->geometry().adjusted(0, 0, diff_x, diff_y), options);
        }
        break;

    case BottomLeft:
        diff_x = event->lastPos().x() - event->pos().x();
        diff_y = event->pos().y() - event->lastPos().y();
        if (diff_x == 0 && diff_y == 0)
            break;
        if (parent->form()) {
            diff_x *= 2.0;
            diff_y *= 2.0;
            ControlPropertyManager::setWidth(parent, parent->size().width() + diff_x, options);
            ControlPropertyManager::setHeight(parent, parent->size().height() + diff_y, options);
        } else {
            ControlPropertyManager::setGeometry(parent,
                                                parent->geometry().adjusted(-diff_x, 0, 0, diff_y), options);
        }
        break;
    }

    if (parent->size().width() < SIZE || parent->size().height() < SIZE)
        ControlPropertyManager::setSize(parent, startSize, options);
}

void Resizer::mouseReleaseEvent(QGsme* event)
{
    QGraphicsItem::mouseReleaseEvent(event);
    m_resizing = false;
    static_cast<Control*>(parentWidget())->setResizing(false);
}

QRectF Resizer::boundingRect() const
{
    return QRectF(0, 0, SIZE, SIZE);
}
