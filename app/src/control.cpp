#include <control.h>
#include <delayer.h>
#include <fit.h>
#include <qmlpreviewer.h>
#include <designerscene.h>
#include <savemanager.h>
#include <delayer.h>

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
#include <QGraphicsSceneDragDropEvent>
#include <QGraphicsSceneMouseEvent>
#include <QMessageBox>

#define TOOLBOX_ITEM_KEY "QURBUEFaQVJMSVlJWiBIQUZJWg"
#define RESIZER_SIZE (fit(6.0))
#define HIGHLIGHT_COLOR ("#100085ff")
#define SELECTION_COLOR ("#444444")
#define OUTLINE_COLOR (Qt::darkGray)
#define RESIZER_COLOR (Qt::white)
#define RESIZER_OUTLINE_COLOR ("#252525")
#define PREVIEW_REFRESH_INTERVAL 100
#define RESIZE_TRANSACTION_INTERVAL 800
#define MAGNETIC_FIELD (fit(3))

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

    _transactionTimer.setInterval(RESIZE_TRANSACTION_INTERVAL);
    connect(&_transactionTimer, SIGNAL(timeout()), SLOT(startTransaction()));
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

    _transactionTimer.start();
}

void Resizer::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsItem::mouseReleaseEvent(event);
    _resizing = false;
}

void Resizer::startTransaction()
{
    _transactionTimer.stop();
    auto* parent = static_cast<Control*>(parentItem());
    SaveManager::setVariantProperty(parent->id(), "x", parent->x());
    SaveManager::setVariantProperty(parent->id(), "y", parent->y());
    SaveManager::setVariantProperty(parent->id(), "width", parent->size().width());
    SaveManager::setVariantProperty(parent->id(), "height", parent->size().height());
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
        Resizer resizers[8];
        QTimer refreshTimer;
        QmlPreviewer qmlPreviewer;
};


