#include <controlscene.h>
#include <fit.h>
#include <savemanager.h>

#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QDebug>

#define GUIDELINE_COLOR ("#0D80E7")

using namespace Fit;

class ControlScenePrivate : public QObject
{
        Q_OBJECT
        enum LockWay { NoLock, Vertical, Horizontal, Both };

    public:
        explicit ControlScenePrivate(ControlScene* parent);

    public:
        ControlScene* parent;
        bool itemPressed;
        bool itemMoving;
};

ControlScenePrivate::ControlScenePrivate(ControlScene* parent)
    : QObject(parent)
    , parent(parent)
    , itemPressed(false)
    , itemMoving(false)
{
}

ControlScenePrivate* ControlScene::_d = nullptr;
QPointer<Control> ControlScene::_currentControl = nullptr;
bool ControlScene::_snapping = true;
QPointF ControlScene::_lastMousePos;

ControlScene::ControlScene(QObject *parent)
    : QGraphicsScene(parent)
{
    if (_d)
        return;
    _d = new ControlScenePrivate(this);

    connect(this, &ControlScene::changed, [=] {
        if (_currentControl)
            setSceneRect(currentControl()->frameGeometry().adjusted(-fit(8), -fit(8), fit(8), fit(8)));
    });
}

ControlScene* ControlScene::instance()
{
    return _d->parent;
}

Control* ControlScene::currentControl()
{
    return _currentControl;
}

void ControlScene::setCurrentControl(Control* currentControl)
{
    if (_currentControl)
        _d->parent->removeItem(_currentControl);

    _currentControl = currentControl;
    _d->parent->addItem(_currentControl);
}

void ControlScene::removeControl(Control* control)
{
    SaveManager::removeSave(control->id());
    ControlScene::instance()->removeItem(control);
}

void ControlScene::removeChildControlsOnly(Control* parent)
{
    SaveManager::removeChildSavesOnly(parent->id());

    for (auto control : parent->childControls())
        ControlScene::instance()->removeItem(control);
}

QList<Control*> ControlScene::controls(Qt::SortOrder order)
{
    QList<Control*> controls;
    for (auto item : _d->parent->items(order)) {
        if (dynamic_cast<Control*>(item) && !dynamic_cast<Control*>(item)) {
            controls << static_cast<Control*>(item);
        }
    }
    return controls;
}

QList<Control*> ControlScene::selectedControls()
{
    QList<Control*> selectedControls;
    for (auto item : _d->parent->selectedItems()) {
        if (dynamic_cast<Control*>(item) && !dynamic_cast<Control*>(item)) {
            selectedControls << static_cast<Control*>(item);
        }
    }
    return selectedControls;
}

void ControlScene::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsScene::mousePressEvent(event);

    auto selectedControls = this->selectedControls();
    for (auto control : selectedControls)
        if (_currentControl->higherZValue() != control->zValue())
            control->setZValue(_currentControl->higherZValue() == -MAX_Z_VALUE
                               ? 0 : _currentControl->higherZValue() + 1);

    auto itemUnderMouse = itemAt(event->scenePos(), QTransform());
    if (this->selectedControls().contains((Control*)itemUnderMouse))
        _d->itemPressed = true;

    _d->itemMoving = false;

    for (auto control : currentControl()->childControls())
        control->setDragging(false);

    update();
}

void ControlScene::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsScene::mouseMoveEvent(event);

    if (_currentControl && selectedControls().size() > 0 &&
        _d->itemPressed && !Resizer::resizing()) {
        _d->itemMoving = true;
        if (_snapping && selectedControls().size() == 1)
            _currentControl->stickSelectedControlToGuideLines();
    }

    if (_d->itemMoving) {
        for (auto selectedControl : selectedControls()) {
            selectedControl->setDragging(true);
        }
    }

    _lastMousePos = event->scenePos();

    update();
}

void ControlScene::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsScene::mouseReleaseEvent(event);
    _d->itemPressed = false;
    _d->itemMoving = false;

    for (auto control : currentControl()->childControls())
        control->setDragging(false);

    update();
}

void ControlScene::drawForeground(QPainter* painter, const QRectF& rect)
{
    QGraphicsScene::drawForeground(painter, rect);

    if ((_d->itemMoving || Resizer::resizing()) && _snapping) {
        auto guideLines = _currentControl->guideLines();
        QPen pen(GUIDELINE_COLOR);
        pen.setWidthF(1.0);
        pen.setJoinStyle(Qt::RoundJoin);
        pen.setCapStyle(Qt::RoundCap);
        pen.setStyle(Qt::DashLine);
        painter->setPen(pen);
        painter->setBrush(pen.color());
        painter->drawLines(guideLines);

        for (QLineF line : guideLines) {
            pen.setStyle(Qt::SolidLine);
            painter->setPen(pen);
            painter->drawRoundedRect(QRectF(line.p1() - QPointF(1.0, 1.0), QSizeF(2.0, 2.0)), 1.0, 1.0);
            painter->drawRoundedRect(QRectF(line.p2() - QPointF(1.0, 1.0), QSizeF(2.0, 2.0)), 1.0, 1.0);
        }
    }
}

QPointF ControlScene::lastMousePos()
{
    return _lastMousePos;
}

bool ControlScene::snapping()
{
    return _snapping;
}

void ControlScene::setSnapping(bool snapping)
{
    _snapping = snapping;
}

bool ControlScene::showOutlines()
{
    return Control::showOutline();;
}

void ControlScene::setShowOutlines(bool value)
{
    Control::setShowOutline(value);
    for (auto control : controls()) {
        control->update();
    }
}

#include "controlscene.moc"

