#include <control.h>
#include <fit.h>
#include <random>
#include <filemanager.h>
#include <controlwatcher.h>
#include <suppressor.h>
#include <savebackend.h>
#include <controlscene.h>
#include <formscene.h>

#include <QtMath>
#include <QtWidgets>
#include <QtSvg>

#define HIGHLIGHT_COLOR (QColor("#174C4E4D"))
#define SELECTION_COLOR ("#404447")
#define OUTLINE_COLOR ("#808487")
#define BACKGROUND_COLOR (QColor("#F0F4F7"))
#define PREVIEW_REFRESH_INTERVAL 100
#define RESIZE_TRANSACTION_INTERVAL 800
#define GEOMETRY_SIGNAL_DELAY 800
#define MARGIN_TOP (fit::fx(14))
#define ADJUST(x) ((x).adjusted(0.5, 0.5, -0.5, -0.5))

#define pS (QApplication::primaryScreen())
#define cW (ControlWatcher::instance())

//!
//! ************************* [global] **************************
//!

namespace {
    /* Fills the restricted area by the size with pattern into
     * the transparent dest. Then draws source into the center of the dest. */
    void drawCenter(QImage& dest, const QImage& source, const QSizeF& size)
    {
        qreal dpr = pS->devicePixelRatio();

        QBrush brush;
        brush.setColor("#b0b4b7");
        brush.setStyle(Qt::Dense6Pattern);

        QPainter painter(&dest);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setBrush(brush);
        painter.setPen("#808487");

        QRectF rect;
        rect.setTopLeft({0.0, 0.0});
        rect.setWidth(size.width());
        rect.setHeight(size.height());

        QRectF rect_2;
        rect_2.setWidth(source.width() / dpr);
        rect_2.setHeight(source.height() / dpr);
        rect_2.moveCenter(rect.center());

        painter.drawRect(ADJUST(rect));
        painter.drawImage(rect_2, source);
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

    public slots:
        void updatePreview(const QSharedPointer<PreviewResult>& result);

    public:
        Control* parent;
        QImage preview;
        bool hoverOn;
};

//TODO: Search for ControlScene and FormScene and make their usage less
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

    connect(PreviewBackend::instance(),
      SIGNAL(previewReady(const QSharedPointer<PreviewResult>&)),
            SLOT(updatePreview(const QSharedPointer<PreviewResult>&)));
}

