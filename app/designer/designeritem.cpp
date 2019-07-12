#include <designeritem.h>
#include <designerscene.h>
#include <QGraphicsSceneMouseEvent>

DesignerItem::DesignerItem(Control* parent) : QGraphicsObject(parent)
  , m_pen(DesignerScene::pen())
  , m_brush(Qt::white)
  , m_dragStarted(false)
{
    setVisible(false);
    setFlag(ItemIgnoresTransformations);
    setAcceptedMouseButtons(Qt::LeftButton);
    setZValue(std::numeric_limits<int>::max());
}

DesignerScene* DesignerItem::scene() const
{
    return static_cast<DesignerScene*>(QGraphicsItem::scene());
}

Control* DesignerItem::parentControl() const
{
    return static_cast<Control*>(parentItem());
}

QPen DesignerItem::pen() const
{
    return m_pen;
}

void DesignerItem::setPen(const QPen& pen)
{
    m_pen = pen;
    update();
}

QBrush DesignerItem::brush() const
{
    return m_brush;
}

void DesignerItem::setBrush(const QBrush& brush)
{
    m_brush = brush;
    update();
}

QFont DesignerItem::font() const
{
    return m_font;
}

void DesignerItem::setFont(const QFont& font)
{
    m_font = font;
    update();
}

qreal DesignerItem::width() const
{
    return size().width();
}

qreal DesignerItem::height() const
{
    return size().height();
}

QSizeF DesignerItem::size() const
{
    return m_rect.size();
}

void DesignerItem::setSize(const QSizeF& size)
{
    if (m_rect.size() != size) {
        prepareGeometryChange();
        m_rect.setSize(size);
        update();
    }
}

void DesignerItem::setSize(qreal width, qreal height)
{
    setSize(QSizeF(width, height));
}

QRectF DesignerItem::rect() const
{
    return m_rect;
}

void DesignerItem::setRect(const QRectF& rect)
{
    if (m_rect != rect) {
        prepareGeometryChange();
        m_rect = rect;
        update();
    }
}

void DesignerItem::setRect(qreal x, qreal y, qreal w, qreal h)
{
    setRect(QRectF(x, y, w, h));
}

QRectF DesignerItem::geometry() const
{
    return QRectF(pos(), size());
}

void DesignerItem::setGeometry(const QRectF& geometry)
{
    setPos(geometry.topLeft());
    setSize(geometry.size());
}

void DesignerItem::setGeometry(qreal x, qreal y, qreal w, qreal h)
{
    setGeometry(QRectF(x, y, w, h));
}

QRectF DesignerItem::boundingRect() const
{
    return m_rect;
}

bool DesignerItem::dragStarted() const
{
    return m_dragStarted;
}

QPointF DesignerItem::snapPosition() const
{
    return m_snapPosition;
}

void DesignerItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event)
{
    event->ignore();
    emit doubleClicked();
}

void DesignerItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    Q_UNUSED(event)
    m_dragStartPoint = event->pos();
}

void DesignerItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    const QPointF& diff = event->pos() - m_dragStartPoint;
    if (m_dragStarted || diff.manhattanLength() >= scene()->startDragDistance()) {
        m_snapPosition = scene()->snapPosition(parentControl()->mapToParent(mapToParent(diff)));
        if (!m_dragStarted) {
            m_dragStarted = true;
            emit dragStartedChanged();
        }
    }
}

void DesignerItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    Q_UNUSED(event)
    if (m_dragStarted) {
        m_dragStarted = false;
        emit dragStartedChanged();
    }
}
