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
#include <windowmanager.h>
#include <centralwidget.h>
#include <designerview.h>
#include <mainwindow.h>

#include <QGraphicsView>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QPointer>
#include <QPen>
#include <QtMath>
#include <QApplication>
#include <QTimer>

DesignerScene::DesignerScene(QObject* parent) : QGraphicsScene(parent)
  , m_currentForm(nullptr)
  , m_dragLayer(new DesignerItem)
  , m_gadgetLayer(new GadgetLayer(this))
{
    setItemIndexMethod(QGraphicsScene::NoIndex);

    m_dragLayer->setAcceptedMouseButtons(Qt::NoButton);
    m_dragLayer->setFlag(DesignerItem::ItemHasNoContents);
    m_dragLayer->setZValue(std::numeric_limits<int>::max());
    addItem(m_dragLayer);

    m_gadgetLayer->setAcceptedMouseButtons(Qt::NoButton);
    m_gadgetLayer->setZValue(std::numeric_limits<int>::max());
    addItem(m_gadgetLayer);

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
    delete control;
}

Form* DesignerScene::currentForm() const
{
    return m_currentForm.data();
}

qreal DesignerScene::zoomLevel() const
{
    return views().first()->transform().m11();
}

int DesignerScene::startDragDistance()
{
    const SceneSettings* settings = DesignerSettings::sceneSettings();
    return settings->dragStartDistance;
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
    // 10 margin is a protection against the unexpected
    // form movement that happens when a form is selected
    setSceneRect(visibleItemsBoundingRect().adjusted(-10, -10, 10, 10));
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
    return rect.adjusted(-0.5 / zoomLevel(), -0.5 / zoomLevel(), 0, 0);
}

QList<Control*> DesignerScene::controlsAt(const QPointF& pos) const
{
    QList<Control*> controls;

    const QList<QGraphicsItem*>& itemsUnderPos = items(pos);
    for (QGraphicsItem* item : itemsUnderPos) {
        if (item->type() == Control::Type || item->type() == Form::Type)
            controls.append(static_cast<Control*>(item));
    }

    return controls;
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
    for (QGraphicsItem *item : items()) {
        if (item->isVisible())
            boundingRect |= item->sceneBoundingRect();
    }
    return boundingRect.adjusted(-10, -15, 10, 10);
}

void DesignerScene::drawForeground(QPainter* painter, const QRectF& rect)
{
    QGraphicsScene::drawForeground(painter, rect);

    for (DesignerItem* selectedItem : selectedItems())
        paintSelectionOutline(painter, selectedItem);

    const QList<DesignerItem*>& items = draggedResizedSelectedItems();
    if (items.size() > 1) { // Multiple items moving
        const QRectF& itemsBoundingRect = boundingRect(items);
        const QRectF& outlineRect = QRectF(items.first()->parentItem()->mapToScene(itemsBoundingRect.topLeft()),
                                           itemsBoundingRect.size());
        paintOutline(painter, outlineRect);
    }

    const QVector<QLineF>& lines = guidelines();
    if (!lines.isEmpty()) {
        painter->setPen(pen());
        painter->drawLines(lines);
        for (const QLineF& line : lines) {
            painter->setBrush(outlineColor());
            painter->drawRoundedRect(QRectF(line.p1() - QPointF(1.5, 1.5), QSizeF(3.0, 3.0)), 1.5, 1.5);
            painter->drawRoundedRect(QRectF(line.p2() - QPointF(1.5, 1.5), QSizeF(3.0, 3.0)), 1.5, 1.5);
        }
    }
}

void DesignerScene::paintOutline(QPainter* painter, const QRectF& rect)
{
    QPen linePen(QColor(0, 0, 0, 150));
    linePen.setCosmetic(true);
    linePen.setCapStyle(Qt::FlatCap);
    linePen.setJoinStyle(Qt::MiterJoin);
    linePen.setDashPattern({2., 1.});

    painter->setPen(linePen);
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(rect);

    linePen.setColor(QColor(255, 255, 255, 150));
    linePen.setDashPattern({1., 2.});
    linePen.setDashOffset(2.);

    painter->setPen(linePen);
    painter->drawRect(rect);

    linePen.setColor(QColor(0, 0, 0, 150));
    linePen.setStyle(Qt::SolidLine);
    painter->setPen(linePen);
    painter->drawPoint(rect.topLeft());
    painter->drawPoint(rect.topRight() - QPointF(0.25, 0.0));
    painter->drawPoint(rect.bottomLeft());
    painter->drawPoint(rect.bottomRight() - QPointF(0.25, 0.0));
}

void DesignerScene::paintSelectionOutline(QPainter* painter, DesignerItem* selectedItem)
{
    const QRectF& rect = selectedItem->mapRectToScene(selectedItem->rect());
    const qreal m = 0.5 / zoomLevel();
    QPainterPath path;
    path.addRect(rect.adjusted(-m, -m, m, m));
    path.addRect(rect.adjusted(m, m, -m, -m));
    QPainterPath path2;
    for (ResizerItem* resizer : gadgetLayer()->resizers(selectedItem)) {
        if (resizer->isVisible())
            path2.addRect(QRectF(resizer->pos() + resizer->rect().topLeft() / zoomLevel(), resizer->size() / zoomLevel()));
    }
    painter->setPen(Qt::NoPen);
    painter->setBrush(outlineColor());
    painter->drawPath(path.subtracted(path2));
}

QVector<QLineF> DesignerScene::guidelines() const
{
    QVector<QLineF> lines;
    QList<DesignerItem*> items(selectedItems());

    for (int i = items.size() - 1; i >= 0; --i) {
        DesignerItem* item = items.at(i);
        if (!item->beingDragged() && !item->beingResized())
            items.removeAt(i);
    }

    // May contain it since we can resize the form
    if (items.contains(m_currentForm))
        items.removeOne(m_currentForm);

    if (items.isEmpty())
        return lines;

    const QRectF& geometry = boundingRect(items);
    const QPointF& center = geometry.center();
    const DesignerItem* parent = items.first()->parentItem();

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

QRectF DesignerScene::boundingRect(const QList<DesignerItem*>& items)
{
    QRectF rect = items.first()->geometry();
    for (const DesignerItem* item : items)
        if (items.first() != item)
            rect |= QRectF(items.first()->parentItem()->mapFromItem(item->parentItem(), item->pos()), item->size());
    return rect;
}

void DesignerScene::discharge()
{
    clearSelection();
    m_gadgetLayer->clearResizers();
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
