#include <designerscene.h>
#include <resizeritem.h>
#include <controlpropertymanager.h>
#include <projectexposingmanager.h>
#include <controlcreationmanager.h>
#include <controlremovingmanager.h>
#include <designersettings.h>
#include <scenesettings.h>
#include <headlineitem.h>
#include <gadgetlayer.h>
#include <anchorlayer.h>
#include <paintlayer.h>
#include <windowmanager.h>
#include <centralwidget.h>
#include <designerview.h>
#include <mainwindow.h>
#include <hashfactory.h>
#include <utilityfunctions.h>

#include <QMimeData>
#include <QGraphicsView>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QPointer>
#include <QPen>
#include <QtMath>
#include <QApplication>
#include <QTimer>

#include <anchoreditor.h>
DesignerScene::DesignerScene(QObject* parent) : QGraphicsScene(parent)
  , m_dragLayer(new DesignerItem)
  , m_gadgetLayer(new GadgetLayer)
  , m_anchorLayer(new AnchorLayer)
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

    m_anchorLayer->setAcceptedMouseButtons(Qt::RightButton);
    m_anchorLayer->setZValue(std::numeric_limits<int>::max());
    addItem(m_anchorLayer);

    m_paintLayer->setAcceptedMouseButtons(Qt::NoButton);
    m_paintLayer->setZValue(std::numeric_limits<int>::max());
    addItem(m_paintLayer);

    connect(this, &DesignerScene::changed,
            this, &DesignerScene::onChange);
    connect(this, &DesignerScene::selectionChanged,
            m_gadgetLayer, &GadgetLayer::handleSceneSelectionChange);
    connect(this, &DesignerScene::currentFormChanged,
            m_gadgetLayer, &GadgetLayer::handleSceneCurrentFormChange);
    connect(m_gadgetLayer, &GadgetLayer::headlineDoubleClicked,
            this, &DesignerScene::onHeadlineDoubleClick);
    connect(ControlCreationManager::instance(), &ControlCreationManager::controlCreated,
            m_gadgetLayer, &GadgetLayer::addResizers);
    connect(ProjectExposingManager::instance(), &ProjectExposingManager::controlExposed,
            m_gadgetLayer, &GadgetLayer::addResizers);
    connect(ControlRemovingManager::instance(), &ControlRemovingManager::controlAboutToBeRemoved,
            m_gadgetLayer, &GadgetLayer::removeResizers);
    connect(ControlPropertyManager::instance(), &ControlPropertyManager::geometryChanged,
            m_anchorLayer, &AnchorLayer::updateGeometry);
    connect(this, &DesignerScene::currentFormChanged,
            m_anchorLayer, &AnchorLayer::updateGeometry);
    connect(m_anchorLayer, &AnchorLayer::activatedChanged, // FIXME
            m_paintLayer, [=] {
        m_paintLayer->update();
        if (!m_anchorLayer->activated()) {
            static auto e = new AnchorEditor(0);
            const QLineF line(anchorLayer()->mapToScene(anchorLayer()->mousePressPoint()),
                              anchorLayer()->mapToScene(anchorLayer()->mouseMovePoint()));
            e->activate(topLevelControl(line.p1()), topLevelControl(line.p2()));
        }
    });
    connect(ControlPropertyManager::instance(), &ControlPropertyManager::geometryChanged,
            m_paintLayer, &PaintLayer::updateGeometry);
    connect(this, &DesignerScene::currentFormChanged,
            m_paintLayer, &PaintLayer::updateGeometry);
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
        NOTE: InspectorPane dependency: We emit currentFormChanged signal before actually showing
              it thus "selectionChanged" signal is being emitted afterwards. Otherwise selectionChanged
              signal getting emitted before currentFormChanged signal, hence InspectorPane clears
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

void DesignerScene::unsetCursor() const
{
    Q_ASSERT(views().size() == 1);
    views().first()->viewport()->unsetCursor();
}

void DesignerScene::setCursor(Qt::CursorShape cursor) const
{
    Q_ASSERT(views().size() == 1);
    views().first()->viewport()->setCursor(cursor);
}

void DesignerScene::prepareDragLayer(DesignerItem* item)
{
    m_parentBeforeDrag = item->parentItem();
    if (const DesignerItem* parentItem = item->parentItem()) {
        m_dragLayer->setTransform(QTransform::fromTranslate(parentItem->scenePos().x(),
                                                            parentItem->scenePos().y()));
    }
}

