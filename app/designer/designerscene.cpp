#include <designerscene.h>
#include <suppressor.h>
#include <resizeritem.h>
#include <controlpropertymanager.h>
#include <projectexposingmanager.h>
#include <controlcreationmanager.h>
#include <controlremovingmanager.h>
#include <designersettings.h>
#include <scenesettings.h>
#include <headlineitem.h>
#include <gadgetlayer.h>
#include <paintlayer.h>
#include <windowmanager.h>
#include <centralwidget.h>
#include <designerview.h>
#include <mainwindow.h>
#include <hashfactory.h>

#include <QGraphicsView>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QPointer>
#include <QPen>
#include <QtMath>
#include <QApplication>
#include <QTimer>

DesignerScene::DesignerScene(QObject* parent) : QGraphicsScene(parent)
  , m_dragLayer(new DesignerItem)
  , m_gadgetLayer(new GadgetLayer)
  , m_paintLayer(new PaintLayer)
  , m_currentForm(nullptr)
{
    setItemIndexMethod(QGraphicsScene::NoIndex);

    m_dragLayer->setAcceptedMouseButtons(Qt::NoButton);
    m_dragLayer->setFlag(DesignerItem::ItemHasNoContents);
    m_dragLayer->setZValue(std::numeric_limits<int>::max());
    addItem(m_dragLayer);

    m_gadgetLayer->setAcceptedMouseButtons(Qt::NoButton);
    m_gadgetLayer->setZValue(std::numeric_limits<int>::max());
    connect(this, &DesignerScene::selectionChanged,
            m_gadgetLayer, &GadgetLayer::handleSceneSelectionChange);
    connect(this, &DesignerScene::currentFormChanged,
            m_gadgetLayer, &GadgetLayer::handleSceneCurrentFormChange);
    addItem(m_gadgetLayer);

    m_paintLayer->setAcceptedMouseButtons(Qt::NoButton);
    m_paintLayer->setZValue(std::numeric_limits<int>::max());
    addItem(m_paintLayer);

    connect(this, &DesignerScene::changed, this, [=] {
        setSceneRect(sceneRect() | visibleItemsBoundingRect());
    });

    // This contructor is called from MainWindow ->
    // CentralWidget -> DesignerView -> DesignerScene
    // So WindowManager::mainWindow()->centralWidget()
    // is invalid right here
    QMetaObject::invokeMethod(this, [=] {
        connect(ControlPropertyManager::instance(), &ControlPropertyManager::doubleClicked, this, [=] (Control* i) {
            QTimer::singleShot(100, [=] {
                WindowManager::mainWindow()->centralWidget()->designerView()->onControlDoubleClick(i);
            });
        }, Qt::QueuedConnection);
    }, Qt::QueuedConnection);
    QMetaObject::invokeMethod(this, [=] {
        connect(ControlPropertyManager::instance(), &ControlPropertyManager::geometryChanged,
                paintLayer(), &PaintLayer::updateGeometry);
    }, Qt::QueuedConnection);
    connect(this, &DesignerScene::currentFormChanged, paintLayer(), &PaintLayer::updateGeometry);
    connect(m_gadgetLayer, &GadgetLayer::headlineDoubleClicked, this, [=] (bool isFormHeadline) {
        QTimer::singleShot(100, [=] {
            WindowManager::mainWindow()->centralWidget()->designerView()->
                    onControlDoubleClick(isFormHeadline ? currentForm() : selectedControls().first());
        });
    }, Qt::QueuedConnection);

    connect(ControlCreationManager::instance(), &ControlCreationManager::controlCreated,
            m_gadgetLayer, &GadgetLayer::addResizers);
    connect(ProjectExposingManager::instance(), &ProjectExposingManager::controlExposed,
            m_gadgetLayer, &GadgetLayer::addResizers);
    connect(ControlRemovingManager::instance(), &ControlRemovingManager::controlAboutToBeRemoved,
            m_gadgetLayer, &GadgetLayer::removeResizers);
}

void DesignerScene::addForm(Form* form)
{
    if (m_forms.contains(form))
        return;

    // NOTE: We don't have to call ControlPropertyManager::setParent,
    // since there is no valid parent concept for forms in Designer;
    // fors are directly put into DesignerScene

    m_forms.append(form);
}

void DesignerScene::removeForm(Form* form)
{
    // NOTE: If the given form address is the current form,
    // then if this line runs, QPointer clears the address
    // within m_currentForm, because its object is "delete"d
    removeControl(form);

    m_forms.removeAll(form);
}

void DesignerScene::removeControl(Control* control)
{
    removeItem(control);
    delete control; // Deletes its children too
}

