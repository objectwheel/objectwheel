#include <designeritem.h>
#include <designerscene.h>
#include <private/qgraphicsitem_p.h>
#include <QGraphicsSceneMouseEvent>

DesignerItem::DesignerItem(DesignerItem* parent) : QGraphicsObject(parent)
  , m_inSetGeometry(false)
  , m_beingDragged(false)
  , m_beingResized(false)
  , m_dragAccepted(false)
  , m_raised(false)
{
    setAcceptedMouseButtons(Qt::LeftButton);
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

DesignerScene* DesignerItem::scene() const
{
    return static_cast<DesignerScene*>(QGraphicsObject::scene());
}

DesignerItem* DesignerItem::parentItem() const
{
    return static_cast<DesignerItem*>(QGraphicsObject::parentItem());
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
    for (QGraphicsItem* item : QGraphicsObject::childItems()) {
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
            m_rect.setSize(newSize);
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

bool DesignerItem::dragAccepted() const
{
    return m_dragAccepted;
}

QPointF DesignerItem::mousePressPoint() const
{
    return m_mousePressPoint;
}

bool DesignerItem::event(QEvent* event)
{
    switch (event->type()) {
    case QEvent::UngrabMouse:
        ungrabMouseEvent(event);
        break;
    default:
        break;
    }
    return QGraphicsObject::event(event);
}

void DesignerItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event)
{
    event->ignore();
    emit doubleClicked();
}

void DesignerItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    m_mousePressPoint = event->pos();
    QGraphicsObject::mousePressEvent(event);
}

void DesignerItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    const QPointF& dragDistance = event->pos() - m_mousePressPoint;

    if (!m_dragAccepted && dragDistance.manhattanLength() < scene()->startDragDistance())
        return;

    m_dragAccepted = true;

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

        scene()->prepareDragLayer(this);
        scene()->setCursor(Qt::ClosedHandCursor);

        for (DesignerItem* movableSelectedAncestorItem : m_movableSelectedAncestorItems) {
            movableSelectedAncestorItem->setBeingDragged(true);
            movableSelectedAncestorItem->setRaised(true);
        }

        QGraphicsObject::mouseMoveEvent(event);
    }
}

void DesignerItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsObject::mouseReleaseEvent(event);

    if ((flags() & ItemIsMovable) && m_dragAccepted) {
        scene()->unsetCursor();
        for (DesignerItem* movableSelectedAncestorItem : m_movableSelectedAncestorItems) {
            movableSelectedAncestorItem->setBeingDragged(false);
            movableSelectedAncestorItem->setRaised(false);
        }
        m_movableSelectedAncestorItems.clear();
    }
    m_dragAccepted = false;
}

void DesignerItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(painter)
    Q_UNUSED(option)
    Q_UNUSED(widget)
}

void DesignerItem::ungrabMouseEvent(QEvent*)
{
    QGraphicsSceneMouseEvent release;
    release.setButtons(acceptedMouseButtons());
    mouseReleaseEvent(&release);
}

QVariant DesignerItem::itemChange(int change, const QVariant& value)
{
    switch (change) {
    case ItemPositionHasChanged:
        if (!m_inSetGeometry)
            emit geometryChanged();
        break;

    default:
        break;
    }

    if (change < ItemSizeChange)
        return QGraphicsObject::itemChange(GraphicsItemChange(change), value);

    return value;
}

void DesignerItem::setRaised(bool raised)
{
    Q_ASSERT(parentItem() && scene());
    if (m_raised != raised) {
        m_raised = raised;
        if (m_raised) {
            m_parentItemBeforeRaise = parentItem();
            setParentItem(scene()->dragLayerItem());
        } else {
            setParentItem(m_parentItemBeforeRaise);
        }
    }
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

QVariant DesignerItem::itemChange(DesignerItem::GraphicsItemChange change, const QVariant& value)
{
    return itemChange(int(change), value);
}