bool DesignerScene::isLayerItem(DesignerItem* item) const
{
    return item == m_dragLayer
            || item == m_gadgetLayer
            || item == m_anchorLayer
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

AnchorLayer* DesignerScene::anchorLayer() const
{
    return m_anchorLayer;
}

PaintLayer* DesignerScene::paintLayer() const
{
    return m_paintLayer;
}

DesignerItem* DesignerScene::parentBeforeDrag() const
{
    return m_parentBeforeDrag;
}

QList<Form*> DesignerScene::forms() const
{
    return m_forms.toList();
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

QList<DesignerItem*> DesignerScene::selectedItems() const
{
    QList<DesignerItem*> selectedItems;
    for (QGraphicsItem* selectedItem : QGraphicsScene::selectedItems()) {
        if (selectedItem->type() >= DesignerItem::Type)
            selectedItems.append(static_cast<DesignerItem*>(selectedItem));
    }
    return selectedItems;
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

Control* DesignerScene::topLevelControl(const QPointF& pos) const
{
    const QList<Control*> allItems(items<Control>(pos));
    if (allItems.isEmpty())
        return nullptr;
    return allItems.first();
}

Control* DesignerScene::highlightControl(const QPointF& pos) const
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

    QList<Control*> itemsAtPos = items<Control>(pos);
    for (int i = itemsAtPos.size() - 1; i >= 0; --i) {
        DesignerItem* item = itemsAtPos.at(i);
        if (draggedItems.contains(item))
            itemsAtPos.removeAt(i);
    }

    if (itemsAtPos.isEmpty())
        return nullptr;

    // Ordered based on stacking order, higher first
    return itemsAtPos.first();
}

bool DesignerScene::showAllAnchors() const
{
    // TODO: Take that from view
    // TODO: Make sure it returns true when anchor editor is open
    return true || anchorLayer()->activated();
}

qreal DesignerScene::devicePixelRatio() const
{
    Q_ASSERT(views().size() == 1);
    return views().first()->devicePixelRatioF();
}

QPointF DesignerScene::cursorPos() const
{
    Q_ASSERT(views().size() == 1);
    QGraphicsView* view = views().first();
    return view->mapToScene(view->viewport()->mapFromGlobal(QCursor::pos()));
}

QRectF DesignerScene::visibleItemsBoundingRect() const
{
    // Does not take untransformable items into account.
    QRectF boundingRect;
    for (DesignerItem *item : items()) {
        if (item->isVisible() && !isLayerItem(item))
            boundingRect |= item->sceneBoundingRect();
    }
    return boundingRect.adjusted(-10, -15, 10, 10);
}

QVector<QLineF> DesignerScene::guidelines() const
{
    // FIXME: doesn't correctly work for items within a parent
    using namespace UtilityFunctions;

    QVector<QLineF> lines; /*FIXME:*/ return lines;
    QList<Control*> stillItems = items<Control>();
    const QList<DesignerItem*>& movingItems = draggedResizedSelectedItems();

    if (m_currentForm) {
        stillItems.append(m_currentForm);
        stillItems.append(m_currentForm->childControls());
    }

    for (DesignerItem* movingItem : movingItems)
        stillItems.removeOne(static_cast<Control*>(movingItem));

    for (DesignerItem* stillItem : qAsConst(stillItems)) {
        const QRectF& geometry = itemsBoundingRect(movingItems);
        const QRectF& otherGeometry = stillItem->sceneBoundingRect();

        /* Child center <-> Parent center */
        if (qRound64(geometry.center().x()) == qRound64(otherGeometry.center().x()))
            lines.append({geometry.center(), otherGeometry.center()});

        if (qRound64(geometry.center().y()) == qRound64(otherGeometry.center().y()))
            lines.append({geometry.center(), otherGeometry.center()});

        /* Child left <-> Parent center */
        if (qRound64(leftCenter(geometry).x()) == qRound64(otherGeometry.center().x()))
            lines.append({leftCenter(geometry), otherGeometry.center()});

        /* Child left <-> Parent left */
        if (qRound64(leftCenter(geometry).x()) == qRound64(otherGeometry.x()))
            lines.append({leftCenter(geometry), leftCenter(otherGeometry)});

        /* Child right <-> Parent center */
        if (qRound64(rightCenter(geometry).x()) == qRound64(otherGeometry.center().x()))
            lines.append({rightCenter(geometry), otherGeometry.center()});

        /* Child right <-> Parent right */
        if (qRound64(rightCenter(geometry).x()) == qRound64(rightCenter(otherGeometry).x()))
            lines.append({rightCenter(geometry), rightCenter(otherGeometry)});

        /* Child top <-> Parent center */
        if (qRound64(topCenter(geometry).y()) == qRound64(otherGeometry.center().y()))
            lines.append({topCenter(geometry), otherGeometry.center()});

        /* Child top <-> Parent top */
        if (qRound64(topCenter(geometry).y()) == qRound64(topCenter(otherGeometry).y()))
            lines.append({topCenter(geometry), topCenter(otherGeometry)});

        /* Child bottom <-> Parent center */
        if (qRound64(bottomCenter(geometry).y()) == qRound64(otherGeometry.center().y()))
            lines.append({bottomCenter(geometry), otherGeometry.center()});

        /* Child bottom <-> Parent bottom */
        if (qRound64(bottomCenter(geometry).y()) == qRound64(bottomCenter(otherGeometry).y()))
            lines.append({bottomCenter(geometry), bottomCenter(otherGeometry)});
    }
    return lines;
}

bool DesignerScene::showGridViewDots()
{
    return DesignerSettings::sceneSettings()->showGridViewDots;
}

bool DesignerScene::showClippedControls()
{
    return DesignerSettings::sceneSettings()->showClippedControls;
}

bool DesignerScene::showMouseoverOutline()
{
    return DesignerSettings::sceneSettings()->showMouseoverOutline;
}

int DesignerScene::gridSize()
{
    return DesignerSettings::sceneSettings()->gridSize;
}

int DesignerScene::startDragDistance()
{
    return DesignerSettings::sceneSettings()->dragStartDistance;
}

qreal DesignerScene::zoomLevel()
{
    return DesignerSettings::sceneSettings()->sceneZoomLevel;
}

DesignerScene::OutlineMode DesignerScene::outlineMode()
{
    return OutlineMode(DesignerSettings::sceneSettings()->controlOutlineDecoration);
}

QColor DesignerScene::anchorColor()
{
    return DesignerSettings::sceneSettings()->anchorColor;
}

QColor DesignerScene::outlineColor()
{
    return DesignerSettings::sceneSettings()->outlineColor;
}

QBrush DesignerScene::backgroundTexture()
{
    return DesignerSettings::sceneSettings()->toBackgroundBrush();
}

QBrush DesignerScene::blankControlDecorationBrush(const QColor& color)
{
    return DesignerSettings::sceneSettings()->toBlankControlDecorationBrush(color);
}

QPen DesignerScene::pen(const QColor& color, qreal width, bool cosmetic)
{
    QPen pen(color, width, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin);
    pen.setCosmetic(cosmetic);
    return pen;
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

QRectF DesignerScene::outerRect(const QRectF& rect)
{
    // Use on rectangular shapes where the item
    // is transformable but the pen is cosmetic

    return rect.adjusted(-0.5 / zoomLevel(), -0.5 / zoomLevel(), 0, 0);
}

QRectF DesignerScene::itemsBoundingRect(const QList<DesignerItem*>& items)
{
    QRectF boundingRect;
    for (DesignerItem* item : items)
        boundingRect |= item->sceneBoundingRect();
    return boundingRect;
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

void DesignerScene::discharge()
{
    clearSelection();
    m_forms.clear();
    m_currentForm.clear();
}

void DesignerScene::onChange()
{
    setSceneRect(sceneRect() | visibleItemsBoundingRect());
}

void DesignerScene::onHeadlineDoubleClick(bool isFormHeadline)
{
    ControlPropertyManager::instance()->doubleClicked(isFormHeadline
                                                      ? m_currentForm
                                                      : selectedControls().first());
}

void DesignerScene::handleToolDrop(QGraphicsSceneDragDropEvent* event)
{
    QString dir;
    RenderInfo info;
    UtilityFunctions::pull(event->mimeData()->data(QStringLiteral("application/x-objectwheel-tool")), dir);
    UtilityFunctions::pull(event->mimeData()->data(QStringLiteral("application/x-objectwheel-render-info")), info);

    info.image.setDevicePixelRatio(devicePixelRatio()); // QDataStream cannot write dpr
    // NOTE: Move the item position backwards as much as next parent margins are
    // Because it will be followed by a ControlPropertyManager::setParent call and it
    // will move the item by setting a transform on it according to its parent margin
    Control* parentControl = static_cast<Control*>(m_recentHighlightedItem.data());
    const QPointF margins(parentControl->margins().left(), parentControl->margins().top());
    Control* newControl = ControlCreationManager::createControl(
                parentControl,
                dir, DesignerScene::snapPosition(parentControl->mapFromScene(event->scenePos() - QPointF(5, 5) - margins)),
                info.surroundingRect.size(), UtilityFunctions::imageToPixmap(info.image));
    if (newControl) {
        clearSelection();
        newControl->setSelected(true);
    } else {
        UtilityFunctions::showMessage(nullptr,
                                      tr("Oops"),
                                      tr("Operation failed, control has got problems."),
                                      QMessageBox::Critical);
    }
}

void DesignerScene::reparentControl(Control* control, Control* parentControl) const
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
    ControlPropertyManager::setParent(control, parentControl, ControlPropertyManager::SaveChanges
                                      | ControlPropertyManager::UpdateRenderer);
    ControlPropertyManager::setPos(control, newPos, options, control->m_geometrySyncKey);
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
        if ((m_recentHighlightedItem = topLevelControl(event->scenePos())))
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