DesignerItem* DesignerScene::highlightItem(const QPointF& pos) const
{
    QList<DesignerItem*> draggedItems = draggedResizedSelectedItems();
    for (int i = draggedItems.size() - 1; i >= 0; --i) {
        DesignerItem* item = draggedItems.at(i);
        if (item->beingResized())
            return nullptr;
        draggedItems.append(item->childItems());
    }

    if (draggedItems.isEmpty())
        return nullptr;

    QList<DesignerItem*> itemsAtPos = items(pos);
    for (int i = itemsAtPos.size() - 1; i >= 0; --i) {
        DesignerItem* item = itemsAtPos.at(i);
        if (draggedItems.contains(item))
            itemsAtPos.removeAt(i);
    }

    // Ordered based on stacking order, higher first
    for (DesignerItem* item : itemsAtPos) {
        if (item->type() >= Control::Type)
            return item;
    }

    return nullptr;
}

void DesignerScene::reparentControl(Control* control, Control* parentControl) const
{
    ControlPropertyManager::Options options = ControlPropertyManager::SaveChanges
            | ControlPropertyManager::CompressedCall;

    if (control->gui())
        options |= ControlPropertyManager::UpdateRenderer;
    // NOTE: Do not move this assignment below setParent,
    // because parent change effects the newPos result
    control->m_geometryCorrection = QRectF();
    control->m_geometryHash = HashFactory::generate();
    const QPointF& newPos = DesignerScene::snapPosition(control->mapToItem(parentControl, QPointF()));
    ControlPropertyManager::setParent(control, parentControl, ControlPropertyManager::SaveChanges
                                      | ControlPropertyManager::UpdateRenderer);
    ControlPropertyManager::setPos(control, newPos, options, control->m_geometryHash);
    // NOTE: We compress setPos because there might be some other
    // compressed setPos'es in the list, We want the setPos that
    // happens after reparent operation to take place at the very last
}

void DesignerScene::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsScene::mouseMoveEvent(event);

    if (m_recentHighlightedItem)
        m_recentHighlightedItem->setBeingHighlighted(false);
    if ((m_recentHighlightedItem = highlightItem(event->scenePos())))
        m_recentHighlightedItem->setBeingHighlighted(true);
}

void DesignerScene::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    const QList<DesignerItem*>& draggedItems = draggedResizedSelectedItems();

    QGraphicsScene::mouseReleaseEvent(event);

    if (m_recentHighlightedItem) {
        if (!draggedItems.isEmpty() && draggedItems.first()->parentItem() != m_recentHighlightedItem) {
            for (DesignerItem* draggedItem : draggedItems) {
                reparentControl(static_cast<Control*>(draggedItem),
                                static_cast<Control*>(m_recentHighlightedItem.data()));
            }
            clearSelection();
            m_recentHighlightedItem->setSelected(true);
        }
        m_recentHighlightedItem->setBeingHighlighted(false);
        m_recentHighlightedItem.clear();
    }
}

Form* DesignerScene::currentForm() const
{
    return m_currentForm.data();
}

qreal DesignerScene::zoomLevel()
{
    return DesignerSettings::sceneSettings()->sceneZoomLevel;
}

int DesignerScene::startDragDistance()
{
    return DesignerSettings::sceneSettings()->dragStartDistance;
}

int DesignerScene::gridSize()
{
    return DesignerSettings::sceneSettings()->gridSize;
}

qreal DesignerScene::lowerZ(DesignerItem* parentItem)
{
    qreal z = 0;
    for (DesignerItem* childItem : parentItem->childItems(false)) {
        if (childItem->zValue() < z)
            z = childItem->zValue();
    }
    return z;
}

qreal DesignerScene::higherZ(DesignerItem* parentItem)
{
    qreal z = 0;
    for (DesignerItem* childItem : parentItem->childItems(false)) {
        if (childItem->zValue() > z)
            z = childItem->zValue();
    }
    return z;
}

QList<DesignerItem*> DesignerScene::selectedItems() const
{
    QList<DesignerItem*> selectedItems;
    for (QGraphicsItem* selectedItem : QGraphicsScene::selectedItems()) {
        if (selectedItem->type() >= DesignerItem::Type)
            selectedItems.append(static_cast<DesignerItem*>(selectedItem));
    }
    return selectedItems;
}

void DesignerScene::unsetCursor()
{
    views().first()->viewport()->unsetCursor();
}

void DesignerScene::setCursor(Qt::CursorShape cursor)
{
    views().first()->viewport()->setCursor(cursor);
}

