#include <control.h>
#include <fit.h>
#include <random>
#include <filemanager.h>
#include <controlwatcher.h>
#include <suppressor.h>
#include <savemanager.h>
#include <controlscene.h>
#include <formscene.h>

#include <QtMath>
#include <QtWidgets>
#include <QtSvg>

#define TOOLBOX_ITEM_KEY "QURBUEFaQVJMSVlJWiBIQUZJWg"
#define HIGHLIGHT_COLOR (QColor("#174C4E4D"))
#define SELECTION_COLOR ("#404447")
#define OUTLINE_COLOR ("#808487")
#define BACKGROUND_COLOR (QColor("#F0F4F7"))
#define PREVIEW_REFRESH_INTERVAL 100
#define RESIZE_TRANSACTION_INTERVAL 800
#define GEOMETRY_SIGNAL_DELAY 800
#define MAGNETIC_FIELD (fit(3))
#define MARGIN_TOP (fit(14))
#define pS (QApplication::primaryScreen())
#define cW (ControlWatcher::instance())

using namespace Fit;

//!
//! ************************* [global] **************************
//!

static std::random_device rd;
static std::mt19937 mt(rd());
static std::uniform_int_distribution<qint32> rand_dist(-2147483648, 2147483647);

//!
//! ****************** [Control Private] ******************
//!

class ControlPrivate : public QObject
{
        Q_OBJECT

    public:
        ControlPrivate(Control* parent);

    public slots:
        void updatePreview(PreviewResult result);

    public:
        Control* parent;
        QPixmap preview;
        bool hoverOn;
};

ControlPrivate::ControlPrivate(Control* parent)
    : QObject(parent)
    , parent(parent)
    , hoverOn(false)
{
    int i = 0;
    for (auto& resizer : parent->_resizers) {
        resizer.setParentItem(parent);
        resizer.setPlacement(Resizer::Placement(i++));
    }

    connect(QmlPreviewer::instance(),
      SIGNAL(previewReady(PreviewResult)),
      SLOT(updatePreview(PreviewResult)));
}

void ControlPrivate::updatePreview(PreviewResult result)
{
    if (result.control != parent)
        return;

    parent->_errors = result.errors;
    preview = result.preview;
    parent->_gui = result.gui;
    parent->_properties = result.properties;
    parent->_events = result.events;

    if (!result.hasError()) {
        if (result.gui) {
            if (!parent->form())
                parent->_clip = result.property("clip").toBool();
        }
        parent->setVisible(result.gui);
    }

    parent->update();
    if (result.hasError()) {
        emit parent->errorOccurred();
        emit cW->errorOccurred(parent);
    }
    emit parent->previewChanged();
    emit cW->previewChanged(parent);
}

//!
//! ********************** [Control] **********************
//!

bool Control::_showOutline = false;
QList<Control*> Control::_controls;

Control::Control(const QString& url, const DesignMode& mode,
  const QString& uid, Control* parent)
    : QGraphicsWidget(parent)
    , _clip(true)
    , _d(new ControlPrivate(this))
    , _uid(uid.isEmpty() ? SaveManager::uid(dname(dname(url))) : uid)
    , _url(url)
    , _mode(mode)
    , _dragging(false)
    , _dragIn(false)
    , _gui(true)
{
    _controls << this;

    setFlag(Control::ItemIsFocusable);
    setFlag(Control::ItemIsSelectable);
    setFlag(Control::ItemSendsGeometryChanges);
    setFlag(Control::ItemIsMovable);
    setAcceptHoverEvents(true);
    setAcceptDrops(true);

    setZValue(SaveManager::z(dir()));
    setId(SaveManager::id(dname(dname(url))));
    setPos(SaveManager::x(dir()), SaveManager::y(dir()));
    resize(SaveManager::width(dir()), SaveManager::height(dir()));

    connect(this, &Control::visibleChanged, [=] {
        if (size().width() < 2 || size().height() < 2)
            refresh();
    });

    connect(this, &Control::geometryChanged, [=] {
        Suppressor::suppress(GEOMETRY_SIGNAL_DELAY, "geometryChanged",
          std::bind(&ControlWatcher::geometryChanged, cW, this));
    });

    connect(this, &Control::zChanged, [this]{
        emit cW->zValueChanged(this);
    });

    connect(this, &Control::parentChanged, [this]{
        emit cW->parentChanged(this);
    });
}