void ControlPrivate::updatePreview(const QSharedPointer<PreviewResult>& result)
{
    if (result->control != parent)
        return;

    preview = result->preview;
    parent->_errors = result->errors;
    parent->_gui = result->gui;
    parent->_properties = result->properties;
    parent->_events = result->events;

    if (!result->hasError()) {
        if (result->gui) {
            if (!parent->form())
                parent->_clip = result->property("clip").toBool();
        }
    }

    parent->update();
    if (result->hasError()) {
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
//TODO: Why we need mode()?
Control::Control(const QString& url, const DesignMode& mode,
  const QString& uid, Control* parent)
    : QGraphicsWidget(parent)
    , _clip(true)
    , _d(new ControlPrivate(this))
    , _uid(uid.isEmpty() ? SaveBackend::uid(dname(dname(url))) : uid)
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

    setZValue(SaveBackend::z(dir()));
    setId(SaveBackend::id(dname(dname(url))));
    setPos(SaveBackend::x(dir()), SaveBackend::y(dir()));
    resize(fit::fx(SaveBackend::width(dir())), fit::fx(SaveBackend::height(dir())));
    if (size().isNull()) resize(SIZE_NONGUI_CONTROL);
    _d->preview = initialPreview();

    connect(this, &Control::geometryChanged, this, [=] {
        QPointer<Control> p(this);
        Suppressor::suppress(GEOMETRY_SIGNAL_DELAY, "geometryChanged", [=] {
            if (p)
                emit cW->geometryChanged(this);
        });
    });

    connect(this, &Control::zChanged, this, [this] {
        // refresh(); No need, cause it doesn't change preview
        emit cW->zValueChanged(this);
    });

    connect(this, &Control::parentChanged, this, [this] {
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
    auto randNum = QRandomGenerator::global()->generate();
    auto randNum1 = QRandomGenerator::global()->generate();
    auto randNum2 = QRandomGenerator::global()->generate();
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
    PreviewBackend::requestPreview(this);
}

void Control::updateUid()
{
    _uid = SaveBackend::uid(dir());
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
        mimeData->text() == TOOL_KEY) {
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
        control != this && !control->dragIn()) {
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
    auto selectedControls = scene->selectedControls();
    selectedControls.removeOne(scene->mainControl());

    for (auto control : scene->mainControl()->childControls()) {
        if (control->dragIn() && dragging() && parentControl() != control) {
            for (auto sc : selectedControls) {
                if (sc->dragging()) {
                    control->dropControl(sc);
                }
            }
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
    refresh();
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

QImage Control::initialPreview() const
{
    qreal dpr = pS->devicePixelRatio();
    qreal min = qMin(fit::fx(24), qMin(size().width(), size().height()));

    QImage preview(
        qCeil(size().width() * dpr),
        qCeil(size().height() * dpr),
        QImage::Format_ARGB32_Premultiplied
    );

    preview.setDevicePixelRatio(dpr);
    preview.fill(Qt::transparent);

    QImage wait(":/resources/images/wait.png");
    wait.setDevicePixelRatio(dpr);

    drawCenter(
        preview,
        wait.scaled(
            min * dpr,
            min * dpr,
            Qt::IgnoreAspectRatio,
            Qt::SmoothTransformation
        ),
        size()
    );

    return preview;
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

QList<Control*>& Control::controls()
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
    painter->drawImage(rect(), _d->preview,
      QRectF(QPointF(0, 0), size() * pS->devicePixelRatio()));

    QLinearGradient gradient(innerRect.center().x(), innerRect.y(),
      innerRect.center().x(), innerRect.bottom());
    gradient.setColorAt(0, HIGHLIGHT_COLOR.lighter(110));
    gradient.setColorAt(1, HIGHLIGHT_COLOR.darker(110));

    if (_dragIn) {
        if (_showOutline) {
            painter->fillRect(innerRect, gradient);
        } else {
            QImage highlight(_d->preview);
            QPainter p(&highlight);
            p.setCompositionMode(QPainter::CompositionMode_SourceAtop);
            p.fillRect(_d->preview.rect(), gradient);
            p.end();
            painter->drawImage(rect(), highlight, QRectF(
              QPointF(0, 0), size() * pS->devicePixelRatio()));
        }
    }

    if (isSelected() || _showOutline) {
        QPen pen;
        pen.setWidthF(fit::fx(1));
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
        painter->drawLine(innerRect.topLeft(),
          innerRect.topLeft() + QPointF(2, 0));
        painter->drawLine(innerRect.topLeft(),
          innerRect.topLeft() + QPointF(0, 2));
        painter->drawLine(innerRect.bottomLeft(),
          innerRect.bottomLeft() + QPointF(2, 0));
        painter->drawLine(innerRect.bottomLeft(),
          innerRect.bottomLeft() + QPointF(0, -2));
        painter->drawLine(innerRect.topRight(),
          innerRect.topRight() + QPointF(-2, 0));
        painter->drawLine(innerRect.topRight(),
          innerRect.topRight() + QPointF(0, 2));
        painter->drawLine(innerRect.bottomRight(),
          innerRect.bottomRight() + QPointF(-2, 0));
        painter->drawLine(innerRect.bottomRight(),
          innerRect.bottomRight() + QPointF(0, -2));
    }
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
            resizable = false;
            qFatal("Something went wrong.");
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
    , _skin(SaveBackend::skin(dir()))
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
        } case Desktop: { //FIXME: Bad window frame, redesign it.
            auto skinRect = QRectF({0, 0}, size() + QSizeF(fit::fx(2), fit::fx(2.0 * MARGIN_TOP / 1.35)));
            skinRect.moveCenter(innerRect.center());
            skinRect.moveTop(skinRect.top() - MARGIN_TOP / 1.5);
            QPainterPath path;
            path.setFillRule(Qt::WindingFill);
            path.addRect(skinRect.adjusted(0, fit::fx(10), 0, 0));
            path.addRoundedRect(skinRect.adjusted(0, 0, 0, -skinRect.height() + fit::fx(15)), fit::fx(3), fit::fx(3));

            QLinearGradient gradient(skinRect.center().x(), skinRect.y(),
                                     skinRect.center().x(), skinRect.y() + fit::fx(2.2 * MARGIN_TOP / 1.35));
            gradient.setColorAt(0, QColor("#E8ECEF"));
            gradient.setColorAt(1, QColor("#D3D7DA"));
            painter->setBrush(gradient);
            painter->setPen(QColor("#D3D7DA").darker(106));
            painter->drawPath(path.simplified());

            auto btnExtRect = QRectF(skinRect.left() + fit::fx(8), skinRect.top() + fit::fx(4.5), fit::fx(11), fit::fx(11));
            auto btnMinRect = QRectF(skinRect.left() + fit::fx(27), skinRect.top() + fit::fx(4.5), fit::fx(11), fit::fx(11));
            auto btnMaxRect = QRectF(skinRect.left() + fit::fx(46), skinRect.top() + fit::fx(4.5), fit::fx(11), fit::fx(11));

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
                auto ciks = QImage(":/resources/images/ciks.png");
                painter->setPen(QColor("#4c0102"));
                painter->drawLine(btnExtRect.topLeft() + QPoint(fit::fx(3), fit::fx(3)), btnExtRect.bottomRight() + QPoint(-fit::fx(3), -fit::fx(3)));
                painter->drawLine(btnExtRect.topRight() + QPoint(-fit::fx(3), fit::fx(3)), btnExtRect.bottomLeft() + QPoint(fit::fx(3), -fit::fx(3)));
                painter->setPen(QColor("#985712"));
                painter->drawLine(QPointF(btnMinRect.left() + fit::fx(2.5), btnMinRect.center().y()),
                                  QPointF(btnMinRect.right() - fit::fx(2.5), btnMinRect.center().y()));
                painter->drawImage(btnMaxRect.adjusted(fit::fx(2.0), fit::fx(2.0), -fit::fx(2.0), -fit::fx(2.0)), ciks, ciks.rect());
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
        pen.setWidthF(fit::fx(1));
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
