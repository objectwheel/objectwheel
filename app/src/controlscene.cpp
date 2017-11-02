#include <controlscene.h>
#include <fit.h>
#include <savemanager.h>

#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QDebug>

#define GUIDELINE_COLOR ("#0D80E7")
#define LINE_COLOR ("#606467")
#define NGCS_PANEL_WIDTH (fit(100))

using namespace Fit;

class ControlScenePrivate : public QObject
{
        Q_OBJECT

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

ControlScene::ControlScene(QObject *parent)
    : QGraphicsScene(parent)
    , _mainControl(nullptr)
    , _d(new ControlScenePrivate(this))
    , _snapping(true)
{
    connect(this, &ControlScene::changed, [=] {
        if (_mainControl)
            setSceneRect(mainControl()->frameGeometry().
              adjusted(-fit(8), -fit(8), fit(8), fit(8)));
    });
}

Control* ControlScene::mainControl()
{
    return _mainControl;
}

void ControlScene::setMainControl(Control* mainControl)
{
    if (_mainControl)
        removeItem(_mainControl);
    _mainControl = mainControl;
    addItem(_mainControl);
    emit mainControlChanged(mainControl);
}

void ControlScene::removeControl(Control* control)
{
    if (control == mainControl())
        return;
    for (auto ctrl : control->childControls())
        emit aboutToRemove(ctrl);
    emit aboutToRemove(control);
    removeItem(control);
    control->deleteLater();
    emit controlRemoved(control);
}

void ControlScene::removeChildControlsOnly(Control* parent)
{
    for (auto control : parent->childControls())
        emit aboutToRemove(control);
    for (auto control : parent->childControls())
        control->deleteLater();
    for (auto control : parent->childControls(false))
        removeItem(control);
}

QList<Control*> ControlScene::controls(Qt::SortOrder order)
{
    QList<Control*> controls;
    for (auto item : items(order))
        if (dynamic_cast<Control*>(item))
            controls << static_cast<Control*>(item);
    return controls;
}

QList<Control*> ControlScene::selectedControls()
{
    QList<Control*> selectedControls;
    for (auto item : selectedItems())
        if (dynamic_cast<Control*>(item))
            selectedControls << static_cast<Control*>(item);
    return selectedControls;
}

void ControlScene::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsScene::mousePressEvent(event);

    if (_mainControl == nullptr)
        return;

    auto selectedControls = this->selectedControls();
    selectedControls.removeOne(mainControl());

    for (auto control : selectedControls)
        control->setZValue(control->zValue() + 1);

    auto itemUnderMouse = itemAt(event->scenePos(), QTransform());
    if (selectedControls.contains((Control*)itemUnderMouse))
        _d->itemPressed = true;

    _d->itemMoving = false;

    for (auto control : mainControl()->childControls())
        control->setDragging(false);

    update();
}

void ControlScene::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsScene::mouseMoveEvent(event);

    if (_mainControl == nullptr)
        return;

    auto selectedControls = this->selectedControls();
    selectedControls.removeOne(mainControl());

    if (_mainControl && selectedControls.size() > 0 &&
        _d->itemPressed && !Resizer::resizing()) {
        _d->itemMoving = true;
        if (_snapping && selectedControls.size() == 1)
            _mainControl->stickSelectedControlToGuideLines();
    }

    if (_d->itemMoving)
        for (auto selectedControl : selectedControls)
            selectedControl->setDragging(true);

    _lastMousePos = event->scenePos();

    update();
}

void ControlScene::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    auto selectedControls = this->selectedControls();
    selectedControls.removeOne(mainControl());

    if (selectedControls.size() > 0)
        for (auto control : selectedControls)
            control->setZValue(control->zValue() - 1);

    QGraphicsScene::mouseReleaseEvent(event);
    _d->itemPressed = false;
    _d->itemMoving = false;

    if (_mainControl == nullptr)
        return;

    for (auto control : mainControl()->childControls())
        control->setDragging(false);

    update();
}

void ControlScene::drawForeground(QPainter* painter, const QRectF& rect)
{
    QGraphicsScene::drawForeground(painter, rect);
    painter->setRenderHint(QPainter::Antialiasing);

    if ((_d->itemMoving || Resizer::resizing()) && _snapping && _mainControl != nullptr) {
        auto guideLines = _mainControl->guideLines();
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


    if (_mainControl == nullptr) {
        QPen pen;
        QRectF rect(0, 0, fit(150), fit(60));
        rect.moveCenter(sceneRect().center());
        pen.setStyle(Qt::DotLine);
        pen.setColor(LINE_COLOR);
        painter->setPen(pen);
        painter->drawRect(rect);
        painter->drawText(rect, "No controls selected", QTextOption(Qt::AlignCenter));
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
    for (auto control : controls())
        control->update();
}

#include "controlscene.moc"