Control::~Control()
{
    _controls.removeOne(this);
}

bool Control::showOutline()
{
    return _showOutline;
}

void Control::setShowOutline(const bool value)
{
    _showOutline = value;
}

void Control::updateUids()
{
    for (auto control : _controls)
        control->updateUid();
}

QString Control::generateUid()
{
    QByteArray uidData;
    auto randNum = rand_dist(mt);
    auto randNum1 = rand_dist(mt);
    auto randNum2 = rand_dist(mt);
    auto dateMs = QDateTime::currentMSecsSinceEpoch();
    uidData.insert(0, QString::number(dateMs));
    uidData.insert(0, QString::number(randNum));
    uidData.insert(0, QString::number(randNum1));
    uidData.insert(0, QString::number(randNum2));
    return QCryptographicHash::hash(uidData, QCryptographicHash::Sha256).toHex();
}

QList<Control*> Control::childControls(bool dive) const
{
    QList<Control*> controls;
    for (auto item : childItems()) {
        if (dynamic_cast<Control*>(item)) {
            controls << static_cast<Control*>(item);
            if (dive)
                controls << controls.last()->childControls(dive);
        }
    }
    return controls;
}

void Control::hideResizers()
{
    for (auto& resizer : _resizers) {
        resizer.hide();
    }
}

void Control::showResizers()
{
    for (auto& resizer : _resizers) {
        resizer.show();
    }
}

Control* Control::parentControl() const
{
    return dynamic_cast<Control*>(parentItem());
}

QString Control::id() const
{
    return _id;
}

void Control::setId(const QString& id)
{
    _id = id;
    setToolTip(id);
}

QString Control::url() const
{
    return _url;
}

bool Control::contains(const QString& id) const
{
    for (auto control : childControls()) {
        if (control->id() == id)
            return true;
    }
    return false;
}

bool Control::form() const
{
    return (dynamic_cast<const Form*>(this) != nullptr);
}

void Control::refresh()
{
    QmlPreviewer::requestPreview(this);
}

void Control::updateUid()
{
    _uid = SaveManager::uid(dir());
}

void Control::centralize()
{
    setPos(-size().width() / 2.0, -size().height() / 2.0);
}

QRectF Control::frameGeometry() const
{
    return QRectF({QPointF(-size().width() / 2.0, -size().height() / 2.0), size()});
}

void Control::dragEnterEvent(QGraphicsSceneDragDropEvent* event)
{
    auto mimeData = event->mimeData();
    if (mimeData->hasUrls() && mimeData->hasText() &&
        mimeData->text() == TOOLBOX_ITEM_KEY) {
        _dragIn = true;
        event->accept();
    } else {
        event->ignore();
    }
    update();
}

void Control::dragLeaveEvent(QGraphicsSceneDragDropEvent* event)
{
    _dragIn = false;
    event->accept();
    update();
}

void Control::dragMoveEvent(QGraphicsSceneDragDropEvent* event)
{
    event->accept();
}

void Control::dropEvent(QGraphicsSceneDragDropEvent* event)
{
    _dragIn = false;
    event->accept();
    emit controlDropped(event->pos(),
      event->mimeData()->urls().first().toLocalFile());
    emit cW->controlDropped(this, event->pos(),
      event->mimeData()->urls().first().toLocalFile());
    update();
}

void Control::dropControl(Control* control)
{
    control->setPos(mapFromItem(control->
      parentItem(), control->pos()));
    control->setParentItem(this);
    update();
}

void Control::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
{
    QGraphicsWidget::hoverEnterEvent(event);
    _d->hoverOn = true;
    update();
}

void Control::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
    QGraphicsWidget::hoverLeaveEvent(event);
    _d->hoverOn = false;
    update();
}

void Control::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsWidget::mousePressEvent(event);

    if (event->button() == Qt::MidButton)
        event->ignore();
    else
        event->accept();
}

void Control::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsWidget::mouseMoveEvent(event);

    Control* control = nullptr;
    auto items = scene()->items(event->scenePos());
    if (_dragging && items.size() > 1 && (control = dynamic_cast<Control*>(items[1])) &&
        control != this) {
        control->setDragIn(true);

        auto scene = static_cast<ControlScene*>(this->scene());
        for (auto c : scene->mainControl()->childControls())
            if (c != control)
                c->setDragIn(false);

        if (scene->mainControl() != control)
            scene->mainControl()->setDragIn(false);
    }

    if (event->button() == Qt::MidButton)
        event->ignore();
    else
        event->accept();
}

