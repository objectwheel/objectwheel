#include <controlscene.h>
#include <fit.h>
#include <savebackend.h>
#include <suppressor.h>

#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QPointer>
#include <QDebug>

#define GUIDELINE_COLOR ("#0D80E7")
#define LINE_COLOR ("#606467")
#define NGCS_PANEL_WIDTH (fit::fx(100))
#define MAGNETIC_FIELD (fit::fx(3))

class ControlScenePrivate : public QObject
{
        Q_OBJECT

    public:
        explicit ControlScenePrivate(ControlScene* parent);
        QRectF united(const QList<Control*> controls);

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

QRectF ControlScenePrivate::united(const QList<Control*> controls)
{
    QRectF rect = controls.first()->geometry();
    for (auto control : controls)
        if (controls.first() != control)
            rect |= QRectF(controls.first()->parentControl()->mapFromItem
              (control->parentControl(), control->pos()), control->size());
    return rect;
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
              adjusted(-fit::fx(8), -fit::fx(8), fit::fx(8), fit::fx(8)));
    });
}

Control* ControlScene::mainControl() const
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

QList<Control*> ControlScene::selectedControls() const
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

    //TODO: Raise selected controls

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

    if (_mainControl && !selectedControls.isEmpty() &&
        _d->itemPressed && !Resizer::resizing()) {
        _d->itemMoving = true;
        if (_snapping) {
            auto controlUnderMouse = (Control*)(itemAt
              (event->scenePos(), QTransform()));

            for(int i = 0; i < selectedControls.size(); i++) {
                auto control = selectedControls[i];
                if (!control->gui() || controlUnderMouse->parentControl()
                 != control->parentControl() ||
                  selectedControls.
                   contains(control->parentControl())) {
                    selectedControls.removeOne(control);
                    control->setSelected(false);
                    i--;
                }
            }

            stick();
        }
    }

    if (_d->itemMoving)
        for (auto selectedControl : selectedControls)
            selectedControl->setDragging(true);

    _lastMousePos = event->scenePos();

    QPointer<ControlScene> p(this);
    Suppressor::suppress(100, "update", [=] {
        if (p)
            update();
    });
}

void ControlScene::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
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
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    if ((_d->itemMoving || Resizer::resizing())/*&& _snapping */&& _mainControl != nullptr) {
        {
            auto selectedControls = this->selectedControls();
            selectedControls.removeOne(mainControl());

            for(int i = 0; i < selectedControls.size(); i++) {
                auto control = selectedControls[i];
                if (!control->gui() || selectedControls.
                  contains(control->parentControl())) {
                    selectedControls.removeOne(control);
                    i--;
                }
            }

            if (selectedControls.size() > 1) {
                QPen pen("#404447");
                pen.setWidthF(fit::fx(1));
                pen.setStyle(Qt::DotLine);
                painter->setPen(pen);
                painter->setBrush(Qt::NoBrush);
                auto r = _d->united(selectedControls);
                painter->drawRect(QRectF(selectedControls.first()->parentControl()->mapToScene(r.topLeft()), r.size()));
            }
        }

        const auto& guideLines = this->guideLines();
        painter->setPen(GUIDELINE_COLOR);
        painter->drawLines(guideLines);

        for (QLineF line : guideLines) {
            painter->setBrush(QColor(GUIDELINE_COLOR));
            painter->drawRoundedRect(QRectF(line.p1() - QPointF(1.0, 1.0), QSizeF(2.0, 2.0)), 1.0, 1.0);
            painter->drawRoundedRect(QRectF(line.p2() - QPointF(1.0, 1.0), QSizeF(2.0, 2.0)), 1.0, 1.0);
        }
    }

    if (_mainControl == nullptr) {
        QPen pen;
        pen.setWidthF(fit::fx(1));
        QRectF rect(0, 0, fit::fx(150), fit::fx(60));
        rect.moveCenter(sceneRect().center());
        pen.setStyle(Qt::DotLine);
        pen.setColor(LINE_COLOR);
        painter->setPen(pen);
        painter->drawText(rect, "No tools selected", QTextOption(Qt::AlignCenter));
    }
    painter->restore();
}

