#include <designerscene.h>
#include <designerview.h>
#include <designersettings.h>
#include <scenesettings.h>
#include <generalsettings.h>
#include <interfacesettings.h>
#include <form.h>
#include <gadgetlayer.h>
#include <mouselayer.h>
#include <paintlayer.h>
#include <hashfactory.h>
#include <utilityfunctions.h>
#include <controlpropertymanager.h>
#include <projectexposingmanager.h>
#include <controlremovingmanager.h>
#include <controlproductionmanager.h>

#include <QtMath>
#include <QMimeData>
#include <QGraphicsSceneMouseEvent>

DesignerScene::DesignerScene(QObject* parent) : QGraphicsScene(parent)
  , m_anchorVisibility(VisibleForSelectedControlsOnly)
  , m_cursorShape(Qt::CustomCursor)
  , m_dragLayer(new DesignerItem)
  , m_gadgetLayer(new GadgetLayer)
  , m_mouseLayer(new MouseLayer)
  , m_paintLayer(new PaintLayer)
  , m_parentBeforeDrag(nullptr)
{
    setItemIndexMethod(QGraphicsScene::NoIndex);

    m_dragLayer->setAcceptedMouseButtons(Qt::NoButton);
    m_dragLayer->setFlag(DesignerItem::ItemHasNoContents);
    m_dragLayer->setZValue(std::numeric_limits<int>::max());
    addItem(m_dragLayer);

    m_gadgetLayer->setAcceptedMouseButtons(Qt::NoButton);
    m_gadgetLayer->setZValue(std::numeric_limits<int>::max());
    addItem(m_gadgetLayer);

    m_mouseLayer->setAcceptedMouseButtons(Qt::RightButton);
    m_mouseLayer->setZValue(std::numeric_limits<int>::max());
    addItem(m_mouseLayer);

    m_paintLayer->setAcceptedMouseButtons(Qt::NoButton);
    m_paintLayer->setZValue(std::numeric_limits<int>::max());
    addItem(m_paintLayer);

    // Make sure DesignerView is initalized
    QMetaObject::invokeMethod(this, &DesignerScene::onSceneSettingsChange, Qt::QueuedConnection);

    connect(DesignerSettings::instance(), &DesignerSettings::sceneSettingsChanged,
            this, &DesignerScene::onSceneSettingsChange);
    connect(this, &DesignerScene::changed,
            this, &DesignerScene::onChange);
    connect(this, &DesignerScene::selectionChanged,
            m_gadgetLayer, &GadgetLayer::handleSceneSelectionChange);
    connect(this, &DesignerScene::currentFormChanged,
            m_gadgetLayer, &GadgetLayer::handleSceneCurrentFormChange);
    connect(ControlProductionManager::instance(), &ControlProductionManager::controlProduced,
            m_gadgetLayer, &GadgetLayer::addResizers);
    connect(ProjectExposingManager::instance(), &ProjectExposingManager::controlExposed,
            m_gadgetLayer, &GadgetLayer::addResizers);
    connect(ControlRemovingManager::instance(), &ControlRemovingManager::controlAboutToBeRemoved,
            m_gadgetLayer, &GadgetLayer::removeResizers);
    connect(ControlPropertyManager::instance(), &ControlPropertyManager::geometryChanged,
            m_mouseLayer, &MouseLayer::updateGeometry);
    connect(this, &DesignerScene::currentFormChanged,
            m_mouseLayer, &MouseLayer::updateGeometry);
    connect(m_mouseLayer, &MouseLayer::draggingActivatedChanged,
            this, &DesignerScene::onMouseLayerDraggingActivation);
    connect(ControlPropertyManager::instance(), &ControlPropertyManager::geometryChanged,
            m_paintLayer, &PaintLayer::updateGeometry);
    connect(this, &DesignerScene::currentFormChanged,
            m_paintLayer, &PaintLayer::updateGeometry);
}

void DesignerScene::clear()
{
    clearSelection();
    m_forms.clear();
    m_currentForm.clear();
    update();
}

