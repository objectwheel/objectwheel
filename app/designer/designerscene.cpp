#include <designerscene.h>
#include <suppressor.h>
#include <resizeritem.h>
#include <controlpropertymanager.h>
#include <designersettings.h>
#include <scenesettings.h>
#include <designerview.h>
#include <headlineitem.h>

#include <private/qgraphicsscene_p.h>

#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QPointer>
#include <QPen>
#include <QtMath>

namespace {

QRectF united(const QList<Control*>& controls)
{
    QRectF rect = controls.first()->geometry();
    for (auto control : controls)
        if (controls.first() != control)
            rect |= QRectF(controls.first()->parentControl()->mapFromItem
                           (control->parentControl(), control->pos()), control->size());
    return rect;
}

bool itemPressed = false;
bool itemMoving = false;
}

DesignerScene::DesignerScene(DesignerView* view, QObject *parent) : QGraphicsScene(parent)
  , m_view(view)
  , m_currentForm(nullptr)
{
    setItemIndexMethod(QGraphicsScene::NoIndex);
    connect(this, &DesignerScene::selectionChanged, this, [=]{
        QList<Control*> sc = selectedControls();
        sc.removeOne(currentForm());
        if (sc.isEmpty())
            return;
        if (sc.size() > 1) {
            for (Control* c : sc)
                c->headlineItem()->setVisible(false);
        } else {
            sc.first()->headlineItem()->setVisible(true);
        }
    });
}

void DesignerScene::addForm(Form* form)
{
    if (m_forms.contains(form))
        return;

    // NOTE: We don't have to call ControlPropertyManager::setParent, since there is no valid
    // parent concept for forms in Designer; fors are directly put into DesignerScene

    form->setVisible(false);

    m_forms.append(form);

    if (!m_currentForm)
        setCurrentForm(form);
}

void DesignerScene::addControl(Control* /*control*/, Control* /*parentControl*/)
{
    // TODO
    Q_ASSERT(0);
}

void DesignerScene::removeForm(Form* form)
{
    Form* currentForm = m_currentForm.data();
    removeControl(form); // 1. If the given form address is the current form, then if this line runs,
    // QPointer clears the address within m_currentForm, because its object is "delete"d
    // 2. Thus we copy it.

    m_forms.removeAll(form);

    if (currentForm == form) // 3. And we compare it here with the copied one
        setCurrentForm(m_forms.first());
}

void DesignerScene::removeControl(Control* control)
{
    removeItem(control);
    delete control;
}

Form* DesignerScene::currentForm()
{
    return m_currentForm.data();
}

DesignerView* DesignerScene::view() const
{
    return m_view;
}

qreal DesignerScene::zoomLevel() const
{
    return view()->transform().m11();
}

int DesignerScene::startDragDistance() const
{
    return QApplication::startDragDistance();
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

void DesignerScene::unsetViewportCursor()
{
    view()->viewport()->unsetCursor();
}

void DesignerScene::setViewportCursor(Qt::CursorShape cursor)
{
    view()->viewport()->setCursor(cursor);
}

QPointF DesignerScene::snapPosition(qreal x, qreal y) const
{
    return snapPosition(QPointF(x, y));
}

QPointF DesignerScene::snapPosition(const QPointF& pos) const
{
    const SceneSettings* settings = DesignerSettings::sceneSettings();
    if (settings->snappingEnabled) {
        const qreal x = qFloor(pos.x() / settings->gridSize) * settings->gridSize;
        const qreal y = qFloor(pos.y() / settings->gridSize) * settings->gridSize;
        return QPointF(x, y);
    }
    return pos;
}

QSizeF DesignerScene::snapSize(qreal x, qreal y, qreal w, qreal h) const
{
    return snapSize(QPointF(x, y), QSizeF(w, h));
}

QSizeF DesignerScene::snapSize(const QPointF& pos, const QSizeF& size) const
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
    for (auto item : selectedItems()) {
        if (item->type() == Control::Type || item->type() == Form::Type)
            selectedControls.append(static_cast<Control*>(item));
    }
    return selectedControls;
}

void DesignerScene::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsScene::mousePressEvent(event);

    if (m_currentForm == nullptr)
        return;

    auto selectedControls = this->selectedControls();
    selectedControls.removeOne(m_currentForm);

    //TODO: Raise selected controls

    auto itemUnderMouse = itemAt(event->scenePos(), QTransform());
    if (selectedControls.contains((Control*)itemUnderMouse))
        itemPressed = true;

    itemMoving = false;

    update();
}

