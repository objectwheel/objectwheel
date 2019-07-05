#include <designerscene.h>
#include <suppressor.h>
#include <resizer.h>
#include <controlpropertymanager.h>
#include <designersettings.h>
#include <scenesettings.h>

#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QPointer>
#include <QPen>

#define NGCS_PANEL_WIDTH (100)
#define MAGNETIC_FIELD (3)

namespace {
static const char zValueProperty[] = "_q_DesignerScene_zValueProperty";

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

DesignerScene::DesignerScene(QObject *parent) : QGraphicsScene(parent)
  , m_currentForm(nullptr)
{
    connect(this, &DesignerScene::changed, [=] {
        if (m_currentForm)
            setSceneRect(m_currentForm->frameGeometry().adjusted(-8, -8, 8, 8));
    });
}

void DesignerScene::addForm(Form* form)
{
    if (m_forms.contains(form))
        return;

    addItem(form);
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

QList<Control*> DesignerScene::controlsAt(const QPointF& pos) const
{
    QList<Control*> controls;

    const QList<QGraphicsItem*>& itemsUnderPos = items(pos);
    for (QGraphicsItem* item : itemsUnderPos) {
        Control* control = dynamic_cast<Control*>(item);

        if (control)
            controls.append(control);
    }

    return controls;
}

QList<Control*> DesignerScene::selectedControls() const
{
    QList<Control*> selectedControls;
    for (auto item : selectedItems())
        if (dynamic_cast<Control*>(item))
            selectedControls << static_cast<Control*>(item);
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
        if (ctrl->resized())
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

            stick();
        }
    }

    if (itemMoving) {
        m_draggedControls = selectedControls;
        for (Control* selectedControl : selectedControls) {
            selectedControl->setProperty(zValueProperty, selectedControl->zValue());
            selectedControl->setZValue(std::numeric_limits<qreal>::max());
            selectedControl->setDragging(true);
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

    if (m_currentForm == nullptr)
        return;

    for (Control* draggedControl : m_draggedControls) {
        draggedControl->setZValue(draggedControl->property(zValueProperty).toReal());
        draggedControl->setDragging(false);
    }

    update();
}

void DesignerScene::drawForeground(QPainter* painter, const QRectF& rect)
{
    QGraphicsScene::drawForeground(painter, rect);

    auto selectedControls = this->selectedControls();
    bool resizedAnyway = false; // NOTE: Might we use scene->mauseGrabberItem in a way?
    for (auto ctrl : selectedControls) {
        if (ctrl->resized())
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
                QPen pen("#777777");
                pen.setWidthF(1);
                pen.setStyle(Qt::DotLine);
                painter->setPen(pen);
                painter->setBrush(Qt::NoBrush);
                auto r = united(selectedControls);
                painter->drawRect(QRectF(selectedControls.first()->parentControl()->mapToScene(r.topLeft()), r.size()));
            }
        }

        const auto& guideLines = this->guideLines();
        painter->setPen(highlightPen());
        painter->drawLines(guideLines);

        for (QLineF line : guideLines) {
            painter->setBrush(highlightPen().color());
            painter->drawRoundedRect(QRectF(line.p1() - QPointF(1.0, 1.0), QSizeF(2.0, 2.0)), 1.0, 1.0);
            painter->drawRoundedRect(QRectF(line.p2() - QPointF(1.0, 1.0), QSizeF(2.0, 2.0)), 1.0, 1.0);
        }
    }
}

QPointF DesignerScene::lastMousePos() const
{
    return m_lastMousePos;
}