void DesignerScene::addForm(Form* form)
{
    m_forms.insert(form);
}

void DesignerScene::removeForm(Form* form)
{
    // NOTE: If the given form address is the current form,
    // then if this line runs, QPointer clears the address
    // within m_currentForm, because its object is "delete"d
    // So no need to clear it again if it was the current form

    removeControl(form);

    m_forms.remove(form);
}

void DesignerScene::removeControl(Control* control)
{
    if (control->scene())
        removeItem(control);
    delete control; // Deletes its children too
}

void DesignerScene::setCurrentForm(Form* currentForm)
{
    if (m_currentForm == currentForm)
        return;

    if (!m_forms.contains(currentForm))
        return;

    /*
        NOTE: ControlsPane dependency: We emit currentFormChanged signal before actually showing
              it thus "selectionChanged" signal is being emitted afterwards. Otherwise selectionChanged
              signal getting emitted before currentFormChanged signal, hence ControlsPane clears
              the selection before saving selection state of a form in currentFormChanged signal.
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

void DesignerScene::shrinkSceneRect()
{
    // 10 margin is a protection against a bug that
    // moves forms unexpectedly when they are selected

    setSceneRect(visibleItemsBoundingRect().adjusted(-10, -10, 10, 10));
}

void DesignerScene::unsetCursor()
{
    if (m_cursorShape != Qt::CustomCursor) {
        view()->viewport()->setCursor(m_cursorShape);
        m_cursorShape = Qt::CustomCursor;
    }
}

void DesignerScene::setCursor(Qt::CursorShape cursorShape)
{
    m_cursorShape = view()->viewport()->cursor().shape();
    view()->viewport()->setCursor(cursorShape);
}

void DesignerScene::prepareDragLayer(const DesignerItem* item)
{
    m_parentBeforeDrag = item->parentItem();
    if (const DesignerItem* parentItem = item->parentItem()) {
        m_dragLayer->setTransform(QTransform::fromTranslate(parentItem->scenePos().x(),
                                                            parentItem->scenePos().y()));
    }
}

bool DesignerScene::isLayerItem(const DesignerItem* item) const
{
    return item == m_dragLayer
            || item == m_gadgetLayer
            || item == m_mouseLayer
            || item == m_paintLayer;
}

Form* DesignerScene::currentForm() const
{
    return m_currentForm;
}

DesignerItem* DesignerScene::dragLayer() const
{
    return m_dragLayer;
}

GadgetLayer* DesignerScene::gadgetLayer() const
{
    return m_gadgetLayer;
}

MouseLayer* DesignerScene::mouseLayer() const
{
    return m_mouseLayer;
}

DesignerItem* DesignerScene::parentBeforeDrag() const
{
    return m_parentBeforeDrag;
}

DesignerView* DesignerScene::view() const
{
    Q_ASSERT(views().size() == 1);
    return static_cast<DesignerView*>(views().first());
}

QList<Form*> DesignerScene::forms() const
{
    return m_forms.values();
}

QList<Control*> DesignerScene::selectedControls() const
{
    const QList<QGraphicsItem*>& items = QGraphicsScene::selectedItems();
    QList<Control*> selectedControls;
    for (QGraphicsItem* selectedItem : items) {
        if (selectedItem->type() >= Control::Type)
            selectedControls.append(static_cast<Control*>(selectedItem));
    }
    return selectedControls;
}

QList<DesignerItem*> DesignerScene::selectedItems() const
{
    const QList<QGraphicsItem*>& items = QGraphicsScene::selectedItems();
    QList<DesignerItem*> selectedItems;
    for (QGraphicsItem* selectedItem : items) {
        if (selectedItem->type() >= DesignerItem::Type)
            selectedItems.append(static_cast<DesignerItem*>(selectedItem));
    }
    return selectedItems;
}

QList<DesignerItem*> DesignerScene::draggedResizedSelectedItems() const
{
    QList<DesignerItem*> items(selectedItems());
    for (int i = items.size() - 1; i >= 0; --i) {
        const DesignerItem* item = items.at(i);
        if (!item->beingDragged() && !item->beingResized())
            items.removeAt(i);
    }
    return items;
}

Control* DesignerScene::topLevelControl(const QPointF& pos) const
{
    const QList<Control*>& allItems = items<Control>(pos);
    if (allItems.isEmpty())
        return nullptr;
    return allItems.first();
}

Control* DesignerScene::highlightControl(const QPointF& pos) const
{
    QList<DesignerItem*> draggedItems = draggedResizedSelectedItems();
    for (int i = draggedItems.size() - 1; i >= 0; --i) {
        const DesignerItem* item = draggedItems.at(i);
        if (item->beingResized())
            return nullptr;
        draggedItems.append(item->childItems());
    }

    if (draggedItems.isEmpty())
        return nullptr;

    QList<Control*> itemsAtPos = items<Control>(pos);
    for (int i = itemsAtPos.size() - 1; i >= 0; --i) {
        if (draggedItems.contains(itemsAtPos.at(i)))
            itemsAtPos.removeAt(i);
    }

    if (itemsAtPos.isEmpty())
        return nullptr;

    // Ordered based on stacking order, higher first
    return itemsAtPos.first();
}

QRectF DesignerScene::outerRect(const QRectF& rect) const
{
    // Use on rectangular shapes where the item
    // is transformable but the pen is cosmetic

    return rect.adjusted(-0.5 / zoomLevel(), -0.5 / zoomLevel(), 0, 0);
}

QRectF DesignerScene::visibleItemsBoundingRect() const
{
    // Does not take untransformable items into account.
    QRectF boundingRect;
    const QList<DesignerItem*>& allItems = items();
    for (const DesignerItem* item : allItems) {
        if (item->isVisible() && !isLayerItem(item))
            boundingRect |= item->sceneBoundingRect();
    }
    return boundingRect.adjusted(-10, -15, 10, 10);
}

qreal DesignerScene::zoomLevel() const
{
    return view()->matrix().m11();
}

qreal DesignerScene::devicePixelRatio() const
{
    return view()->devicePixelRatioF();
}

QPointF DesignerScene::cursorPos() const
{
    return view()->mapToScene(view()->viewport()->mapFromGlobal(QCursor::pos()));
}

QVector<QLineF> DesignerScene::guidelines() const
{
    using namespace UtilityFunctions;

    QVector<QLineF> lines;
    QList<Control*> allControls;
    const QList<DesignerItem*>& movingItems = draggedResizedSelectedItems();

    if (movingItems.isEmpty())
        return lines;

    if (m_currentForm) {
        allControls.append(m_currentForm);
        allControls.append(m_currentForm->childControls());
    }

    for (DesignerItem* movingItem : movingItems) {
        if (movingItem->type() >= Control::Type)
            allControls.removeOne(static_cast<Control*>(movingItem));
    }

    if (allControls.isEmpty())
        return lines;

    const QRectF& geometry = itemsBoundingRect(movingItems);

    for (Control* control : qAsConst(allControls)) {
        const QRectF& otherGeometry = control->sceneBoundingRect();

        /* My center <-> Other center */
        if (qRound64(geometry.center().x()) == qRound64(otherGeometry.center().x()))
            lines.append({geometry.center(), otherGeometry.center()});

        if (qRound64(geometry.center().y()) == qRound64(otherGeometry.center().y()))
            lines.append({geometry.center(), otherGeometry.center()});

        /* My left <-> Other center */
        if (qRound64(geometry.left()) == qRound64(otherGeometry.center().x()))
            lines.append({leftCenter(geometry), otherGeometry.center()});

        /* My right <-> Other center */
        if (qRound64(geometry.right()) == qRound64(otherGeometry.center().x()))
            lines.append({rightCenter(geometry), otherGeometry.center()});

        /* My top <-> Other center */
        if (qRound64(geometry.top()) == qRound64(otherGeometry.center().y()))
            lines.append({topCenter(geometry), otherGeometry.center()});

        /* My bottom <-> Other center */
        if (qRound64(geometry.bottom()) == qRound64(otherGeometry.center().y()))
            lines.append({bottomCenter(geometry), otherGeometry.center()});

        /* My center <-> Other left */
        if (qRound64(geometry.center().x()) == qRound64(otherGeometry.left()))
            lines.append({geometry.center(), leftCenter(otherGeometry)});

        /* My left <-> Other left */
        if (qRound64(geometry.left()) == qRound64(otherGeometry.left()))
            lines.append({leftCenter(geometry), leftCenter(otherGeometry)});

        /* My right <-> Other left */
        if (qRound64(geometry.right()) == qRound64(otherGeometry.left()))
            lines.append({rightCenter(geometry), leftCenter(otherGeometry)});

        /* My center <-> Other right */
        if (qRound64(geometry.center().x()) == qRound64(otherGeometry.right()))
            lines.append({geometry.center(), rightCenter(otherGeometry)});

        /* My left <-> Other right */
        if (qRound64(geometry.left()) == qRound64(otherGeometry.right()))
            lines.append({leftCenter(geometry), rightCenter(otherGeometry)});

        /* My right <-> Other right */
        if (qRound64(geometry.right()) == qRound64(otherGeometry.right()))
            lines.append({rightCenter(geometry), rightCenter(otherGeometry)});

        /* My center <-> Other top */
        if (qRound64(geometry.center().y()) == qRound64(otherGeometry.top()))
            lines.append({geometry.center(), topCenter(otherGeometry)});

        /* My top <-> Other top */
        if (qRound64(geometry.top()) == qRound64(otherGeometry.top()))
            lines.append({topCenter(geometry), topCenter(otherGeometry)});

        /* My bottom <-> Other top */
        if (qRound64(geometry.bottom()) == qRound64(otherGeometry.top()))
            lines.append({bottomCenter(geometry), topCenter(otherGeometry)});

        /* My center <-> Other bottom */
        if (qRound64(geometry.center().y()) == qRound64(otherGeometry.bottom()))
            lines.append({geometry.center(), bottomCenter(otherGeometry)});

        /* My top <-> Other bottom */
        if (qRound64(geometry.top()) == qRound64(otherGeometry.bottom()))
            lines.append({topCenter(geometry), bottomCenter(otherGeometry)});

        /* My bottom <-> Other bottom */
        if (qRound64(geometry.bottom()) == qRound64(otherGeometry.bottom()))
            lines.append({bottomCenter(geometry), bottomCenter(otherGeometry)});
    }
    return lines;
}