void Control::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsWidget::mouseReleaseEvent(event);

    auto scene = static_cast<ControlScene*>(this->scene());

    for (auto control : scene->mainControl()->childControls()) {
        if (control->dragIn() && dragging() &&
            parentControl() != control) {
            control->dropControl(this);
            scene->clearSelection();
            control->setSelected(true);
        }
        control->setDragIn(false);
    }

    if (scene->mainControl()->dragIn() && dragging() &&
        parentControl() != scene->mainControl()) {
        scene->mainControl()->dropControl(this);
        scene->clearSelection();
        scene->mainControl()->setSelected(true);
    }
    scene->mainControl()->setDragIn(false);

    event->accept();
}

void Control::mouseDoubleClickEvent(QGraphicsSceneMouseEvent*)
{
    emit doubleClicked();
    emit cW->doubleClicked(this);
}

void Control::resizeEvent(QGraphicsSceneResizeEvent* event)
{
    QGraphicsWidget::resizeEvent(event);
    for (auto& resizer : _resizers)
        resizer.correct();
}

QVariant Control::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant& value)
{
    switch (change) {
        case ItemSelectedHasChanged:
            if (value.toBool())
                showResizers();
            else
                hideResizers();
            break;
        default:
            break;
    }
    return QGraphicsWidget::itemChange(change, value);
}

const DesignMode& Control::mode() const
{
    return _mode;
}

const QList<QQmlError>& Control::errors() const
{
    return _errors;
}

bool Control::hasErrors() const
{
    return !_errors.isEmpty();
}

const QList<Control*>& Control::controls()
{
    return _controls;
}

void Control::setUrl(const QString& url)
{
    _url = url;
}

QString Control::dir() const // Returns root path
{
    return dname(dname(_url));
}

QString Control::uid() const
{
    return _uid;
}

bool Control::gui() const
{
    return _gui;
}

const QList<QString>& Control::events() const
{
    return _events;
}

const PropertyNodes& Control::properties() const
{
    return _properties;
}

bool Control::clip() const
{
    return _clip;
}

bool Control::dragIn() const
{
    return _dragIn;
}

void Control::setDragIn(bool dragIn)
{
    _dragIn = dragIn;
}

bool Control::dragging() const
{
    return _dragging;
}

void Control::setDragging(bool dragging)
{
    _dragging = dragging;

    if (dragging)
        setCursor(Qt::SizeAllCursor);
    else
        setCursor(Qt::ArrowCursor);
}

int Control::higherZValue() const
{
    int z = 0;
    for (const auto& control : childControls())
        if (control->zValue() > z)
            z = control->zValue();
    return z;
}

int Control::lowerZValue() const
{
    int z = 0;
    for (const auto& control : childControls())
        if (control->zValue() < z)
            z = control->zValue();
    return z;
}