ControlPrivate::ControlPrivate(Control* parent)
    : QObject(parent)
    , parent(parent)
{
    int i = 0;
    for (auto& resizer : resizers) {
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
    for (auto& resizer : resizers) {
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
        resizer.setZValue(1);
    }
}

void ControlPrivate::hideResizers()
{
    for (auto& resizer : resizers) {
        resizer.hide();
    }
}

void ControlPrivate::showResizers()
{
    for (auto& resizer : resizers) {
        resizer.show();
    }
}

void ControlPrivate::refreshPreview()
{
    refreshTimer.stop();

    QSizeF size;
    if (!parent->size().isNull())
        size = parent->size();

    qmlPreviewer.requestReview(parent->url(), size);
}

void ControlPrivate::updatePreview(const PreviewResult& result)
{
    itemPixmap = result.preview;

    if (result.initial) {
        auto scene = static_cast<DesignerScene*>(parent->scene());
        auto currentPage = scene->currentPage();
        auto id = result.id;

        for (int i = 1; currentPage->contains(id); i++)
            id = result.id + QString::number(i);

        parent->setId(id);
        parent->resize(result.size);
        parent->setClip(result.clip);
        Delayer::delay(100);

        SaveManager::addSave(id, parent->url().toLocalFile());
        SaveManager::setId(id, id);
        SaveManager::addParentalRelationship(id, parent->parentControl()->id());
        SaveManager::setVariantProperty(id, "x", parent->x());
        SaveManager::setVariantProperty(id, "y", parent->y());
    }

    parent->update();
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

bool Control::_showOutline = true;

Control::Control(Control* parent)
    : QGraphicsWidget(parent)
    , _d(new ControlPrivate(this))
    , _dragging(false)
    , _dragIn(false)
    , _clip(true)
{
    setFlag(ItemIsFocusable);
    setFlag(ItemIsSelectable);
    setFlag(ItemIsMovable);
    setFlag(ItemSendsGeometryChanges);
    setAcceptDrops(true);
    setAcceptHoverEvents(true);
    setGeometry(0, 0, 0, 0);
}

Control::~Control()
{
    delete _d;
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

QList<Control*> Control::collidingControls(Qt::ItemSelectionMode mode) const
{
    QList<Control*> list;
    for (auto item : collidingItems(mode)) {
        auto control = dynamic_cast<Control*>(item);
        if (control)
            list << control;
    }
    return list;
}

QString Control::id() const
{
    return _id;
}

void Control::setId(const QString& id)
{
    _id = id;
}

QUrl Control::url() const
{
    return _url;
}

void Control::setUrl(const QUrl& url)
{
    _url = url;
}

void Control::refresh()
{
    _d->refreshTimer.start();
}

void Control::dropControl(Control* control)
{
    control->setPos(mapFromItem(control->parentItem(), control->pos()));
    control->setParentItem(this);
    control->refresh();

    SaveManager::addParentalRelationship(control->id(), id());
    SaveManager::setVariantProperty(control->id(), "x", control->x());
    SaveManager::setVariantProperty(control->id(), "y", control->y());
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

    scene()->clearSelection();

    auto control = new Control;
    control->setUrl(event->mimeData()->urls().at(0));
    control->setParentItem(this);
    control->setPos(event->pos());
    control->setSelected(true);
    control->refresh();

    scene()->addItem(control);
    event->accept();
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

        auto scene = static_cast<DesignerScene*>(this->scene());
        for (auto c : scene->currentPage()->childControls())
            if (c != control)
                c->setDragIn(false);

        if (scene->currentPage() != control)
            scene->currentPage()->setDragIn(false);
    }

    if (event->button() == Qt::MidButton)
        event->ignore();
    else
        event->accept();
}

void Control::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsWidget::mouseReleaseEvent(event);

    auto scene = static_cast<DesignerScene*>(this->scene());

    for (auto control : scene->currentPage()->childControls()) {
        if (control->dragIn() && dragging() &&
            parentControl() != control) {
            control->dropControl(this);
            scene->clearSelection();
            control->setSelected(true);
        }
        control->setDragIn(false);
    }

    if (scene->currentPage()->dragIn() && dragging() &&
        parentControl() != scene->currentPage()) {
        scene->currentPage()->dropControl(this);
        scene->clearSelection();
        scene->currentPage()->setSelected(true);
    }
    scene->currentPage()->setDragIn(false);

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

void Control::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
{
    if (_d->itemPixmap.isNull())
        return;

    if (parentControl() && parentControl()->clip() && !_dragging)
        painter->setClipRect(rect().intersected(parentControl()->mapToItem(this, parentControl()->rect().adjusted(1, 1, -1, -1))
                                                .boundingRect()));

    painter->setRenderHint(QPainter::Antialiasing);
    painter->drawPixmap(rect(), _d->itemPixmap, QRectF(QPointF(0, 0), size() * qApp->devicePixelRatio()));

    if (_dragIn) {
        if (_showOutline) {
            painter->fillRect(rect().adjusted(0.5, 0.5, -0.5, -0.5), HIGHLIGHT_COLOR);
        } else {
            QPixmap highlight(_d->itemPixmap);
            QPainter p(&highlight);
            p.setCompositionMode(QPainter::CompositionMode_SourceAtop);
            p.fillRect(_d->itemPixmap.rect(), HIGHLIGHT_COLOR);
            p.end();
            painter->drawPixmap(rect(), highlight, QRectF(QPointF(0, 0), size() * qApp->devicePixelRatio()));
        }
    }

    if (isSelected() || _showOutline) {
        QPen pen;
        pen.setStyle(Qt::DotLine);
        pen.setJoinStyle(Qt::MiterJoin);
        painter->setBrush(Qt::transparent);

        if (isSelected()) {
            pen.setColor(SELECTION_COLOR);
        } else if (_showOutline) {
            pen.setColor(OUTLINE_COLOR);
        }

        painter->setPen(pen);
        painter->drawRect(rect().adjusted(0.5, 0.5, -0.5, -0.5));
    }
}

//! ****************** [Page Private] ******************


class PagePrivate : public QObject
{
        Q_OBJECT

    public:
        explicit PagePrivate(Page* parent);
        void pullMeNearLine(Control* control) const;
        QVector<QLineF> guidelinesFor(const Control* control) const;
};

PagePrivate::PagePrivate(Page* parent)
    : QObject(parent)
{
}

//! ********************** [Page] **********************

const Page::SkinSetting* Page::_skinSetting = nullptr;

Page::Page(Page* parent)
    : Control(parent)
    , _d(new PagePrivate(this))
{
    if (_skinSetting)
        _resizable = _skinSetting->resizable;
    else
        _resizable = true;

    for (auto& resizer : Control::_d->resizers) {
        resizer.setDisabled(!_resizable);
    }

    setFlag(ItemIsMovable, false);
}

void Page::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Control::paint(painter, option, widget);

    if (!isSelected() && !showOutline()) {
        QPen pen;
        pen.setStyle(Qt::DotLine);
        pen.setJoinStyle(Qt::MiterJoin);
        painter->setBrush(Qt::transparent);
        pen.setColor(OUTLINE_COLOR);
        painter->setPen(pen);
        painter->drawRect(rect().adjusted(0.5, 0.5, -0.5, -0.5));
    }

    if (_skinSetting && !_skinSetting->pixmap.isNull() && _skinSetting->rect.isValid())
        painter->drawPixmap(_skinSetting->rect,
                            _skinSetting->pixmap, _skinSetting->pixmap.rect());
}

void Page::resizeEvent(QGraphicsSceneResizeEvent* event)
{
    Control::resizeEvent(event);
    centralize();
}

void Page::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    event->ignore();
}

bool Page::resizable() const
{
    return _resizable;
}

void Page::setResizable(bool resizable)
{
    _resizable = resizable;
    for (auto& resizer : Control::_d->resizers) {
        resizer.setDisabled(!_resizable);
    }
}

bool Page::stickSelectedControlToGuideLines() const
{
    bool ret = false;
    auto scene = static_cast<DesignerScene*>(this->scene());
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

QVector<QLineF> Page::guideLines() const
{
    auto scene = static_cast<DesignerScene*>(this->scene());
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

bool Page::contains(const QString& id) const
{
    for (auto control : childControls()) {
        if (control->id() == id)
            return true;
    }
    return false;
}

bool Page::mainPage() const
{
    return _mainPage;
}

void Page::setMainPage(bool mainPage)
{
    _mainPage = mainPage;
}

void Page::centralize()
{
    setPos(- size().width() / 2.0, - size().height() / 2.0);
}

void Page::setSkinSetting(const SkinSetting* skinSetting)
{
    _skinSetting = skinSetting;
}

const Page::SkinSetting* Page::skinSetting()
{
    return _skinSetting;
}

#include "control.moc"