void DesignerScene::prepareDragLayer(DesignerItem* item)
{
    if (item == 0)
        return;

    m_siblingsBeforeDrag = item->siblingItems();

    if (const DesignerItem* parentItem = item->parentItem()) {
        m_dragLayer->setTransform(QTransform::fromTranslate(parentItem->scenePos().x(),
                                                            parentItem->scenePos().y()));
    }
}

void DesignerScene::shrinkSceneRect()
{
    // 10 margin is a protection against a bug that
    // moves forms unexpectedly when they are selected
    setSceneRect(visibleItemsBoundingRect().adjusted(-10, -10, 10, 10));
}

bool DesignerScene::isLayerItem(DesignerItem* item) const
{
    return item == m_dragLayer || item == m_gadgetLayer || item == m_paintLayer;
}

QPointF DesignerScene::snapPosition(qreal x, qreal y)
{
    return snapPosition(QPointF(x, y));
}

QPointF DesignerScene::snapPosition(const QPointF& pos)
{
    const SceneSettings* settings = DesignerSettings::sceneSettings();
    if (settings->snappingEnabled) {
        const qreal x = qFloor(pos.x() / settings->gridSize) * settings->gridSize;
        const qreal y = qFloor(pos.y() / settings->gridSize) * settings->gridSize;
        return QPointF(x, y);
    }
    return pos;
}

QSizeF DesignerScene::snapSize(qreal x, qreal y, qreal w, qreal h)
{
    return snapSize(QPointF(x, y), QSizeF(w, h));
}

QSizeF DesignerScene::snapSize(const QPointF& pos, const QSizeF& size)
{
    const SceneSettings* settings = DesignerSettings::sceneSettings();
    if (settings->snappingEnabled) {
        const qreal right = pos.x() + size.width();
        const qreal bottom = pos.y() + size.height();
        const QPointF& snapPos = snapPosition(right, bottom);
        return QSizeF(snapPos.x() - pos.x(), snapPos.y() - pos.y());
    }
    return size;
}

// Use on rectangular drawings where the item is transformable but the pen is cosmetic
QRectF DesignerScene::outerRect(const QRectF& rect)
{
    return rect.adjusted(-0.5 / zoomLevel(), -0.5 / zoomLevel(), 0, 0);
}

QList<Control*> DesignerScene::selectedControls() const
{
    QList<Control*> selectedControls;
    for (DesignerItem* item : selectedItems()) {
        if (item->type() >= Control::Type)
            selectedControls.append(static_cast<Control*>(item));
    }
    return selectedControls;
}

QList<DesignerItem*> DesignerScene::draggedResizedSelectedItems() const
{
    QList<DesignerItem*> items(selectedItems());
    for (int i = items.size() - 1; i >= 0; --i) {
        DesignerItem* item = items.at(i);
        if (!item->beingDragged() && !item->beingResized())
            items.removeAt(i);
    }
    return items;
}

QRectF DesignerScene::visibleItemsBoundingRect() const
{
    // Does not take untransformable items into account.
    QRectF boundingRect;
    for (DesignerItem *item : items()) {
        if (isLayerItem(item))
            continue;
        if (item->isVisible())
            boundingRect |= item->sceneBoundingRect();
    }
    return boundingRect.adjusted(-10, -15, 10, 10);
}

bool DesignerScene::showMouseoverOutline()
{
    return DesignerSettings::sceneSettings()->showMouseoverOutline;
}