// FIXME: This function has severe performance issues.
void DesignerScene::stick() const
{
    const SceneSettings* settings = DesignerSettings::sceneSettings();

    auto selectedControls = this->selectedControls();
    selectedControls.removeOne(m_currentForm);

    const QList<Control*> copy(selectedControls);
    for (Control* control : copy) {
        if (copy.contains(control->parentControl()))
            selectedControls.removeOne(control);
    }

    if (selectedControls.isEmpty())
        return;

    static const ControlPropertyManager::Options options = ControlPropertyManager::SaveChanges
            | ControlPropertyManager::UpdateRenderer
            | ControlPropertyManager::CompressedCall;

    const QRectF& frame = united(selectedControls);
    const qreal dx = int(frame.x() / settings->gridSize) * settings->gridSize - frame.x();
    const qreal dy = int(frame.y() / settings->gridSize) * settings->gridSize - frame.y();

    for (Control* control : selectedControls)
        ControlPropertyManager::setPos(control, {control->x() + dx, control->y() + dy}, options);
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
    if (int(center.y()) == int(parent->size().height() / 2.0))
        lines << QLineF(parent->mapToScene(center),
                        parent->mapToScene(QPointF(parent->size().width() / 2.0, center.y())));

    if (int(center.x()) == int(parent->size().width() / 2.0))
        lines << QLineF(parent->mapToScene(center),
                        parent->mapToScene(QPointF(center.x(), parent->size().height() / 2.0)));

    /* Child left <-> Parent center */
    if (int(geometry.x()) == int(parent->size().width() / 2.0))
        lines << QLineF(parent->mapToScene(QPointF(geometry.x(), center.y())),
                        parent->mapToScene(QPointF(geometry.x(), parent->size().height() / 2.0)));

    /* Child left <-> Parent left */
    if (int(geometry.x()) == 0)
        lines << QLineF(parent->mapToScene(QPointF(0, 0)),
                        parent->mapToScene(parent->rect().bottomLeft()));

    /* Child right <-> Parent center */
    if (int(geometry.topRight().x()) == int(parent->size().width() / 2.0))
        lines << QLineF(parent->mapToScene(QPointF(geometry.topRight().x(), center.y())),
                        parent->mapToScene(QPointF(geometry.topRight().x(), parent->size().height() / 2.0)));

    /* Child right <-> Parent right */
    if (int(geometry.topRight().x()) == int(parent->size().width()))
        lines << QLineF(parent->mapToScene(QPointF(parent->size().width(), 0)),
                        parent->mapToScene(QPointF(parent->size().width(), parent->size().height())));

    /* Child top <-> Parent center */
    if (int(geometry.y()) == int(parent->size().height() / 2.0))
        lines << QLineF(parent->mapToScene(QPointF(center.x(), parent->size().height() / 2.0)),
                        parent->mapToScene(QPointF(parent->size().width() / 2.0, parent->size().height() / 2.0)));

    /* Child top <-> Parent top */
    if (int(geometry.y()) == 0)
        lines << QLineF(parent->mapToScene(QPointF(0, 0)),
                        parent->mapToScene(QPointF(parent->size().width(), 0)));

    /* Child bottom <-> Parent center */
    if (int(geometry.bottomLeft().y()) == int(parent->size().height() / 2.0))
        lines << QLineF(parent->mapToScene(QPointF(center.x(), parent->size().height() / 2.0)),
                        parent->mapToScene(QPointF(parent->size().width() / 2.0, parent->size().height() / 2.0)));

    /* Child bottom <-> Parent bottom */
    if (int(geometry.bottomLeft().y()) == int(parent->size().height()))
        lines << QLineF(parent->mapToScene(QPointF(0, parent->size().height())),
                        parent->mapToScene(QPointF(parent->size().width(), parent->size().height())));

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

QPen DesignerScene::highlightPen()
{
    QPen pen(QBrush("#4ba2ff"), 1, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin);
    pen.setCosmetic(true);
    return pen;
}

QPen DesignerScene::nonCosmeticHighlightPen()
{
    QPen pen(QBrush("#4ba2ff"), 1, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin);
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

void DesignerScene::centralize()
{
    if (m_currentForm) {
        qreal x = -m_currentForm->size().width() / 2.0;
        qreal y = -m_currentForm->size().height() / 2.0;
        ControlPropertyManager::setPos(m_currentForm, {x, y}, ControlPropertyManager::NoOption);
    }
}

void DesignerScene::setCurrentForm(Form* currentForm)
{
    if (!m_forms.contains(currentForm) || m_currentForm == currentForm)
        return;

    /*
        NOTE: InspectorPane dependency: We prevent "selectionChanged" signal being emitted here
              Otherwise selectionChanged signal getting emitted before currentFormChanged signal,
              hence InspectorPane clears the selection before saving selection state of a form in
              currentFormChanged signal.
    */
    blockSignals(true);

    if (m_currentForm)
        m_currentForm->setVisible(false); // Clears selection and emits selectionChanged on DesignerScene

    m_currentForm = currentForm;
    m_currentForm->setVisible(true);
    centralize();

    blockSignals(false);

    emit currentFormChanged(m_currentForm);
}

const QList<Form*>& DesignerScene::forms() const
{
    return m_forms;
}