void Control::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
{
    if (_d->preview.isNull())
        return;

    auto innerRect = rect().adjusted(0.5, 0.5, -0.5, -0.5);

    if (gui() && parentControl() && parentControl()->clip() && !_dragging)
        painter->setClipRect(rect().intersected(parentControl()->mapToItem
          (this, parentControl()->rect().adjusted(1, 1, -1, -1)).boundingRect()));

    painter->setRenderHint(QPainter::Antialiasing);
    painter->drawPixmap(rect(), _d->preview, QRectF(QPointF(0, 0), size() * pS->devicePixelRatio()));

    QLinearGradient gradient(innerRect.center().x(), innerRect.y(),
                             innerRect.center().x(), innerRect.bottom());
    gradient.setColorAt(0, HIGHLIGHT_COLOR.lighter(110));
    gradient.setColorAt(1, HIGHLIGHT_COLOR.darker(110));

    if (_dragIn) {
        if (_showOutline) {
            painter->fillRect(innerRect, gradient);
        } else {
            QPixmap highlight(_d->preview);
            QPainter p(&highlight);
            p.setCompositionMode(QPainter::CompositionMode_SourceAtop);
            p.fillRect(_d->preview.rect(), gradient);
            p.end();
            painter->drawPixmap(rect(), highlight, QRectF(QPointF(0, 0), size() * pS->devicePixelRatio()));
        }
    }

    if (isSelected() || _showOutline) {
        QPen pen;
        pen.setStyle(Qt::DotLine);
        painter->setBrush(Qt::transparent);

        if (isSelected()) {
            pen.setColor(SELECTION_COLOR);
        } else if (_showOutline) {
            if (_d->hoverOn)
                pen.setStyle(Qt::SolidLine);
            pen.setColor(OUTLINE_COLOR);
        }

        painter->setPen(pen);
        painter->drawRect(innerRect);

        // Draw corner lines
        pen.setStyle(Qt::SolidLine);
        painter->setPen(pen);
        painter->drawLine(innerRect.topLeft(), innerRect.topLeft() + QPointF(2, 0));
        painter->drawLine(innerRect.topLeft(), innerRect.topLeft() + QPointF(0, 2));
        painter->drawLine(innerRect.bottomLeft(), innerRect.bottomLeft() + QPointF(2, 0));
        painter->drawLine(innerRect.bottomLeft(), innerRect.bottomLeft() + QPointF(0, -2));
        painter->drawLine(innerRect.topRight(), innerRect.topRight() + QPointF(-2, 0));
        painter->drawLine(innerRect.topRight(), innerRect.topRight() + QPointF(0, 2));
        painter->drawLine(innerRect.bottomRight(), innerRect.bottomRight() + QPointF(-2, 0));
        painter->drawLine(innerRect.bottomRight(), innerRect.bottomRight() + QPointF(0, -2));
    }
}