QVector<QLineF> DesignerScene::guidelines() const
{
    // FIXME: doesn't correctly work for items within a parent
    QVector<QLineF> lines;
    QList<DesignerItem*> items(draggedResizedSelectedItems());

    // May contain it since we can resize the form
    if (items.contains(m_currentForm))
        items.removeOne(m_currentForm);

    if (items.isEmpty())
        return lines;

    const QRectF& geometry = itemsBoundingRect(items);
    const QPointF& center = geometry.center();
    const DesignerItem* parent = items.first()->parentItem();
    Q_ASSERT(parent);

    /* Child center <-> Parent center */
    if (int(center.y()) == int(parent->height() / 2.0))
        lines << QLineF(parent->mapToScene(center),
                        parent->mapToScene(QPointF(parent->width() / 2.0, center.y())));

    if (int(center.x()) == int(parent->width() / 2.0))
        lines << QLineF(parent->mapToScene(center),
                        parent->mapToScene(QPointF(center.x(), parent->height() / 2.0)));

    /* Child left <-> Parent center */
    if (int(geometry.x()) == int(parent->width() / 2.0))
        lines << QLineF(parent->mapToScene(QPointF(geometry.x(), center.y())),
                        parent->mapToScene(QPointF(geometry.x(), parent->height() / 2.0)));

    /* Child left <-> Parent left */
    if (int(geometry.x()) == 0)
        lines << QLineF(parent->mapToScene(QPointF(0, 0)),
                        parent->mapToScene(parent->rect().bottomLeft()));

    /* Child right <-> Parent center */
    if (int(geometry.topRight().x()) == int(parent->width() / 2.0))
        lines << QLineF(parent->mapToScene(QPointF(geometry.topRight().x(), center.y())),
                        parent->mapToScene(QPointF(geometry.topRight().x(), parent->height() / 2.0)));

    /* Child right <-> Parent right */
    if (int(geometry.topRight().x()) == int(parent->width()))
        lines << QLineF(parent->mapToScene(QPointF(parent->width(), 0)),
                        parent->mapToScene(QPointF(parent->width(), parent->height())));

    /* Child top <-> Parent center */
    if (int(geometry.y()) == int(parent->height() / 2.0))
        lines << QLineF(parent->mapToScene(QPointF(center.x(), parent->height() / 2.0)),
                        parent->mapToScene(QPointF(parent->width() / 2.0, parent->height() / 2.0)));

    /* Child top <-> Parent top */
    if (int(geometry.y()) == 0)
        lines << QLineF(parent->mapToScene(QPointF(0, 0)),
                        parent->mapToScene(QPointF(parent->width(), 0)));

    /* Child bottom <-> Parent center */
    if (int(geometry.bottomLeft().y()) == int(parent->height() / 2.0))
        lines << QLineF(parent->mapToScene(QPointF(center.x(), parent->height() / 2.0)),
                        parent->mapToScene(QPointF(parent->width() / 2.0, parent->height() / 2.0)));

    /* Child bottom <-> Parent bottom */
    if (int(geometry.bottomLeft().y()) == int(parent->height()))
        lines << QLineF(parent->mapToScene(QPointF(0, parent->height())),
                        parent->mapToScene(QPointF(parent->width(), parent->height())));

    if (items.size() != 1)
        return lines;

    for (const DesignerItem* siblingItem : m_siblingsBeforeDrag) {
        auto cgeometry = siblingItem->geometry();
        auto ccenter = cgeometry.center();

        /* Item1 center <-> Item2 center */
        if (int(center.x()) == int(ccenter.x()))
            lines << QLineF(parent->mapToScene(QPointF(center.x(), center.y())),
                            parent->mapToScene(QPointF(center.x(), ccenter.y())));

        if (int(center.y()) == int(ccenter.y()))
            lines << QLineF(parent->mapToScene(QPointF(center.x(), center.y())),
                            parent->mapToScene(QPointF(ccenter.x(), center.y())));

        /* Item1 center <-> Item2 left */
        if (int(center.x()) == int(cgeometry.topLeft().x()))
            lines << QLineF(parent->mapToScene(QPointF(center.x(), center.y())),
                            parent->mapToScene(QPointF(center.x(), ccenter.y())));

        /* Item1 center <-> Item2 top */
        if (int(center.y()) == int(cgeometry.topLeft().y()))
            lines << QLineF(parent->mapToScene(QPointF(center.x(), center.y())),
                            parent->mapToScene(QPointF(ccenter.x(), center.y())));

        /* Item1 center <-> Item2 right */
        if (int(center.x()) == int(cgeometry.bottomRight().x()))
            lines << QLineF(parent->mapToScene(QPointF(center.x(), center.y())),
                            parent->mapToScene(QPointF(center.x(), ccenter.y())));

        /* Item1 center <-> Item2 bottom */
        if (int(center.y()) == int(cgeometry.bottomRight().y()))
            lines << QLineF(parent->mapToScene(QPointF(center.x(), center.y())),
                            parent->mapToScene(QPointF(ccenter.x(), center.y())));

        /* Item1 left <-> Item2 left/center/right */
        if (int(geometry.x()) == int(cgeometry.x()) ||
                int(geometry.x()) == int(ccenter.x()) ||
                int(geometry.x()) == int(cgeometry.topRight().x()))
            lines << QLineF(parent->mapToScene(QPointF(geometry.x(), center.y())),
                            parent->mapToScene(QPointF(geometry.x(), ccenter.y())));

        /* Item1 right <-> Item2 left/center/right */
        if (int(geometry.topRight().x()) == int(cgeometry.x()) ||
                int(geometry.topRight().x()) == int(ccenter.x()) ||
                int(geometry.topRight().x()) == int(cgeometry.topRight().x()))
            lines << QLineF(parent->mapToScene(QPointF(geometry.topRight().x(), center.y())),
                            parent->mapToScene(QPointF(geometry.topRight().x(), ccenter.y())));

        /* Item1 top <-> Item2 top/center/bottom */
        if (int(geometry.y()) == int(cgeometry.y()) ||
                int(geometry.y()) == int(ccenter.y()) ||
                int(geometry.y()) == int(cgeometry.bottomLeft().y()))
            lines << QLineF(parent->mapToScene(QPointF(center.x(), geometry.y())),
                            parent->mapToScene(QPointF(ccenter.x(), geometry.y())));

        /* Item1 bottom <-> Item2 top/center/bottom */
        if (int(geometry.bottomLeft().y()) == int(cgeometry.y()) ||
                int(geometry.bottomLeft().y()) == int(ccenter.y()) ||
                int(geometry.bottomLeft().y()) == int(cgeometry.bottomLeft().y()))
            lines << QLineF(parent->mapToScene(QPointF(center.x(), geometry.bottomLeft().y())),
                            parent->mapToScene(QPointF(ccenter.x(), geometry.bottomLeft().y())));
    }
    return lines;
}