DesignerScene::AnchorVisibility DesignerScene::anchorVisibility() const
{
    return m_anchorVisibility;
}

void DesignerScene::setAnchorVisibility(DesignerScene::AnchorVisibility anchorVisibility)
{
    if (m_anchorVisibility != anchorVisibility) {
        m_anchorVisibility = anchorVisibility;
        update();
    }
}

void DesignerScene::reparentControl(Control* control, Control* parentControl, const QPointF& pos) const
{
    // NOTE: We compress setPos because there might be some other
    // compressed setPos'es in the list, We want the setPos that
    // happens after reparent operation to take place at the very last
    ControlPropertyManager::Options options = ControlPropertyManager::SaveChanges
            | ControlPropertyManager::CompressedCall;

    if (control->gui())
        options |= ControlPropertyManager::UpdateRenderer;

    control->setGeometrySyncEnabled(false);
    control->m_geometrySyncKey = HashFactory::generate();

    // NOTE: Do not move this assignment below setParent,
    // because parent change effects the newPos result
    // NOTE: Move the item position backwards as much as next parent margins are
    // Because it will be followed by a ControlPropertyManager::setParent call and it
    // will move the item by setting a transform on it according to its parent margin
    const QPointF margins(parentControl->margins().left(), parentControl->margins().top());
    const QPointF& newPos = DesignerScene::snapPosition(control->mapToItem(parentControl, -margins));
    ControlPropertyManager::setParent(control, parentControl, ControlPropertyManager::SaveChanges |
                                      ControlPropertyManager::UpdateRenderer);
    ControlPropertyManager::setPos(control, pos.isNull() ? newPos : pos, options, control->m_geometrySyncKey);
}

