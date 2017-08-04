#include <control.h>
#include <fit.h>
#include <qmlpreviewer.h>
#include <windowscene.h>
#include <savemanager.h>

#include <QDebug>
#include <QTimer>
#include <QtMath>
#include <QPen>
#include <QQuickWindow>
#include <QWidget>
#include <QPainter>
#include <QBitmap>
#include <QVector>
#include <QMimeData>
#include <QGraphicsScene>
#include <QApplication>
#include <QQmlEngine>
#include <QQuickWidget>
#include <QQmlComponent>
#include <QQmlIncubator>
#include <QQuickItem>
#include <QQuickItemGrabResult>
#include <QSharedPointer>
#include <QGraphicsView>
#include <QGraphicsSceneDragDropEvent>
#include <QGraphicsSceneMouseEvent>
#include <QMessageBox>
#include <QMetaObject>

#define TOOLBOX_ITEM_KEY "QURBUEFaQVJMSVlJWiBIQUZJWg"
#define RESIZER_SIZE (fit(6.0))
#define HIGHLIGHT_COLOR (QColor("#174C4E4D"))
#define SELECTION_COLOR ("#404447")
#define OUTLINE_COLOR ("#808487")
#define RESIZER_COLOR (Qt::white)
#define RESIZER_OUTLINE_COLOR ("#202427")
#define PREVIEW_REFRESH_INTERVAL 100
#define RESIZE_TRANSACTION_INTERVAL 800
#define MAGNETIC_FIELD (fit(3))
#define PAGE_PP_SIZE (fit(QSize(250, 415)))
#define PAGE_PL_SIZE (fit(QSize(415, 250)))
#define PAGE_TOP_MARGIN (fit(14))
#define MOBILE_SKIN_COLOR (QColor("#52616D"))

using namespace Fit;

//!
//! ********************** [Resizer] **********************
//!

bool Resizer::_resizing = false;

Resizer::Resizer(Control *parent)
    : QGraphicsWidget(parent)
    , _placement(Top)
    , _disabled(false)
{
    setVisible(false);
    setAcceptedMouseButtons(Qt::LeftButton);
    setAcceptHoverEvents(true);
}

QRectF Resizer::boundingRect() const
{
    return QRectF(0, 0, RESIZER_SIZE, RESIZER_SIZE);
}

void Resizer::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
{
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setPen(RESIZER_OUTLINE_COLOR);
    painter->setBrush(RESIZER_COLOR);
    painter->drawRoundedRect(boundingRect().adjusted(0.5, 0.5, -0.5, -0.5),
                             RESIZER_SIZE / 4.0, RESIZER_SIZE / 4.0);
}

void Resizer::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsItem::mousePressEvent(event);
    event->accept();
    _resizing = true;
}

void Resizer::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsItem::mouseMoveEvent(event);

    if (_disabled)
        return;

    qreal diff_x, diff_y;
    auto* parent = static_cast<Control*>(parentItem());
    auto startSize = parent->size();
    switch (_placement) {
        case Top:
            diff_y = event->lastPos().y() - event->pos().y();
            parent->setGeometry(parent->geometry().adjusted(0, -diff_y, 0, 0));
            break;

        case Right:
            diff_x = event->pos().x() - event->lastPos().x();
            parent->setGeometry(parent->geometry().adjusted(0, 0, diff_x, 0));
            break;

        case Bottom:
            diff_y = event->pos().y() - event->lastPos().y();
            parent->setGeometry(parent->geometry().adjusted(0, 0, 0, diff_y));
            break;

        case Left:
            diff_x = event->lastPos().x() - event->pos().x();
            parent->setGeometry(parent->geometry().adjusted(-diff_x, 0, 0, 0));
            break;

        case TopLeft:
            diff_x = event->lastPos().x() - event->pos().x();
            diff_y = event->lastPos().y() - event->pos().y();
            parent->setGeometry(parent->geometry().adjusted(-diff_x, -diff_y, 0, 0));
            break;

        case TopRight:
            diff_x = event->pos().x() - event->lastPos().x();
            diff_y = event->lastPos().y() - event->pos().y();
            parent->setGeometry(parent->geometry().adjusted(0, -diff_y, diff_x, 0));
            break;

        case BottomRight:
            diff_x = event->pos().x() - event->lastPos().x();
            diff_y = event->pos().y() - event->lastPos().y();
            parent->setGeometry(parent->geometry().adjusted(0, 0, diff_x, diff_y));
            break;

        case BottomLeft:
            diff_x = event->lastPos().x() - event->pos().x();
            diff_y = event->pos().y() - event->lastPos().y();
            parent->setGeometry(parent->geometry().adjusted(-diff_x, 0, 0, diff_y));
            break;
    }

    if (parent->size().width() < RESIZER_SIZE ||
        parent->size().height() < RESIZER_SIZE) {
        parent->resize(startSize);
    }
}

