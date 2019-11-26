#include <designeritem.h>
#include <designerscene.h>
#include <designerview.h>
#include <designersettings.h>
#include <scenesettings.h>
#include <private/qgraphicsitem_p.h>
#include <QGraphicsSceneMouseEvent>

DesignerItem::DesignerItem(DesignerItem* parent) : QGraphicsObject(parent)
  , m_raised(false)
  , m_resizable(false)
  , m_beingDragged(false)
  , m_beingResized(false)
  , m_beingHighlighted(false)
  , m_dragAccepted(false)
  , m_inSetGeometry(false)
  , m_mousePressCursorShape(Qt::CustomCursor)
{
}

int DesignerItem::type() const
{
    return Type;
}

bool DesignerItem::dragAccepted() const
{
    return m_dragAccepted;
}

bool DesignerItem::raised() const
{
    return m_raised;
}

void DesignerItem::setRaised(bool raised)
{
    if (m_raised != raised) {
        m_raised = raised;
        if (m_raised) {
            m_parentItemBeforeRaise = parentItem();
            if (scene())
                setParentItem(scene()->dragLayer());
        } else {
            setParentItem(m_parentItemBeforeRaise);
        }
    }
}

bool DesignerItem::resizable() const
{
    return m_resizable;
}

void DesignerItem::setResizable(bool resizable)
{
    if (m_resizable != resizable) {
        m_resizable = resizable;
        emit resizableChanged();
    }
}

bool DesignerItem::beingDragged() const
{
    return m_beingDragged;
}

void DesignerItem::setBeingDragged(bool beingDragged)
{
    if (m_beingDragged != beingDragged) {
        m_beingDragged = beingDragged;
        emit beingDraggedChanged();
    }
}

bool DesignerItem::beingResized() const
{
    return m_beingResized;
}

void DesignerItem::setBeingResized(bool beingResized)
{
    if (m_beingResized != beingResized) {
        m_beingResized = beingResized;
        emit beingResizedChanged();
    }
}

bool DesignerItem::beingHighlighted() const
{
    return m_beingHighlighted;
}

void DesignerItem::setBeingHighlighted(bool beingHighlighted)
{
    if (m_beingHighlighted != beingHighlighted) {
        m_beingHighlighted = beingHighlighted;
        update();
        emit beingHighlightedChanged();
    }
}

qreal DesignerItem::devicePixelRatio() const
{
    if (scene())
        return scene()->devicePixelRatio();
    return QApplication::primaryScreen()->devicePixelRatio();
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

void DesignerItem::setSize(qreal width, qreal height)
{
    setSize(QSizeF(width, height));
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

QRectF DesignerItem::boundingRect() const
{
    return m_rect;
}

QRectF DesignerItem::rect() const
{
    return m_rect;
}

void DesignerItem::setRect(qreal x, qreal y, qreal w, qreal h)
{
    setRect(QRectF(x, y, w, h));
}

void DesignerItem::setRect(const QPointF& topLeft, const QSizeF& size)
{
    setRect(QRectF(topLeft, size));
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

QRectF DesignerItem::geometry() const
{
    return QRectF(pos(), size());
}

void DesignerItem::setGeometry(qreal x, qreal y, qreal w, qreal h)
{
    setGeometry(QRectF(x, y, w, h));
}

void DesignerItem::setGeometry(const QPointF& pos, const QSizeF& size)
{
    setGeometry(QRectF(pos, size));
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

Qt::CursorShape DesignerItem::mousePressCursorShape() const
{
    return m_mousePressCursorShape;
}

void DesignerItem::setMousePressCursorShape(const Qt::CursorShape& mousePressCursorShape)
{
    m_mousePressCursorShape = mousePressCursorShape;
}

DesignerScene* DesignerItem::scene() const
{
    return static_cast<DesignerScene*>(QGraphicsObject::scene());
}

DesignerItem* DesignerItem::parentItem() const
{
    return static_cast<DesignerItem*>(QGraphicsObject::parentItem());
}

DesignerItem* DesignerItem::topLevelItem() const
{
    DesignerItem* parent = const_cast<DesignerItem*>(this);
    while (DesignerItem* grandPa = parent->parentItem())
        parent = grandPa;
    return parent;
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

bool DesignerItem::event(QEvent* event)
{
    switch (event->type()) {
    case QEvent::UngrabMouse:
        mouseUngrabEvent(event);
        break;
    default:
        break;
    }
    return QGraphicsObject::event(event);
}

void DesignerItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsItem::mouseDoubleClickEvent(event);
    emit doubleClicked(event->buttons());
}

void DesignerItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    if (scene() && m_mousePressCursorShape != Qt::CustomCursor)
        scene()->setCursor(m_mousePressCursorShape);
    QGraphicsObject::mousePressEvent(event);
}

void DesignerItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    // Damn event->button() always returns
    // Qt::NoButton for mouse move events
    const QPointF& dragDistance = event->pos() - event->buttonDownPos(scene()->view()->mousePressButton());

    if (!m_dragAccepted && dragDistance.manhattanLength()
            < DesignerSettings::sceneSettings()->dragStartDistance) {
        return;
    }

    m_dragAccepted = true;

    if (scene() && (event->buttons() & Qt::LeftButton) && (flags() & ItemIsMovable)
            && m_movableSelectedAncestorItems.isEmpty()) {
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

        const QList<DesignerItem*>& ancestorSiblings = myMovableSelectedAncestorItem->siblingItems();
        for (DesignerItem* movableSelectedAncestorItem : m_movableSelectedAncestorItems.toList()) {
            if (!ancestorSiblings.contains(movableSelectedAncestorItem)) {
                for (DesignerItem* childItem : movableSelectedAncestorItem->childItems())
                    childItem->setSelected(false);
                movableSelectedAncestorItem->setSelected(false);
                m_movableSelectedAncestorItems.remove(movableSelectedAncestorItem);
            }
        }

        m_movableSelectedAncestorItems.insert(myMovableSelectedAncestorItem);

        scene()->prepareDragLayer(myMovableSelectedAncestorItem);

        for (DesignerItem* movableSelectedAncestorItem : m_movableSelectedAncestorItems) {
            movableSelectedAncestorItem->setBeingDragged(true);
            movableSelectedAncestorItem->setRaised(true);
        }
    }

    QGraphicsObject::mouseMoveEvent(event);
}

void DesignerItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(painter)
    Q_UNUSED(option)
    Q_UNUSED(widget)
}

void DesignerItem::mouseUngrabEvent(QEvent* event)
{
    Q_UNUSED(event)
    if (scene() && m_mousePressCursorShape != Qt::CustomCursor)
        scene()->unsetCursor();
    if ((flags() & ItemIsMovable) && m_dragAccepted) {
        for (DesignerItem* movableSelectedAncestorItem : m_movableSelectedAncestorItems) {
            movableSelectedAncestorItem->setRaised(false);
            movableSelectedAncestorItem->setBeingDragged(false);
        }
        m_movableSelectedAncestorItems.clear();
    }
    m_dragAccepted = false;
}

QVariant DesignerItem::itemChange(int change, const QVariant& value)
{
    switch (change) {
    case ItemPositionHasChanged:
        if (!m_inSetGeometry)
            emit geometryChanged();
        break;
    case ItemTransformOriginPointHasChanged:
        emit transformOriginPointChanged();
        break;
    default:
        break;
    }

    if (change < ItemSizeChange)
        return QGraphicsObject::itemChange(GraphicsItemChange(change), value);

    return value;
}

QVariant DesignerItem::itemChange(DesignerItem::GraphicsItemChange change, const QVariant& value)
{
    return itemChange(int(change), value);
}