void DesignerScene::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsScene::mouseMoveEvent(event);

    if (m_currentForm == nullptr)
        return;

    const SceneSettings* settings = DesignerSettings::sceneSettings();
    auto selectedControls = this->selectedControls();
    bool resizedAnyway = false; // NOTE: Might we use scene->mauseGrabberItem in a way?
    for (auto ctrl : selectedControls) {
        if (ctrl->beingResized())
            resizedAnyway = true;
    }
    selectedControls.removeOne(m_currentForm);

    if (m_currentForm && !selectedControls.isEmpty() && itemPressed && !resizedAnyway) {
        itemMoving = true;
        if (settings->snappingEnabled) {
            auto controlUnderMouse = (Control*)(itemAt(event->scenePos(), QTransform()));

            if (!controlUnderMouse)
                return;

            const QList<Control*> copy(selectedControls);
            for(Control* control : copy) {
                if (controlUnderMouse->parentControl() != control->parentControl()
                        || copy.contains(control->parentControl())) {
                    selectedControls.removeOne(control);
                    control->setSelected(false);
                }
            }

            //       FIXME     stick();
        }
    }

    m_lastMousePos = event->scenePos();

    QPointer<DesignerScene> p(this);
    Suppressor::suppress(100, "update", [=] {
        if (p)
            update();
    });
}

void DesignerScene::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsScene::mouseReleaseEvent(event);
    itemPressed = false;
    itemMoving = false;
}

void DesignerScene::drawForeground(QPainter* painter, const QRectF& rect)
{
    QGraphicsScene::drawForeground(painter, rect);

    auto selectedControls = this->selectedControls();
    bool resizedAnyway = false; // NOTE: Might we use scene->mauseGrabberItem in a way?
    for (auto ctrl : selectedControls) {
        if (ctrl->beingResized())
            resizedAnyway = true;
    }
    selectedControls.removeOne(m_currentForm);

    if ((itemMoving || resizedAnyway)/*&& m_snapping */&& m_currentForm != nullptr) {
        {
            const QList<Control*> copy(selectedControls);
            for (const Control* control : copy) {
                for (Control* childControl : control->childControls())
                    selectedControls.removeOne(childControl);
            }

            if (selectedControls.size() > 1) {
                auto r = united(selectedControls);
                paintOutline(painter, QRectF(selectedControls.first()->parentControl()->mapToScene(r.topLeft()), r.size()));
            }
        }

        const auto& guideLines = this->guideLines();
        painter->setPen(pen());
        painter->drawLines(guideLines);

        for (QLineF line : guideLines) {
            painter->setBrush(outlineColor());
            painter->drawRoundedRect(QRectF(line.p1() - QPointF(1.5, 1.5), QSizeF(3.0, 3.0)), 1.5, 1.5);
            painter->drawRoundedRect(QRectF(line.p2() - QPointF(1.5, 1.5), QSizeF(3.0, 3.0)), 1.5, 1.5);
        }
    }
}

void DesignerScene::paintOutline(QPainter* painter, const QRectF& rect)
{
    QPen linePen(QColor(0, 0, 0, 200));
    linePen.setCosmetic(true);
    linePen.setDashPattern({2., 1.});

    painter->setPen(linePen);
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(rect);

    linePen.setColor(QColor(255, 255, 255, 200));
    linePen.setDashPattern({1., 2.});
    linePen.setDashOffset(2.);

    painter->setPen(linePen);
    painter->drawRect(rect);
}

QPointF DesignerScene::lastMousePos() const
{
    return m_lastMousePos;
}

// FIXME: This function has severe performance issues.
QVector<QLineF> DesignerScene::guideLines() const
{
    QVector<QLineF> lines;
    auto selectedControls = this->selectedControls();
    selectedControls.removeOne(m_currentForm);

    const QList<Control*> copy(selectedControls);
    for (Control* control : copy) {
        if (copy.contains(control->parentControl()))
            selectedControls.removeOne(control);
    }

    if (selectedControls.isEmpty())
        return lines;

    const auto& parent = selectedControls.first()->parentControl();
    const auto& geometry = united(selectedControls);
    const auto& center = geometry.center();

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

    for (auto childControl : parent->childControls(false)) {
        if (selectedControls.contains(childControl))
            continue;

        auto cgeometry = childControl->geometry();
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

void DesignerScene::discharge()
{
    clearSelection();
    clear();

    m_forms.clear();
    m_currentForm.clear();
    m_lastMousePos = QPointF();

    itemPressed = false;
    itemMoving = false;
}

void DesignerScene::setCurrentForm(Form* currentForm)
{
    Q_D(QGraphicsScene);

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
        m_currentForm->setVisible(true);
        d->growingItemsBoundingRect = QRectF();
        emit sceneRectChanged(d->growingItemsBoundingRect);
    }
}

const QList<Form*>& DesignerScene::forms() const
{
    return m_forms;
}