void Resizer::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsItem::mouseReleaseEvent(event);
    _resizing = false;
}

bool Resizer::disabled() const
{
    return _disabled;
}

void Resizer::setDisabled(bool disabled)
{
    _disabled = disabled;
}

bool Resizer::resizing()
{
    return _resizing;
}

Resizer::Placement Resizer::placement() const
{
    return _placement;
}

void Resizer::setPlacement(const Resizer::Placement& placement)
{
    _placement = placement;

    switch (_placement) {
        case Resizer::Top:
            setCursor(Qt::SizeVerCursor);
            break;

        case Resizer::Right:
            setCursor(Qt::SizeHorCursor);
            break;

        case Resizer::Bottom:
            setCursor(Qt::SizeVerCursor);
            break;

        case Resizer::Left:
            setCursor(Qt::SizeHorCursor);
            break;

        case Resizer::TopLeft:
            setCursor(Qt::SizeFDiagCursor);
            break;

        case Resizer::TopRight:
            setCursor(Qt::SizeBDiagCursor);
            break;

        case Resizer::BottomRight:
            setCursor(Qt::SizeFDiagCursor);
            break;

        case Resizer::BottomLeft:
            setCursor(Qt::SizeBDiagCursor);
            break;
    }
}

//!
//! ****************** [Control Private] ******************
//!

class ControlPrivate : public QObject
{
        Q_OBJECT

    public:
        ControlPrivate(Control* parent);
        void fixResizerCoordinates();
        void hideResizers();
        void showResizers();

    public slots:
        void refreshPreview();
        void updatePreview(const PreviewResult& result);
        void handlePreviewErrors(QList<QQmlError> errors);

    public:
        Control* parent;
        QPixmap itemPixmap;
        QTimer refreshTimer;
        QmlPreviewer qmlPreviewer;
        bool hoverOn;
        bool initialized;
};

ControlPrivate::ControlPrivate(Control* parent)
    : QObject(parent)
    , parent(parent)
    , hoverOn(false)
    , initialized(false)
{
    int i = 0;
    for (auto& resizer : parent->_resizers) {
        resizer.setParentItem(parent);
        resizer.setPlacement(Resizer::Placement(i++));
    }

    refreshTimer.setInterval(PREVIEW_REFRESH_INTERVAL);
    connect(&refreshTimer, SIGNAL(timeout()), SLOT(refreshPreview()));
    connect(&qmlPreviewer, SIGNAL(errorsOccurred(QList<QQmlError>)),
            SLOT(handlePreviewErrors(QList<QQmlError>)));
    connect(&qmlPreviewer, SIGNAL(previewReady(PreviewResult)),
            SLOT(updatePreview(PreviewResult)));
}