QPointF ControlScene::lastMousePos() const
{
    return _lastMousePos;
}

void ControlScene::clearScene()
{
    clear();
    _mainControl.clear();
    _d->itemPressed = false;
    _d->itemMoving = false;
}

bool ControlScene::snapping() const
{
    return _snapping;
}

void ControlScene::setSnapping(bool snapping)
{
    _snapping = snapping;
}

bool ControlScene::showOutlines() const
{
    return Control::showOutline();;
}

void ControlScene::setShowOutlines(bool value)
{
    Control::setShowOutline(value);
    if (mainControl())
        mainControl()->update();
}

bool ControlScene::stick() const
{
    bool ret = false;
    auto selectedControls = this->selectedControls();
    selectedControls.removeOne(mainControl());

    for(int i = 0; i < selectedControls.size(); i++) {
        auto control = selectedControls[i];
        if (!control->gui() || selectedControls.
          contains(control->parentControl())) {
            selectedControls.removeOne(control);
            i--;
        }
    }

    if (selectedControls.isEmpty())
        return ret;

    const auto& parent = selectedControls.first()->parentControl();
    auto geometry = _d->united(selectedControls);
    auto center = geometry.center();

    /* Child center <-> Parent center */
    if (center.y() <= parent->size().height() / 2.0 + MAGNETIC_FIELD &&
        center.y() >= parent->size().height() / 2.0 - MAGNETIC_FIELD) {
        auto g = geometry;
        geometry.moveCenter({center.x(), parent->size().height() / 2.0});
        center = geometry.center();
        for (auto control : selectedControls)
            control->setPos(control->pos() + geometry.topLeft() - g.topLeft());
        ret = true;
    }

    if (center.x() <= parent->size().width() / 2.0 + MAGNETIC_FIELD &&
        center.x() >= parent->size().width() / 2.0 - MAGNETIC_FIELD) {
        auto g = geometry;
        geometry.moveCenter({parent->size().width() / 2.0, center.y()});
        center = geometry.center();
        for (auto control : selectedControls)
            control->setPos(control->pos() + geometry.topLeft() - g.topLeft());
        ret = true;
    }

    /* Child left <-> Parent center */
    if (geometry.topLeft().x() <= parent->size().width() / 2.0 + MAGNETIC_FIELD &&
        geometry.topLeft().x() >= parent->size().width() / 2.0 - MAGNETIC_FIELD) {
        auto g = geometry;
        geometry.moveLeft(parent->size().width() / 2.0);
        center = geometry.center();
        for (auto control : selectedControls)
            control->setPos(control->pos() + geometry.topLeft() - g.topLeft());
        ret = true;
    }

    /* Child left <-> Parent left */
    if (geometry.topLeft().x() <= MAGNETIC_FIELD &&
        geometry.topLeft().x() >= - MAGNETIC_FIELD) {
        auto g = geometry;
        geometry.moveLeft(0);
        center = geometry.center();
        for (auto control : selectedControls)
            control->setPos(control->pos() + geometry.topLeft() - g.topLeft());
        ret = true;
    }

    /* Child right <-> Parent center */
    if (geometry.topRight().x() <= parent->size().width() / 2.0 + MAGNETIC_FIELD &&
        geometry.topRight().x() >= parent->size().width() / 2.0 - MAGNETIC_FIELD) {
        auto g = geometry;
        geometry.moveRight(parent->size().width() / 2.0);
        center = geometry.center();
        for (auto control : selectedControls)
            control->setPos(control->pos() + geometry.topLeft() - g.topLeft());
        ret = true;
    }

    /* Child right <-> Parent right */
    if (geometry.topRight().x() <= parent->size().width() + MAGNETIC_FIELD &&
        geometry.topRight().x() >= parent->size().width() - MAGNETIC_FIELD) {
        auto g = geometry;
        geometry.moveRight(parent->size().width());
        center = geometry.center();
        for (auto control : selectedControls)
            control->setPos(control->pos() + geometry.topLeft() - g.topLeft());
        ret = true;
    }

    /* Child top <-> Parent center */
    if (geometry.y() <= parent->size().height() / 2.0 + MAGNETIC_FIELD &&
        geometry.y() >= parent->size().height() / 2.0 - MAGNETIC_FIELD) {
        auto g = geometry;
        geometry.moveTop(parent->size().height() / 2.0);
        center = geometry.center();
        for (auto control : selectedControls)
            control->setPos(control->pos() + geometry.topLeft() - g.topLeft());
        ret = true;
    }

    /* Child top <-> Parent top */
    if (geometry.y() <= MAGNETIC_FIELD &&
        geometry.y() >= - MAGNETIC_FIELD) {
        auto g = geometry;
        geometry.moveTop(0);
        center = geometry.center();
        for (auto control : selectedControls)
            control->setPos(control->pos() + geometry.topLeft() - g.topLeft());
        ret = true;
    }

    /* Child bottom <-> Parent center */
    if (geometry.bottomLeft().y() <= parent->size().height() / 2.0 + MAGNETIC_FIELD &&
        geometry.bottomLeft().y() >= parent->size().height() / 2.0 - MAGNETIC_FIELD) {
        auto g = geometry;
        geometry.moveBottom(parent->size().height() / 2.0);
        center = geometry.center();
        for (auto control : selectedControls)
            control->setPos(control->pos() + geometry.topLeft() - g.topLeft());
        ret = true;
    }

    /* Child bottom <-> Parent bottom */
    if (geometry.bottomLeft().y() <= parent->size().height() + MAGNETIC_FIELD &&
        geometry.bottomLeft().y() >= parent->size().height() - MAGNETIC_FIELD) {
        auto g = geometry;
        geometry.moveBottom(parent->size().height());
        center = geometry.center();
        for (auto control : selectedControls)
            control->setPos(control->pos() + geometry.topLeft() - g.topLeft());
        ret = true;
    }

    for (auto childControl : parent->childControls(false)) {
        if (selectedControls.contains(childControl) || !childControl->gui())
            continue;

        auto cgeometry = childControl->geometry();
        auto ccenter = cgeometry.center();

        /* Item1 center <-> Item2 center */
        if (center.x() <= ccenter.x() + MAGNETIC_FIELD &&
            center.x() >= ccenter.x() - MAGNETIC_FIELD) {
            auto g = geometry;
            geometry.moveCenter({ccenter.x(), center.y()});
            center = geometry.center();
            for (auto control : selectedControls)
                control->setPos(control->pos() + geometry.topLeft() - g.topLeft());
            ret = true;
        }

        if (center.y() <= ccenter.y() + MAGNETIC_FIELD &&
            center.y() >= ccenter.y() - MAGNETIC_FIELD) {
            auto g = geometry;
            geometry.moveCenter({center.x(), ccenter.y()});
            center = geometry.center();
            for (auto control : selectedControls)
                control->setPos(control->pos() + geometry.topLeft() - g.topLeft());
            ret = true;
        }

        /* Item1 center <-> Item2 left */
        if (center.x() <= cgeometry.topLeft().x() + MAGNETIC_FIELD &&
            center.x() >= cgeometry.topLeft().x() - MAGNETIC_FIELD) {
            auto g = geometry;
            geometry.moveCenter({cgeometry.topLeft().x(), center.y()});
            center = geometry.center();
            for (auto control : selectedControls)
                control->setPos(control->pos() + geometry.topLeft() - g.topLeft());
            ret = true;
        }

        /* Item1 center <-> Item2 top */
        if (center.y() <= cgeometry.topLeft().y() + MAGNETIC_FIELD &&
            center.y() >= cgeometry.topLeft().y() - MAGNETIC_FIELD) {
            auto g = geometry;
            geometry.moveCenter({center.x(), cgeometry.topLeft().y()});
            center = geometry.center();
            for (auto control : selectedControls)
                control->setPos(control->pos() + geometry.topLeft() - g.topLeft());
            ret = true;
        }

        /* Item1 center <-> Item2 right */
        if (center.x() <= cgeometry.bottomRight().x() + MAGNETIC_FIELD &&
            center.x() >= cgeometry.bottomRight().x() - MAGNETIC_FIELD) {
            auto g = geometry;
            geometry.moveCenter({cgeometry.bottomRight().x(), center.y()});
            center = geometry.center();
            for (auto control : selectedControls)
                control->setPos(control->pos() + geometry.topLeft() - g.topLeft());
            ret = true;
        }

        /* Item1 center <-> Item2 bottom */
        if (center.y() <= cgeometry.bottomRight().y() + MAGNETIC_FIELD &&
            center.y() >= cgeometry.bottomRight().y() - MAGNETIC_FIELD) {
            auto g = geometry;
            geometry.moveCenter({center.x(), cgeometry.bottomRight().y()});
            center = geometry.center();
            for (auto control : selectedControls)
                control->setPos(control->pos() + geometry.topLeft() - g.topLeft());
            ret = true;
        }

        /* Item1 left <-> Item2 left */
        if (geometry.x() <= cgeometry.x() + MAGNETIC_FIELD &&
            geometry.x() >= cgeometry.x() - MAGNETIC_FIELD) {
            auto g = geometry;
            geometry.moveTopLeft({cgeometry.x(), geometry.y()});
            center = geometry.center();
            for (auto control : selectedControls)
                control->setPos(control->pos() + geometry.topLeft() - g.topLeft());
            ret = true;
        }

        /* Item1 left <-> Item2 center */
        if (geometry.x() <= ccenter.x() + MAGNETIC_FIELD &&
            geometry.x() >= ccenter.x() - MAGNETIC_FIELD) {
            auto g = geometry;
            geometry.moveTopLeft({ccenter.x(), geometry.y()});
            center = geometry.center();
            for (auto control : selectedControls)
                control->setPos(control->pos() + geometry.topLeft() - g.topLeft());
            ret = true;
        }

        /* Item1 left <-> Item2 right */
        if (geometry.x() <= cgeometry.topRight().x() + MAGNETIC_FIELD &&
            geometry.x() >= cgeometry.topRight().x() - MAGNETIC_FIELD) {
            auto g = geometry;
            geometry.moveTopLeft({cgeometry.topRight().x(), geometry.y()});
            center = geometry.center();
            for (auto control : selectedControls)
                control->setPos(control->pos() + geometry.topLeft() - g.topLeft());
            ret = true;
        }

        /* Item1 right <-> Item2 left */
        if (geometry.topRight().x() <= cgeometry.x() + MAGNETIC_FIELD &&
            geometry.topRight().x() >= cgeometry.x() - MAGNETIC_FIELD) {
            auto g = geometry;
            geometry.moveTopRight({cgeometry.x(), geometry.y()});
            center = geometry.center();
            for (auto control : selectedControls)
                control->setPos(control->pos() + geometry.topLeft() - g.topLeft());
            ret = true;
        }

        /* Item1 right <-> Item2 center */
        if (geometry.topRight().x() <= ccenter.x() + MAGNETIC_FIELD &&
            geometry.topRight().x() >= ccenter.x() - MAGNETIC_FIELD) {
            auto g = geometry;
            geometry.moveTopRight({ccenter.x(), geometry.y()});
            center = geometry.center();
            for (auto control : selectedControls)
                control->setPos(control->pos() + geometry.topLeft() - g.topLeft());
            ret = true;
        }

        /* Item1 right <-> Item2 right */
        if (geometry.topRight().x() <= cgeometry.topRight().x() + MAGNETIC_FIELD &&
            geometry.topRight().x() >= cgeometry.topRight().x() - MAGNETIC_FIELD) {
            auto g = geometry;
            geometry.moveTopRight({cgeometry.topRight().x(), geometry.y()});
            center = geometry.center();
            for (auto control : selectedControls)
                control->setPos(control->pos() + geometry.topLeft() - g.topLeft());
            ret = true;
        }

        /* Item1 top <-> Item2 top */
        if (geometry.y() <= cgeometry.y() + MAGNETIC_FIELD &&
            geometry.y() >= cgeometry.y() - MAGNETIC_FIELD) {
            auto g = geometry;
            geometry.moveTopLeft({geometry.x(), cgeometry.y()});
            center = geometry.center();
            for (auto control : selectedControls)
                control->setPos(control->pos() + geometry.topLeft() - g.topLeft());
            ret = true;
        }

        /* Item1 top <-> Item2 center */
        if (geometry.y() <= ccenter.y() + MAGNETIC_FIELD &&
            geometry.y() >= ccenter.y() - MAGNETIC_FIELD) {
            auto g = geometry;
            geometry.moveTopLeft({geometry.x(), ccenter.y()});
            center = geometry.center();
            for (auto control : selectedControls)
                control->setPos(control->pos() + geometry.topLeft() - g.topLeft());
            ret = true;
        }

        /* Item1 top <-> Item2 bottom */
        if (geometry.y() <= cgeometry.bottomLeft().y() + MAGNETIC_FIELD &&
            geometry.y() >= cgeometry.bottomLeft().y() - MAGNETIC_FIELD) {
            auto g = geometry;
            geometry.moveTopLeft({geometry.x(), cgeometry.bottomLeft().y()});
            center = geometry.center();
            for (auto control : selectedControls)
                control->setPos(control->pos() + geometry.topLeft() - g.topLeft());
            ret = true;
        }

        /* Item1 bottom <-> Item2 top */
        if (geometry.bottomLeft().y() <= cgeometry.y() + MAGNETIC_FIELD &&
            geometry.bottomLeft().y() >= cgeometry.y() - MAGNETIC_FIELD) {
            auto g = geometry;
            geometry.moveBottomLeft({geometry.x(), cgeometry.y()});
            center = geometry.center();
            for (auto control : selectedControls)
                control->setPos(control->pos() + geometry.topLeft() - g.topLeft());
            ret = true;
        }

        /* Item1 bottom <-> Item2 center */
        if (geometry.bottomLeft().y() <= ccenter.y() + MAGNETIC_FIELD &&
            geometry.bottomLeft().y() >= ccenter.y() - MAGNETIC_FIELD) {
            auto g = geometry;
            geometry.moveBottomLeft({geometry.x(), ccenter.y()});
            center = geometry.center();
            for (auto control : selectedControls)
                control->setPos(control->pos() + geometry.topLeft() - g.topLeft());
            ret = true;
        }

        /* Item1 bottom <-> Item2 bottom */
        if (geometry.bottomLeft().y() <= cgeometry.bottomLeft().y() + MAGNETIC_FIELD &&
            geometry.bottomLeft().y() >= cgeometry.bottomLeft().y() - MAGNETIC_FIELD) {
            auto g = geometry;
            geometry.moveBottomLeft({geometry.x(), cgeometry.bottomLeft().y()});
            center = geometry.center();
            for (auto control : selectedControls)
                control->setPos(control->pos() + geometry.topLeft() - g.topLeft());
            ret = true;
        }
    }

    return ret;
}

QVector<QLineF> ControlScene::guideLines() const
{
    QVector<QLineF> lines;
    auto selectedControls = this->selectedControls();
    selectedControls.removeOne(mainControl());

    for(int i = 0; i < selectedControls.size(); i++) {
        auto control = selectedControls[i];
        if (!control->gui() || selectedControls.
          contains(control->parentControl())) {
            selectedControls.removeOne(control);
            i--;
        }
    }

    if (selectedControls.isEmpty())
        return lines;

    const auto& parent = selectedControls.first()->parentControl();
    const auto& geometry = _d->united(selectedControls);
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
        if (selectedControls.contains(childControl) || !childControl->gui())
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

#include "controlscene.moc"

