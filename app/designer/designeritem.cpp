#include <designeritem.h>
#include <designerscene.h>
#include <private/qgraphicsitem_p.h>
#include <QGraphicsSceneMouseEvent>

DesignerItem::DesignerItem(DesignerItem* parent) : QGraphicsObject(parent)
  , m_inSetGeometry(false)
  , m_beingDragged(false)
  , m_beingResized(false)
  , m_dragDistanceExceeded(false)
  , m_pen(DesignerScene::pen())
  , m_brush(Qt::white)
{
    setAcceptedMouseButtons(Qt::LeftButton);
}

DesignerScene* DesignerItem::scene() const
{
    return static_cast<DesignerScene*>(QGraphicsItem::scene());
}

DesignerItem* DesignerItem::parentItem() const
{
    return static_cast<DesignerItem*>(QGraphicsItem::parentItem());
}

QList<DesignerItem*> DesignerItem::siblingItems() const
{
    QList<DesignerItem*> siblings;
    if (parentItem()) {
        siblings.append(parentItem()->childItems(false));
        siblings.removeOne(const_cast<DesignerItem*>(this));
    }
    return siblings;
}

QList<DesignerItem*> DesignerItem::childItems(bool recursive) const
{
    QList<DesignerItem*> childs;
    for (QGraphicsItem* item : QGraphicsItem::childItems()) {
        if (item->type() >= Type)
            childs.append(static_cast<DesignerItem*>(item));
    }
    if (recursive) {
        int childCount = childs.size();
        for (int i = 0; i < childCount; ++i)
            childs.append(childs.at(i)->childItems(true));
    }
    return childs;
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
        if (flags() & ItemSendsGeometryChanges) {
            const QVariant newSizeVariant(itemChange(ItemSizeChange, QVariant::fromValue<QSizeF>(size)));
            QSizeF newSize = newSizeVariant.toSizeF();
            if (newSize == m_rect.size())
                return;
            prepareGeometryChange();
            m_rect.setSize(size);
            update();
            itemChange(ItemSizeHasChanged, newSizeVariant);
            emit geometryChanged();
        } else {
            prepareGeometryChange();
            m_rect.setSize(size);
            update();
        }
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
        if (m_rect.topLeft() != rect.topLeft()) {
            prepareGeometryChange();
            m_rect.moveTopLeft(rect.topLeft());
            update();
        }
        setSize(rect.size());
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
    const QRectF& oldGeometry = this->geometry();
    m_inSetGeometry = true;
    setPos(geometry.topLeft());
    m_inSetGeometry = false;
    setSize(geometry.size());
    if ((flags() & ItemSendsGeometryChanges)
            && oldGeometry.topLeft() != pos()
            && oldGeometry.size() == size()) {
        emit geometryChanged();
    }
}

void DesignerItem::setGeometry(qreal x, qreal y, qreal w, qreal h)
{
    setGeometry(QRectF(x, y, w, h));
}

QRectF DesignerItem::boundingRect() const
{
    return m_rect;
}

int DesignerItem::type() const
{
    return Type;
}

bool DesignerItem::beingDragged() const
{
    return m_beingDragged;
}

bool DesignerItem::beingResized() const
{
    return m_beingResized;
}

bool DesignerItem::dragDistanceExceeded() const
{
    return m_dragDistanceExceeded;
}

QVariant DesignerItem::itemChange(int change, const QVariant& value)
{
    if (change == ItemPositionHasChanged) {
        if (!m_inSetGeometry)
            emit geometryChanged();
    }

    if (change < ItemSizeChange)
        return QGraphicsObject::itemChange(GraphicsItemChange(change), value);

    return value;
}

QVariant DesignerItem::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant& value)
{
    return itemChange(int(change), value);
}

void DesignerItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event)
{
    event->ignore();
    emit doubleClicked();
}

void DesignerItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsObject::mousePressEvent(event);
    m_dragStartPoint = event->pos();
    event->accept();
}

void DesignerItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    const QPointF& dragDistance = event->pos() - m_dragStartPoint;

    if (!m_dragDistanceExceeded && dragDistance.manhattanLength() < scene()->startDragDistance())
        return;

    m_dragDistanceExceeded = true;
    m_movableSelectedAncestorItems.clear();

    if (flags() & ItemIsMovable) {
        DesignerItem* ancestor = this;
        DesignerItem* myMovableSelectedAncestorItem = ancestor;
        while (DesignerItem* parent = ancestor->parentItem()) {
            if (parent->isSelected() && (parent->flags() & ItemIsMovable))
                myMovableSelectedAncestorItem = parent;
            ancestor = parent;
        }

        for (DesignerItem* selectedItem : scene()->selectedItems()) {
            if (selectedItem->flags() & ItemIsMovable) {
                if (!QGraphicsItemPrivate::movableAncestorIsSelected(selectedItem))
                    m_movableSelectedAncestorItems.insert(selectedItem);
            }
        }

        m_movableSelectedAncestorItems.remove(myMovableSelectedAncestorItem);

        QList<DesignerItem*> ancestorSiblings = myMovableSelectedAncestorItem->siblingItems();
        for (DesignerItem* movableSelectedAncestorItem : m_movableSelectedAncestorItems.toList()) {
            if (!ancestorSiblings.contains(movableSelectedAncestorItem)) {
                for (DesignerItem* childItem : movableSelectedAncestorItem->childItems())
                    childItem->setSelected(false);
                movableSelectedAncestorItem->setSelected(false);
                m_movableSelectedAncestorItems.remove(movableSelectedAncestorItem);
            }
        }

        m_movableSelectedAncestorItems.insert(myMovableSelectedAncestorItem);

        for (DesignerItem* movableSelectedAncestorItem : m_movableSelectedAncestorItems)
            movableSelectedAncestorItem->setBeingDragged(true);

        scene()->setViewportCursor(Qt::ClosedHandCursor);

        QGraphicsObject::mouseMoveEvent(event);
    }
}

void DesignerItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsObject::mouseReleaseEvent(event);

    if ((flags() & ItemIsMovable) && m_dragDistanceExceeded) {
        scene()->unsetViewportCursor();
        for (DesignerItem* movableSelectedAncestorItem : m_movableSelectedAncestorItems)
            movableSelectedAncestorItem->setBeingDragged(false);
    }

    m_dragDistanceExceeded = false;
}

void DesignerItem::setBeingDragged(bool beingDragged)
{
    if (m_beingDragged != beingDragged) {
        m_beingDragged = beingDragged;
        emit beingDraggedChanged();
    }
}

void DesignerItem::setBeingResized(bool beingResized)
{
    if (m_beingResized != beingResized) {
        m_beingResized = beingResized;
        emit beingResizedChanged();
    }
}