void ControlPrivate::fixResizerCoordinates()
{
    for (auto& resizer : parent->_resizers) {
        switch (resizer.placement()) {
            case Resizer::Top:
                resizer.setPos(parent->size().width() / 2.0 - RESIZER_SIZE / 2.0,
                               - RESIZER_SIZE / 2.0 + 0.5);
                break;

            case Resizer::Right:
                resizer.setPos(parent->size().width() - RESIZER_SIZE / 2.0 - 0.5,
                               parent->size().height() / 2.0 - RESIZER_SIZE / 2.0);
                break;

            case Resizer::Bottom:
                resizer.setPos(parent->size().width() / 2.0 - RESIZER_SIZE / 2.0,
                               parent->size().height() - RESIZER_SIZE / 2.0 - 0.5);
                break;

            case Resizer::Left:
                resizer.setPos(- RESIZER_SIZE / 2.0 + 0.5,
                               parent->size().height() / 2.0 - RESIZER_SIZE / 2.0);
                break;

            case Resizer::TopLeft:
                resizer.setPos(- RESIZER_SIZE / 2.0 + 0.5, - RESIZER_SIZE / 2.0 + 0.5);
                break;

            case Resizer::TopRight:
                resizer.setPos(parent->size().width() - RESIZER_SIZE / 2.0 - 0.5,
                               - RESIZER_SIZE / 2.0 + 0.5);
                break;

            case Resizer::BottomRight:
                resizer.setPos(parent->size().width() - RESIZER_SIZE / 2.0 - 0.5,
                               parent->size().height() - RESIZER_SIZE / 2.0 - 0.5);
                break;

            case Resizer::BottomLeft:
                resizer.setPos( - RESIZER_SIZE / 2.0 + 0.5,
                                parent->size().height() - RESIZER_SIZE / 2.0 - 0.5);
                break;
        }
        resizer.setZValue(MAX_Z_VALUE);
    }
}

void ControlPrivate::hideResizers()
{
    for (auto& resizer : parent->_resizers) {
        resizer.hide();
    }
}

void ControlPrivate::showResizers()
{
    for (auto& resizer : parent->_resizers) {
        resizer.show();
    }
}

void ControlPrivate::refreshPreview()
{
    refreshTimer.stop();

    if (initialized)
        qmlPreviewer.requestReview(parent->url(), parent->size());
    else
        qmlPreviewer.requestReview(parent->url());
}

void ControlPrivate::updatePreview(const PreviewResult& result)
{
    itemPixmap = result.preview;

    if (initialized == false) {
        auto scene = static_cast<WindowScene*>(parent->scene());
        auto currentWindow = scene->currentWindow();
        auto id = result.id;

        QStringList windowNames;
        for (auto window : scene->windows())
            if (window == parent)
                continue;
            else
                windowNames << window->id();

        for (int i = 1; currentWindow->contains(id) || windowNames.contains(id); i++)
            id = result.id + QString::number(i);

        SaveManager::addSave(id, parent->url().toLocalFile());

        scene->clearSelection();

        parent->setFlag(Control::ItemIsFocusable);
        parent->setFlag(Control::ItemIsSelectable);
        parent->setAcceptHoverEvents(true);
        parent->setId(id);
        parent->setGui(result.gui);
        parent->setProperties(result.properties);
        parent->setEvents(result.events);

        if (result.gui == false) {
            parent->setParentItem(WindowScene::currentWindow()); //BUG: occurs when database loading
            parent->_controlTransaction.flushParentChange();
            parent->_controlTransaction.setGeometryTransactionsEnabled(false);
            parent->_controlTransaction.setParentTransactionsEnabled(false);
            parent->_controlTransaction.setZTransactionsEnabled(false);
            parent->resize(NONGUI_CONTROL_SIZE, NONGUI_CONTROL_SIZE);
            WindowScene::nonGuiControlsPanel()->addControl(parent);
            for (auto& resizer : parent->_resizers)
                resizer.setDisabled(true);
        } else {
            parent->setFlag(Control::ItemIsMovable);
            parent->setFlag(Control::ItemSendsGeometryChanges);
            parent->setAcceptDrops(true);
            parent->setPos(result.pos);
            parent->resize(result.size);
            parent->setClip(result.clip);
            parent->setZValue(result.zValue);
        }

        parent->setSelected(true);
        parent->_controlTransaction.flushParentChange();
    }

    parent->update();

    if (initialized == false) {
        initialized = true;
        emit parent->initialized();
    }

    emit parent->previewChanged();
}