DesignerItem* DesignerScene::dragLayer() const
{
    return m_dragLayer;
}

GadgetLayer* DesignerScene::gadgetLayer() const
{
    return m_gadgetLayer;
}

PaintLayer* DesignerScene::paintLayer() const
{
    return m_paintLayer;
}

QColor DesignerScene::outlineColor()
{
    return DesignerSettings::sceneSettings()->outlineColor;
}

QPen DesignerScene::pen(const QColor& color, qreal width, bool cosmetic)
{
    QPen pen(color, width, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin);
    pen.setCosmetic(cosmetic);
    return pen;
}

QBrush DesignerScene::backgroundTexture()
{
    return DesignerSettings::sceneSettings()->toBackgroundBrush();
}

DesignerScene::OutlineMode DesignerScene::outlineMode()
{
    return OutlineMode(DesignerSettings::sceneSettings()->controlOutline);
}

void DesignerScene::drawDashRect(QPainter* painter, const QRectF& rect)
{
    QPen linePen(pen(QColor(255, 255, 255, 150)));
    linePen.setDashPattern({1, 2});
    painter->setPen(linePen);
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(rect);

    linePen.setColor(QColor(0, 0, 0, 150));
    linePen.setDashPattern({2, 1});
    linePen.setDashOffset(2);
    painter->setPen(linePen);
    painter->drawRect(rect);

    linePen.setStyle(Qt::SolidLine);
    painter->setPen(linePen);
    painter->drawPoint(rect.topLeft());
    painter->drawPoint(rect.topRight() - QPointF(0.25, 0.0));
    painter->drawPoint(rect.bottomLeft());
    painter->drawPoint(rect.bottomRight() - QPointF(0.25, 0.0));
}

void DesignerScene::drawDashLine(QPainter* painter, const QLineF& line)
{
    QPen linePen(pen(QColor(0, 0, 0, 150)));
    linePen.setDashPattern({3, 2});
    painter->setPen(linePen);
    painter->setBrush(Qt::NoBrush);
    painter->drawLine(line);

    linePen.setColor(QColor(255, 255, 255, 150));
    linePen.setDashPattern({2, 3});
    linePen.setDashOffset(3);
    painter->setPen(linePen);
    painter->drawLine(line);
}

QRectF DesignerScene::itemsBoundingRect(const QList<DesignerItem*>& items)
{
    QRectF boundingRect;
    for (DesignerItem* item : items)
        boundingRect |= item->sceneBoundingRect();
    return boundingRect;
}

void DesignerScene::discharge()
{
    Q_ASSERT(m_gadgetLayer->m_resizerHash.isEmpty());
    clearSelection();
    m_forms.clear();
    m_currentForm.clear();
}

void DesignerScene::setCurrentForm(Form* currentForm)
{
    if (m_currentForm == currentForm)
        return;

    if (!m_forms.contains(currentForm))
        return;

    /*
        NOTE: InspectorPane dependency: We prevent "selectionChanged" signal being emitted here
              Otherwise selectionChanged signal getting emitted before currentFormChanged signal,
              hence InspectorPane clears the selection before saving selection state of a form in
              currentFormChanged signal.
    */

    Form* previous = m_currentForm;
    m_currentForm = currentForm;

    emit currentFormChanged(currentForm);

    if (previous)
        removeItem(previous);

    if (m_currentForm) {
        addItem(m_currentForm);
        m_currentForm->setPos(0, 0);
        shrinkSceneRect();
    }
}

QList<Form*> DesignerScene::forms() const
{
    return m_forms;
}