bool DesignerScene::isAnchorViable(const Control* sourceControl, const Control* targetControl)
{
    if (isInappropriateAnchorSource(sourceControl))
        return false;

    if (isInappropriateAnchorTarget(targetControl))
        return false;

    if (sourceControl->popup() && sourceControl->parentControl() != targetControl)
        return false;

    if (targetControl->window() && sourceControl->parentControl() != targetControl)
        return false;

    if (targetControl->popup() && sourceControl->parentControl() != targetControl)
        return false;

    if (!sourceControl->siblings().contains(const_cast<Control*>(targetControl)) &&
            sourceControl->parentControl() != targetControl) {
        return false;
    }

    return true;
}

bool DesignerScene::isInappropriateAnchorSource(const Control* control)
{
    if (control == 0)
        return true;

    if (!control->gui())
        return true;

    if (control->window())
        return true;

    return false;
}

bool DesignerScene::isInappropriateAnchorTarget(const Control* control)
{
    if (control == 0)
        return true;

    if (!control->gui())
        return true;

    return false;
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

QRectF DesignerScene::contentRect(const Control* control)
{
    const QMarginsF& margins = control->margins();
    return control->sceneBoundingRect().adjusted(margins.left(), margins.top(), -margins.right(), -margins.bottom());
}

QRectF DesignerScene::itemsBoundingRect(const QList<DesignerItem*>& items)
{
    QRectF boundingRect;
    for (DesignerItem* item : items)
        boundingRect |= item->sceneBoundingRect();
    return boundingRect;
}

qreal DesignerScene::lowerZ(const DesignerItem* parentItem)
{
    qreal z = 0;
    const QList<DesignerItem*>& children = parentItem->childItems(false);
    for (DesignerItem* childItem : children) {
        if (childItem->zValue() < z)
            z = childItem->zValue();
    }
    return z;
}

qreal DesignerScene::higherZ(const DesignerItem* parentItem)
{
    qreal z = 0;
    const QList<DesignerItem*>& children = parentItem->childItems(false);
    for (DesignerItem* childItem : children) {
        if (childItem->zValue() > z)
            z = childItem->zValue();
    }
    return z;
}

QPen DesignerScene::pen(const QColor& color, qreal width, bool cosmetic)
{
    QPen pen(color.isValid() ? color : GeneralSettings::interfaceSettings()->highlightColor, width,
             Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin);
    pen.setCosmetic(cosmetic);
    return pen;
}

void DesignerScene::drawDashLine(QPainter* painter, const QLineF& line)
{
    painter->save();
    QPen linePen(pen(QColor(0, 0, 0, 200), 2));
    linePen.setDashPattern({3, 2});
    painter->setPen(linePen);
    painter->setBrush(Qt::NoBrush);
    painter->drawLine(line);

    linePen.setColor(QColor(255, 255, 255, 200));
    linePen.setDashPattern({2, 3});
    linePen.setDashOffset(3);
    painter->setPen(linePen);
    painter->drawLine(line);
    painter->restore();
}

void DesignerScene::drawDashRect(QPainter* painter, const QRectF& rect)
{
    painter->save();
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
    painter->restore();
}

void DesignerScene::onChange()
{
    setSceneRect(sceneRect() | visibleItemsBoundingRect());
}

void DesignerScene::onMouseLayerDraggingActivation()
{
    if (m_mouseLayer->draggingActivated()) {
        setAnchorVisibility(anchorVisibility() | VisibleForAllControlsDueToMouseLayerDraggingActivation);
    } else {
        setAnchorVisibility(anchorVisibility() & ~VisibleForAllControlsDueToMouseLayerDraggingActivation);
        Control* sourceControl = m_mouseLayer->mouseStartControl();
        Control* targetControl = m_mouseLayer->mouseEndControl();
        if (isAnchorViable(sourceControl, targetControl))
            emit anchorEditorActivated(sourceControl, targetControl);
    }
}

void DesignerScene::onSceneSettingsChange()
{
    const SceneSettings* settings = DesignerSettings::sceneSettings();
    if (zoomLevel() != settings->sceneZoomLevel) {
        view()->resetTransform();
        view()->scale(settings->sceneZoomLevel, settings->sceneZoomLevel);
    }
    const QList<Control*>& allControls = Control::controls();
    for (Control* control : allControls)
        control->setRenderInfo(control->renderInfo());
    update();
}

void DesignerScene::handleToolDrop(QGraphicsSceneDragDropEvent* event)
{
    QString dir;
    QString module;
    RenderInfo info;

    UtilityFunctions::pull(event->mimeData()->data(QStringLiteral("application/x-objectwheel-tool")), dir);
    UtilityFunctions::pull(event->mimeData()->data(QStringLiteral("application/x-objectwheel-module")), module);
    UtilityFunctions::pull(event->mimeData()->data(QStringLiteral("application/x-objectwheel-render-info")), info);

    info.image.setDevicePixelRatio(devicePixelRatio()); // QDataStream cannot write dpr
    // NOTE: Move the item position backwards as much as next parent margins are
    // Because it will be followed by a ControlPropertyManager::setParent call and it
    // will move the item by setting a transform on it according to its parent margin
    Control* parentControl = static_cast<Control*>(m_recentHighlightedItem.data());
    const QPointF margins(parentControl->margins().left(), parentControl->margins().top());
    Control* newControl = ControlProductionManager::produceControl(
                parentControl,
                dir, module,
                DesignerScene::snapPosition(parentControl->mapFromScene(event->scenePos() - QPointF(3, 3) - margins)),
                info.surroundingRect.size(), UtilityFunctions::imageToPixmap(info.image));
    if (newControl) {
        clearSelection();
        newControl->setSelected(true);
    } else {
        UtilityFunctions::showMessage(view(),
                                      tr("Oops"),
                                      tr("Operation failed, control has got problems."),
                                      QMessageBox::Critical);
    }
}

void DesignerScene::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsScene::mouseMoveEvent(event);

    if (mouseGrabberItem()) {
        if (m_recentHighlightedItem)
            m_recentHighlightedItem->setBeingHighlighted(false);
        if ((m_recentHighlightedItem = highlightControl(event->scenePos())))
            m_recentHighlightedItem->setBeingHighlighted(true);
    }
}