void ControlPrivate::handlePreviewErrors(QList<QQmlError> errors)
{
    QMessageBox box;
    box.setText("<b>This tool has some errors, please fix these first.</b>");
    box.setInformativeText(errors[0].description());
    box.setStandardButtons(QMessageBox::Ok);
    box.setDefaultButton(QMessageBox::Ok);
    box.setIcon(QMessageBox::Information);
    box.exec();

    parent->scene()->removeItem(parent);
}

//!
//! ********************** [Control] **********************
//!

bool Control::_showOutline = false;

Control::Control(const QUrl& url, Control* parent)
    : QGraphicsWidget(parent)
    , _d(new ControlPrivate(this))
    , _controlTransaction(this)
    , _url(url)
    , _dragging(false)
    , _dragIn(false)
    , _clip(true)
{
    setGeometry(0, 0, 0, 0);
}

bool Control::showOutline()
{
    return _showOutline;
}

void Control::setShowOutline(const bool value)
{
    _showOutline = value;
}

QList<Control*> Control::childControls() const
{
    QList<Control*> controls;
    for (auto item : childItems()) {
        if (dynamic_cast<Control*>(item)) {
            controls << static_cast<Control*>(item);
            controls << controls.last()->childControls();
        }
    }
    return controls;
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
    QString prevId = _id;
    _id = id;
    setToolTip(id);
    emit idChanged(prevId);
}

QUrl Control::url() const
{
    return _url;
}

void Control::refresh()
{
    _d->refreshTimer.start();
}

void Control::centralize()
{
    setPos(- size().width() / 2.0, - size().height() / 2.0);
}

QRectF Control::frameGeometry() const
{
    return QRectF({QPointF(-size().width() / 2.0, -size().height() / 2.0), size()});
}

void Control::dropControl(Control* control)
{
    control->setPos(mapFromItem(control->parentItem(), control->pos()));
    control->setParentItem(this);
    control->refresh();
    update();
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

    auto pos = event->pos();
    auto control = new Control(event->mimeData()->urls().at(0));
    control->setParentItem(this);
    control->refresh();
    connect(control, &Control::initialized, [=] {
        control->setPos(pos);
    });

    event->accept();
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

        auto scene = static_cast<WindowScene*>(this->scene());
        for (auto c : scene->currentWindow()->childControls())
            if (c != control)
                c->setDragIn(false);

        if (scene->currentWindow() != control)
            scene->currentWindow()->setDragIn(false);
    }

    if (event->button() == Qt::MidButton)
        event->ignore();
    else
        event->accept();
}

void Control::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsWidget::mouseReleaseEvent(event);

    auto scene = static_cast<WindowScene*>(this->scene());

    for (auto control : scene->currentWindow()->childControls()) {
        if (control->dragIn() && dragging() &&
            parentControl() != control) {
            control->dropControl(this);
            scene->clearSelection();
            control->setSelected(true);
        }
        control->setDragIn(false);
    }

    if (scene->currentWindow()->dragIn() && dragging() &&
        parentControl() != scene->currentWindow()) {
        scene->currentWindow()->dropControl(this);
        scene->clearSelection();
        scene->currentWindow()->setSelected(true);
    }
    scene->currentWindow()->setDragIn(false);

    event->accept();
}

void Control::resizeEvent(QGraphicsSceneResizeEvent* event)
{
    QGraphicsWidget::resizeEvent(event);
    _d->fixResizerCoordinates();
    refresh();
}