//FIXME: Control alignment has some bugs
bool Control::stickSelectedControlToGuideLines() const
{
    bool ret = false;
    auto scene = static_cast<ControlScene*>(this->scene());

    if (scene == nullptr)
        return ret;

    auto selectedControls = scene->selectedControls();
    selectedControls.removeOne(scene->mainControl());

    for (auto control : selectedControls)
        if (control->gui() == false)
            selectedControls.removeOne(control);

    if (selectedControls.size() <= 0)
        return ret;

    auto control = selectedControls[0];
    auto parent = control->parentControl();
    auto geometry = control->geometry();
    auto center = geometry.center();

    /* Child center <-> Parent center */
    if (center.y() <= parent->size().height() / 2.0 + MAGNETIC_FIELD &&
        center.y() >= parent->size().height() / 2.0 - MAGNETIC_FIELD) {
        control->setPos(control->x(), parent->size().height() / 2.0 - control->size().height() / 2.0);
        geometry = control->geometry();
        center = geometry.center();
        ret = true;
    }

    if (center.x() <= parent->size().width() / 2.0 + MAGNETIC_FIELD &&
        center.x() >= parent->size().width() / 2.0 - MAGNETIC_FIELD) {
        control->setPos(parent->size().width() / 2.0 - control->size().width() / 2.0, control->y());
        geometry = control->geometry();
        center = geometry.center();
        ret = true;
    }

    /* Child left <-> Parent center */
    if (geometry.topLeft().x() <= parent->size().width() / 2.0 + MAGNETIC_FIELD &&
        geometry.topLeft().x() >= parent->size().width() / 2.0 - MAGNETIC_FIELD) {
        control->setPos(parent->size().width() / 2.0, control->y());
        geometry = control->geometry();
        center = geometry.center();
        ret = true;
    }

    /* Child left <-> Parent left */
    if (geometry.topLeft().x() <= MAGNETIC_FIELD &&
        geometry.topLeft().x() >= - MAGNETIC_FIELD) {
        control->setPos(0, control->y());
        geometry = control->geometry();
        center = geometry.center();
        ret = true;
    }

    /* Child right <-> Parent center */
    if (geometry.topRight().x() <= parent->size().width() / 2.0 + MAGNETIC_FIELD &&
        geometry.topRight().x() >= parent->size().width() / 2.0 - MAGNETIC_FIELD) {
        geometry.moveTopRight({parent->size().width() / 2.0, control->y()});
        control->setPos(geometry.topLeft());
        center = geometry.center();
        ret = true;
    }

    /* Child right <-> Parent right */
    if (geometry.topRight().x() <= parent->size().width() + MAGNETIC_FIELD &&
        geometry.topRight().x() >= parent->size().width() - MAGNETIC_FIELD) {
        geometry.moveTopRight({parent->size().width(), control->y()});
        control->setPos(geometry.topLeft());
        center = geometry.center();
        ret = true;
    }

    /* Child top <-> Parent center */
    if (geometry.y() <= parent->size().height() / 2.0 + MAGNETIC_FIELD &&
        geometry.y() >= parent->size().height() / 2.0 - MAGNETIC_FIELD) {
        geometry.moveTopLeft({control->x(), parent->size().height() / 2.0});
        control->setPos(geometry.topLeft());
        center = geometry.center();
        ret = true;
    }

    /* Child top <-> Parent top */
    if (geometry.y() <= MAGNETIC_FIELD &&
        geometry.y() >= - MAGNETIC_FIELD) {
        geometry.moveTopLeft({control->x(), 0});
        control->setPos(geometry.topLeft());
        center = geometry.center();
        ret = true;
    }

    /* Child bottom <-> Parent center */
    if (geometry.bottomLeft().y() <= parent->size().height() / 2.0 + MAGNETIC_FIELD &&
        geometry.bottomLeft().y() >= parent->size().height() / 2.0 - MAGNETIC_FIELD) {
        geometry.moveBottomLeft({control->x(), parent->size().height() / 2.0});
        control->setPos(geometry.topLeft());
        center = geometry.center();
        ret = true;
    }

    /* Child bottom <-> Parent bottom */
    if (geometry.bottomLeft().y() <= parent->size().height() + MAGNETIC_FIELD &&
        geometry.bottomLeft().y() >= parent->size().height() - MAGNETIC_FIELD) {
        geometry.moveBottomLeft({control->x(), parent->size().height()});
        control->setPos(geometry.topLeft());
        center = geometry.center();
        ret = true;
    }

    for (auto childControl : parent->childControls()) {
        if (childControl == control || childControl->gui() == false)
            continue;

        auto cgeometry = childControl->geometry();
        auto ccenter = cgeometry.center();

        /* Item1 center <-> Item2 center */
        if (center.x() <= ccenter.x() + MAGNETIC_FIELD &&
            center.x() >= ccenter.x() - MAGNETIC_FIELD) {
            geometry.moveCenter({ccenter.x(), center.y()});
            control->setPos(geometry.topLeft());
            center = geometry.center();
            ret = true;
        }

        if (center.y() <= ccenter.y() + MAGNETIC_FIELD &&
            center.y() >= ccenter.y() - MAGNETIC_FIELD) {
            geometry.moveCenter({center.x(), ccenter.y()});
            control->setPos(geometry.topLeft());
            center = geometry.center();
            ret = true;
        }

        /* Item1 center <-> Item2 left */
        if (center.x() <= cgeometry.topLeft().x() + MAGNETIC_FIELD &&
            center.x() >= cgeometry.topLeft().x() - MAGNETIC_FIELD) {
            geometry.moveCenter({cgeometry.topLeft().x(), center.y()});
            control->setPos(geometry.topLeft());
            center = geometry.center();
            ret = true;
        }

        /* Item1 center <-> Item2 top */
        if (center.y() <= cgeometry.topLeft().y() + MAGNETIC_FIELD &&
            center.y() >= cgeometry.topLeft().y() - MAGNETIC_FIELD) {
            geometry.moveCenter({center.x(), cgeometry.topLeft().y()});
            control->setPos(geometry.topLeft());
            center = geometry.center();
            ret = true;
        }

        /* Item1 center <-> Item2 right */
        if (center.x() <= cgeometry.bottomRight().x() + MAGNETIC_FIELD &&
            center.x() >= cgeometry.bottomRight().x() - MAGNETIC_FIELD) {
            geometry.moveCenter({cgeometry.bottomRight().x(), center.y()});
            control->setPos(geometry.topLeft());
            center = geometry.center();
            ret = true;
        }

        /* Item1 center <-> Item2 bottom */
        if (center.y() <= cgeometry.bottomRight().y() + MAGNETIC_FIELD &&
            center.y() >= cgeometry.bottomRight().y() - MAGNETIC_FIELD) {
            geometry.moveCenter({center.x(), cgeometry.bottomRight().y()});
            control->setPos(geometry.topLeft());
            center = geometry.center();
            ret = true;
        }

        /* Item1 left <-> Item2 left */
        if (geometry.x() <= cgeometry.x() + MAGNETIC_FIELD &&
            geometry.x() >= cgeometry.x() - MAGNETIC_FIELD) {
            geometry.moveTopLeft({cgeometry.x(), geometry.y()});
            control->setPos(geometry.topLeft());
            center = geometry.center();
            ret = true;
        }

        /* Item1 left <-> Item2 center */
        if (geometry.x() <= ccenter.x() + MAGNETIC_FIELD &&
            geometry.x() >= ccenter.x() - MAGNETIC_FIELD) {
            geometry.moveTopLeft({ccenter.x(), geometry.y()});
            control->setPos(geometry.topLeft());
            center = geometry.center();
            ret = true;
        }

        /* Item1 left <-> Item2 right */
        if (geometry.x() <= cgeometry.topRight().x() + MAGNETIC_FIELD &&
            geometry.x() >= cgeometry.topRight().x() - MAGNETIC_FIELD) {
            geometry.moveTopLeft({cgeometry.topRight().x(), geometry.y()});
            control->setPos(geometry.topLeft());
            center = geometry.center();
            ret = true;
        }

        /* Item1 right <-> Item2 left */
        if (geometry.topRight().x() <= cgeometry.x() + MAGNETIC_FIELD &&
            geometry.topRight().x() >= cgeometry.x() - MAGNETIC_FIELD) {
            geometry.moveTopRight({cgeometry.x(), geometry.y()});
            control->setPos(geometry.topLeft());
            center = geometry.center();
            ret = true;
        }

        /* Item1 right <-> Item2 center */
        if (geometry.topRight().x() <= ccenter.x() + MAGNETIC_FIELD &&
            geometry.topRight().x() >= ccenter.x() - MAGNETIC_FIELD) {
            geometry.moveTopRight({ccenter.x(), geometry.y()});
            control->setPos(geometry.topLeft());
            center = geometry.center();
            ret = true;
        }

        /* Item1 right <-> Item2 right */
        if (geometry.topRight().x() <= cgeometry.topRight().x() + MAGNETIC_FIELD &&
            geometry.topRight().x() >= cgeometry.topRight().x() - MAGNETIC_FIELD) {
            geometry.moveTopRight({cgeometry.topRight().x(), geometry.y()});
            control->setPos(geometry.topLeft());
            center = geometry.center();
            ret = true;
        }

        /* Item1 top <-> Item2 top */
        if (geometry.y() <= cgeometry.y() + MAGNETIC_FIELD &&
            geometry.y() >= cgeometry.y() - MAGNETIC_FIELD) {
            geometry.moveTopLeft({geometry.x(), cgeometry.y()});
            control->setPos(geometry.topLeft());
            center = geometry.center();
            ret = true;
        }

        /* Item1 top <-> Item2 center */
        if (geometry.y() <= ccenter.y() + MAGNETIC_FIELD &&
            geometry.y() >= ccenter.y() - MAGNETIC_FIELD) {
            geometry.moveTopLeft({geometry.x(), ccenter.y()});
            control->setPos(geometry.topLeft());
            center = geometry.center();
            ret = true;
        }

        /* Item1 top <-> Item2 bottom */
        if (geometry.y() <= cgeometry.bottomLeft().y() + MAGNETIC_FIELD &&
            geometry.y() >= cgeometry.bottomLeft().y() - MAGNETIC_FIELD) {
            geometry.moveTopLeft({geometry.x(), cgeometry.bottomLeft().y()});
            control->setPos(geometry.topLeft());
            center = geometry.center();
            ret = true;
        }

        /* Item1 bottom <-> Item2 top */
        if (geometry.bottomLeft().y() <= cgeometry.y() + MAGNETIC_FIELD &&
            geometry.bottomLeft().y() >= cgeometry.y() - MAGNETIC_FIELD) {
            geometry.moveBottomLeft({geometry.x(), cgeometry.y()});
            control->setPos(geometry.topLeft());
            center = geometry.center();
            ret = true;
        }

        /* Item1 bottom <-> Item2 center */
        if (geometry.bottomLeft().y() <= ccenter.y() + MAGNETIC_FIELD &&
            geometry.bottomLeft().y() >= ccenter.y() - MAGNETIC_FIELD) {
            geometry.moveBottomLeft({geometry.x(), ccenter.y()});
            control->setPos(geometry.topLeft());
            center = geometry.center();
            ret = true;
        }

        /* Item1 bottom <-> Item2 bottom */
        if (geometry.bottomLeft().y() <= cgeometry.bottomLeft().y() + MAGNETIC_FIELD &&
            geometry.bottomLeft().y() >= cgeometry.bottomLeft().y() - MAGNETIC_FIELD) {
            geometry.moveBottomLeft({geometry.x(), cgeometry.bottomLeft().y()});
            control->setPos(geometry.topLeft());
            center = geometry.center();
            ret = true;
        }
    }

    return ret;
}