void DesignerScene::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    const QList<DesignerItem*>& draggedItems = draggedResizedSelectedItems();

    QGraphicsScene::mouseReleaseEvent(event); // Clears beingResized and beingDragged states

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

void DesignerScene::dragMoveEvent(QGraphicsSceneDragDropEvent* event)
{
    if (event->mimeData()->hasFormat(QStringLiteral("application/x-objectwheel-tool"))) {
        if (m_recentHighlightedItem)
            m_recentHighlightedItem->setBeingHighlighted(false);
        m_recentHighlightedItem = topLevelControl(event->scenePos());
        if (m_recentHighlightedItem == 0)
            m_recentHighlightedItem = m_currentForm;
        if (m_recentHighlightedItem)
            m_recentHighlightedItem->setBeingHighlighted(true);
        event->setAccepted(m_recentHighlightedItem);
    } else {
        QGraphicsScene::dragMoveEvent(event);
    }
}

void DesignerScene::dragLeaveEvent(QGraphicsSceneDragDropEvent* event)
{
    QGraphicsScene::dragLeaveEvent(event);

    if (m_recentHighlightedItem) {
        m_recentHighlightedItem->setBeingHighlighted(false);
        m_recentHighlightedItem.clear();
    }
}

void DesignerScene::dropEvent(QGraphicsSceneDragDropEvent* event)
{
    if (event->mimeData()->hasFormat(QStringLiteral("application/x-objectwheel-tool"))
            && m_recentHighlightedItem) {
        handleToolDrop(event);
        m_recentHighlightedItem->setBeingHighlighted(false);
        m_recentHighlightedItem.clear();
        event->acceptProposedAction();
    } else {
        QGraphicsScene::dropEvent(event);
    }
}