QVariant Control::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant& value)
{
    switch (change) {
        case ItemSelectedHasChanged:
            if (value.toBool()) {
                _d->showResizers();
            } else {
                _d->hideResizers();
            }
            break;
        default:
            break;
    }
    return QGraphicsWidget::itemChange(change, value);
}

bool Control::gui() const
{
    return _gui;
}

void Control::setGui(bool value)
{
    _gui = value;
}

QList<QString> Control::events() const
{
    return _events;
}

void Control::setEvents(const QList<QString>& events)
{
    _events = events;
}

QMap<QString, QVariant::Type> Control::properties() const
{
    return _properties;
}

void Control::setProperties(const QMap<QString, QVariant::Type>& properties)
{
    _properties = properties;
}

bool Control::clip() const
{
    return _clip;
}

void Control::setClip(bool clip)
{
    _clip = clip;
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
    int z = -MAX_Z_VALUE;
    for (auto control : childControls())
        if (control->zValue() > z)
            z = control->zValue();
    return z;
}

int Control::lowerZValue() const
{
    int z = MAX_Z_VALUE;
    for (auto control : childControls())
        if (control->zValue() < z)
            z = control->zValue();
    return z;
}

void Control::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
{
    if (_d->itemPixmap.isNull())
        return;

    auto innerRect = rect().adjusted(0.5, 0.5, -0.5, -0.5);

    if (gui() && parentControl() && parentControl()->clip() && !_dragging)
        painter->setClipRect(rect().intersected(parentControl()->mapToItem(this, parentControl()->rect().adjusted(1, 1, -1, -1))
                                                .boundingRect()));
    if (gui() == false) //FIXME
        painter->setClipRect(rect().intersected(parentControl()->mapToItem(this, parentControl()->rect().adjusted(1, 1, -1, -1))
                                                .boundingRect()));

    painter->setRenderHint(QPainter::Antialiasing);
    painter->drawPixmap(rect(), _d->itemPixmap, QRectF(QPointF(0, 0), size() * qApp->devicePixelRatio()));

    QLinearGradient gradient(innerRect.center().x(), innerRect.y(),
                             innerRect.center().x(), innerRect.bottom());
    gradient.setColorAt(0, HIGHLIGHT_COLOR.lighter(110));
    gradient.setColorAt(1, HIGHLIGHT_COLOR.darker(110));

    if (_dragIn) {
        if (_showOutline) {
            painter->fillRect(innerRect, gradient);
        } else {
            QPixmap highlight(_d->itemPixmap);
            QPainter p(&highlight);
            p.setCompositionMode(QPainter::CompositionMode_SourceAtop);
            p.fillRect(_d->itemPixmap.rect(), gradient);
            p.end();
            painter->drawPixmap(rect(), highlight, QRectF(QPointF(0, 0), size() * qApp->devicePixelRatio()));
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

bool Control::stickSelectedControlToGuideLines() const
{
    bool ret = false;
    auto scene = static_cast<WindowScene*>(this->scene());
    auto selectedControls = scene->selectedControls();

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
        if (childControl == control)
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
    auto scene = static_cast<WindowScene*>(this->scene());
    auto selectedControls = scene->selectedControls();

    if (selectedControls.size() <= 0)
        return QVector<QLineF>();

    QVector<QLineF> lines;
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
        if (childControl == control)
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

//! ****************** [Window Private] ******************


class WindowPrivate : public QObject
{
        Q_OBJECT

    public:
        explicit WindowPrivate(Window* parent);
        static void applySkinChange();

    public:
        Window* parent;
        static QSizeF skinSize;
};

QSizeF WindowPrivate::skinSize;

WindowPrivate::WindowPrivate(Window* parent)
    : QObject(parent)
    , parent(parent)
{
}

void WindowPrivate::applySkinChange()
{
    QSize size;
    bool resizable;

    switch (Window::_skin) {
        case Window::PhonePortrait:
            resizable = false;
            size = PAGE_PP_SIZE;
            skinSize = PAGE_PP_SIZE + QSize(16, 75);
            break;

        case Window::PhoneLandscape:
            resizable = false;
            size = PAGE_PL_SIZE;
            skinSize = PAGE_PL_SIZE + QSize(75, 16);
            break;

        case Window::Desktop:
        case Window::NoSkin :
            resizable = true;
            break;
    }

    for (auto window : WindowScene::windows()) {
        if (Window::_skin == Window::PhonePortrait ||
            Window::_skin == Window::PhoneLandscape)
            window->resize(size);
        else
            window->update();

        for (auto& resizer : window->_resizers)
            resizer.setDisabled(!resizable);
    }
}

//! ********************** [Window] **********************

Window::Skin Window::_skin = Window::PhonePortrait;

Window::Window(const QUrl& url, Window* parent)
    : Control(url, parent)
    , _d(new WindowPrivate(this))
{
    connect(this, &Window::initialized, [=] {
        setFlag(ItemIsMovable, false);
    });
    connect(this, &Window::previewChanged, [=] {
        WindowPrivate::applySkinChange();
        this->disconnect(SIGNAL(previewChanged()));
    });
}

void Window::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    auto innerRect = rect().adjusted(0.5, 0.5, -0.5, -0.5);
    painter->setRenderHint(QPainter::Antialiasing);

    switch (_skin) {
        case PhonePortrait: {
            auto skinRect = QRectF({0, 0}, WindowPrivate::skinSize);
            skinRect.moveCenter(innerRect.center());
            skinRect.moveTop(skinRect.top() + PAGE_TOP_MARGIN);
            painter->setBrush(MOBILE_SKIN_COLOR);
            painter->setPen(MOBILE_SKIN_COLOR.darker(110));
            painter->drawRoundedRect(skinRect, fit(10), fit(10));
            painter->setPen(MOBILE_SKIN_COLOR.darker(110));
            painter->setBrush(scene()->views().first()->backgroundBrush());
            painter->drawRoundedRect(QRect(skinRect.x() + skinRect.width() / 3.0, skinRect.top() + PAGE_TOP_MARGIN / 1.5,
                                           skinRect.width() / 3.0, PAGE_TOP_MARGIN / 3.0), PAGE_TOP_MARGIN / 6.0, PAGE_TOP_MARGIN / 6.0);
            painter->drawRoundedRect(QRect(skinRect.x() + skinRect.width() / 2.0 - PAGE_TOP_MARGIN,
                                           skinRect.bottom() - PAGE_TOP_MARGIN / 1.5 - 2 * PAGE_TOP_MARGIN,
                                           2 * PAGE_TOP_MARGIN, 2 * PAGE_TOP_MARGIN), PAGE_TOP_MARGIN, PAGE_TOP_MARGIN);
            break;
        } case PhoneLandscape: {
            auto skinRect = QRectF({0, 0}, WindowPrivate::skinSize);
            skinRect.moveCenter(innerRect.center());
            skinRect.moveLeft(skinRect.left() + PAGE_TOP_MARGIN);
            painter->setBrush(MOBILE_SKIN_COLOR);
            painter->setPen(MOBILE_SKIN_COLOR.darker(110));
            painter->drawRoundedRect(skinRect, fit(10), fit(10));
            painter->setBrush(scene()->views().first()->backgroundBrush());
            painter->setPen(MOBILE_SKIN_COLOR.darker(110));
            painter->drawRoundedRect(QRect(skinRect.left() + PAGE_TOP_MARGIN / 1.5, skinRect.y() + skinRect.height() / 3.0,
                                           PAGE_TOP_MARGIN / 3.0, skinRect.height() / 3.0), PAGE_TOP_MARGIN / 6.0, PAGE_TOP_MARGIN / 6.0);
            painter->drawRoundedRect(QRect(skinRect.right() - PAGE_TOP_MARGIN / 1.5 - 2 * PAGE_TOP_MARGIN,
                                           skinRect.y() + skinRect.height() / 2.0 - PAGE_TOP_MARGIN,
                                           2 * PAGE_TOP_MARGIN, 2 * PAGE_TOP_MARGIN), PAGE_TOP_MARGIN, PAGE_TOP_MARGIN);
            break;
        } case Desktop: {
            auto skinRect = QRectF({0, 0}, size() + QSizeF(fit(2), fit(2.0 * PAGE_TOP_MARGIN / 1.35)));
            skinRect.moveCenter(innerRect.center());
            skinRect.moveTop(skinRect.top() - PAGE_TOP_MARGIN / 1.5);
            QPainterPath path;
            path.setFillRule(Qt::WindingFill);
            path.addRect(skinRect.adjusted(0, fit(10), 0, 0));
            path.addRoundedRect(skinRect.adjusted(0, 0, 0, -skinRect.height() + fit(15)), fit(3), fit(3));

            QLinearGradient gradient(skinRect.center().x(), skinRect.y(),
                                     skinRect.center().x(), skinRect.y() + fit(2.2 * PAGE_TOP_MARGIN / 1.35));
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
                (WindowScene::lastMousePos(), Qt::WindingFill)) {
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
        } case NoSkin: {
            break;
        }
    }

    painter->setBrush(scene()->views().first()->backgroundBrush());
    painter->drawRect(innerRect);

    Control::paint(painter, option, widget);

    if (!isSelected() && !showOutline()) {
        QPen pen;
        pen.setJoinStyle(Qt::MiterJoin);
        if (_skin == PhonePortrait || _skin == PhoneLandscape) {
            pen.setColor(MOBILE_SKIN_COLOR.darker(110));
        } else {
            pen.setStyle(Qt::DotLine);
            pen.setColor(OUTLINE_COLOR);
        }
        painter->setPen(pen);
        painter->setBrush(Qt::transparent);
        painter->drawRect(innerRect);
    }
}

void Window::resizeEvent(QGraphicsSceneResizeEvent* event)
{
    Control::resizeEvent(event);
    centralize();
}

void Window::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    event->ignore();
}

bool Window::contains(const QString& id) const
{
    for (auto control : childControls()) {
        if (control->id() == id)
            return true;
    }
    return false;
}

QRectF Window::frameGeometry() const
{
    QRectF rect;
    if (_skin == PhonePortrait || _skin == PhoneLandscape)
        rect = QRectF({QPointF(-_d->skinSize.width() / 2.0, -_d->skinSize.height() / 2.0), _d->skinSize});
    else if (_skin == NoSkin)
        rect = QRectF({QPointF(-size().width() / 2.0, -size().height() / 2.0), size()});
    else
        rect = QRectF(-size().width() / 2.0, -size().height() / 2.0 - PAGE_TOP_MARGIN / 1.5,
                      size().width(), size().height() + 2.0 * PAGE_TOP_MARGIN / 1.5);
    return rect;
}

bool Window::isMain() const
{
    return _main;
}

void Window::setMain(bool value)
{
    _main = value;
}

void Window::setSkin(const Skin& skin)
{
    _skin = skin;
    WindowPrivate::applySkinChange();
}

const Window::Skin& Window::skin()
{
    return _skin;
}

void Window::centralize()
{
    if (_skin == PhonePortrait)
        setPos(- size().width() / 2.0, - size().height() / 2.0 - PAGE_TOP_MARGIN);
    else if (_skin == PhoneLandscape)
        setPos(- size().width() / 2.0 - PAGE_TOP_MARGIN, - size().height() / 2.0);
    else if (_skin == NoSkin)
        setPos(- size().width() / 2.0, - size().height() / 2.0);
    else
        setPos(- size().width() / 2.0, - size().height() / 2.0 + PAGE_TOP_MARGIN / 1.5);
}

void Window::cleanWindow()
{
    WindowScene::removeChildControlsOnly(this);
}

#include "control.moc"