QVector<QLineF> Control::guideLines() const
{
    QVector<QLineF> lines;
    auto scene = static_cast<ControlScene*>(this->scene());

    if (scene == nullptr)
        return lines;

    auto selectedControls = scene->selectedControls();
    selectedControls.removeOne(scene->mainControl());

    for (auto control : selectedControls)
        if (control->gui() == false)
            selectedControls.removeOne(control);

    if (selectedControls.size() <= 0)
        return lines;

    auto control = selectedControls[0];
    auto parent = control->parentControl();
    auto geometry = control->geometry();
    auto center = geometry.center();

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

    for (auto childControl : parent->childControls()) {
        if (childControl == control || childControl->gui() == false)
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

//! ****************** [Form Private] ******************


class FormPrivate : public QObject
{
        Q_OBJECT

    public:
        explicit FormPrivate(Form* parent);
        void applySkinChange();

    public:
        Form* parent;
};

FormPrivate::FormPrivate(Form* parent)
    : QObject(parent)
    , parent(parent)
{
}

void FormPrivate::applySkinChange()
{
    QSize size;
    bool resizable;

    switch (parent->_skin) {
        case Skin::PhonePortrait:
            resizable = false;
            size = SIZE_FORM;
            break;

        case Skin::PhoneLandscape:
            resizable = false;
            size = SIZE_FORM.transposed();
            break;

        case Skin::Desktop:
        case Skin::NoSkin :
            resizable = true;
            break;
        default:
            break;
    }

    if (parent->_skin == Skin::PhonePortrait ||
        parent->_skin == Skin::PhoneLandscape)
        parent->resize(size);

    for (auto& resizer : parent->_resizers)
        resizer.setDisabled(!resizable);

    parent->update();
}

//! ********************** [Form] **********************

Form::Form(const QString& url, const QString& uid, Form* parent)
    : Control(url, FormGui, uid, parent)
    , _d(new FormPrivate(this))
    , _skin(SaveManager::skin(dir()))
{
    _clip = false;
    setFlag(Control::ItemIsMovable, false);
    _d->applySkinChange();
}

void Form::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    auto innerRect = rect().adjusted(0.5, 0.5, -0.5, -0.5);
    auto scene = static_cast<FormScene*>(this->scene());
    painter->setRenderHint(QPainter::Antialiasing);

    switch (_skin) {
        case PhonePortrait: {
            auto skinRect = QRectF({0, 0}, SIZE_SKIN);
            skinRect.moveCenter(innerRect.center());
            QSvgRenderer svg(QString(":/resources/images/phnv.svg"));
            svg.render(painter, skinRect);
            break;
        } case PhoneLandscape: {
            auto skinRect = QRectF({0, 0}, SIZE_SKIN.transposed());
            skinRect.moveCenter(innerRect.center());
            QSvgRenderer svg(QString(":/resources/images/phnh.svg"));
            svg.render(painter, skinRect);
            break;
        } case Desktop: {
            auto skinRect = QRectF({0, 0}, size() + QSizeF(fit(2), fit(2.0 * MARGIN_TOP / 1.35)));
            skinRect.moveCenter(innerRect.center());
            skinRect.moveTop(skinRect.top() - MARGIN_TOP / 1.5);
            QPainterPath path;
            path.setFillRule(Qt::WindingFill);
            path.addRect(skinRect.adjusted(0, fit(10), 0, 0));
            path.addRoundedRect(skinRect.adjusted(0, 0, 0, -skinRect.height() + fit(15)), fit(3), fit(3));

            QLinearGradient gradient(skinRect.center().x(), skinRect.y(),
                                     skinRect.center().x(), skinRect.y() + fit(2.2 * MARGIN_TOP / 1.35));
            gradient.setColorAt(0, QColor("#E8ECEF"));
            gradient.setColorAt(1, QColor("#D3D7DA"));
            painter->setBrush(gradient);
            painter->setPen(QColor("#D3D7DA").darker(106));
            painter->drawPath(path.simplified());

            auto btnExtRect = QRectF(skinRect.left() + fit(8), skinRect.top() + fit(4.5), fit(11), fit(11));
            auto btnMinRect = QRectF(skinRect.left() + fit(27), skinRect.top() + fit(4.5), fit(11), fit(11));
            auto btnMaxRect = QRectF(skinRect.left() + fit(46), skinRect.top() + fit(4.5), fit(11), fit(11));

            painter->setPen(QColor("#de4643"));
            painter->setBrush(QColor("#fc625d"));
            painter->drawEllipse(btnExtRect);

            painter->setPen(QColor("#dd9e33"));
            painter->setBrush(QColor("#fdbc40"));
            painter->drawEllipse(btnMinRect);

            painter->setPen(QColor("#26a934"));
            painter->setBrush(QColor("#34c84a"));
            painter->drawEllipse(btnMaxRect);

            if (mapToScene(QRectF(btnExtRect.topLeft(), btnMaxRect.bottomRight())).containsPoint
                (scene->lastMousePos(), Qt::WindingFill)) {
                auto ciks = QPixmap(":/resources/images/ciks.png");
                painter->setPen(QColor("#4c0102"));
                painter->drawLine(btnExtRect.topLeft() + QPoint(fit(3), fit(3)), btnExtRect.bottomRight() + QPoint(-fit(3), -fit(3)));
                painter->drawLine(btnExtRect.topRight() + QPoint(-fit(3), fit(3)), btnExtRect.bottomLeft() + QPoint(fit(3), -fit(3)));
                painter->setPen(QColor("#985712"));
                painter->drawLine(QPointF(btnMinRect.left() + fit(2.5), btnMinRect.center().y()),
                                  QPointF(btnMinRect.right() - fit(2.5), btnMinRect.center().y()));
                painter->drawPixmap(btnMaxRect.adjusted(fit(2.0), fit(2.0), -fit(2.0), -fit(2.0)), ciks, ciks.rect());
            }
            break;
        }
        default:
            break;
    }

    painter->setBrush(BACKGROUND_COLOR);
    painter->drawRect(innerRect);

    Control::paint(painter, option, widget);

    if (!isSelected() && !showOutline()) {
        QPen pen;
        pen.setJoinStyle(Qt::MiterJoin);
        if (_skin == PhonePortrait || _skin == PhoneLandscape) {
            pen.setColor(Qt::black);
        } else {
            pen.setStyle(Qt::DotLine);
            pen.setColor(OUTLINE_COLOR);
        }
        painter->setPen(pen);
        painter->setBrush(Qt::transparent);
        painter->drawRect(innerRect);
    }
}

void Form::resizeEvent(QGraphicsSceneResizeEvent* event)
{
    Control::resizeEvent(event);
    centralize();
}

void Form::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    event->ignore();
}

QRectF Form::frameGeometry() const
{
    QRectF rect;
    if (_skin == PhonePortrait || _skin == PhoneLandscape)
        rect = QRectF({QPointF(-SIZE_SKIN.width() / 2.0, -SIZE_SKIN.height() / 2.0), SIZE_SKIN});
    else if (_skin == NoSkin)
        rect = QRectF({QPointF(-size().width() / 2.0, -size().height() / 2.0), size()});
    else
        rect = QRectF(-size().width() / 2.0, -size().height() / 2.0 - MARGIN_TOP / 1.5,
                      size().width(), size().height() + 2.0 * MARGIN_TOP / 1.5);
    return rect;
}

bool Form::main() const
{
    return _main;
}

void Form::setMain(bool value)
{
    _main = value;
}

void Form::setSkin(const Skin& skin)
{
    if (skin == _skin)
        return;
    _skin = skin;
    _d->applySkinChange();
    emit skinChanged();
    emit cW->skinChanged(this);
}

const Skin& Form::skin()
{
    return _skin;
}

#include "control.moc"
