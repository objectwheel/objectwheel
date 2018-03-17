#include <control.h>
#include <fit.h>
#include <dpr.h>
#include <random>
#include <filemanager.h>
#include <controlwatcher.h>
#include <suppressor.h>
#include <designerscene.h>
#include <previewerbackend.h>
#include <saveutils.h>

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

extern const char* TOOL_KEY;

namespace {
    /* Fills the restricted area by the size with pattern into
     * the transparent dest. Then draws source into the center of the dest. */
    void drawCenter(QImage& dest, const QImage& source, const QSizeF& size)
    {
        qreal dpr = DPR;

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
        void updatePreview(const PreviewResult& result);
        void updateAnchors(const Anchors& anchors);

    public:
        Control* parent;
        QImage preview;
        bool hoverOn;
};

//TODO: Search for ControlScene and DesignerScene and make their usage less
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

    connect(PreviewerBackend::instance(), SIGNAL(previewReady(const PreviewResult&)),
      SLOT(updatePreview(const PreviewResult&)));
    connect(PreviewerBackend::instance(), SIGNAL(anchorsReady(const Anchors&)),
      SLOT(updateAnchors(const Anchors&)));
}

void ControlPrivate::updatePreview(const PreviewResult& result)
{
    if (result.uid != parent->uid())
        return;

    preview = result.preview;
    parent->_errors = result.errors;
    parent->_gui = result.gui;
    parent->_properties = result.propertyNodes;
    parent->_events = result.events;

    if (!result.hasError()) {
        if (result.gui) {
            if (!parent->form())
                parent->_clip = result.property("clip").toBool();
        }
    }

    parent->update();
    if (result.hasError()) {
        emit parent->errorOccurred();
        emit cW->errorOccurred(parent);
    }
    emit parent->previewChanged();
    emit cW->previewChanged(parent);
}

void ControlPrivate::updateAnchors(const Anchors& anchors)
{
    if (anchors.uid != parent->uid())
        return;

    qDebug() << anchors.bottom.id;
    qDebug() << anchors.top.id;
    qDebug() << anchors.left.id;
    qDebug() << anchors.right.id;
    qDebug() << anchors.verticalCenter.id;
    qDebug() << anchors.horizontalCenter.id;
    qDebug() << "-----------------------";
}

//!
//! ********************** [Control] **********************
//!

bool Control::_showOutline = false;
QList<Control*> Control::_controls;

Control::Control(
    const QString& url,
    const QString& uid,
    Control* parent
)
    : QGraphicsWidget(parent)
    , _clip(true)
    , _d(new ControlPrivate(this))
    , _uid(uid.isEmpty() ? SaveUtils::uid(dname(dname(url))) : uid)
    , _url(url)
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

    setZValue(SaveUtils::z(dir()));
    setId(SaveUtils::id(dname(dname(url))));
    setPos(SaveUtils::x(dir()), SaveUtils::y(dir()));
    resize(fit::fx(SaveUtils::width(dir())), fit::fx(SaveUtils::height(dir())));
    if (size().isNull()) resize(fit::fx(QSizeF(50, 50)));
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

void Control::refresh(bool repreview)
{
    PreviewerBackend::instance()->requestPreview(rect(), dir(), repreview);
    QTimer::singleShot(0, std::bind(&PreviewerBackend::requestAnchors, PreviewerBackend::instance(), dir()));
}

void Control::updateUid()
{
    _uid = SaveUtils::uid(dir());
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

        for (auto c : scene()->mainForm()->childControls())
            if (c != control)
                c->setDragIn(false);

        if (scene()->mainForm() != control)
            scene()->mainForm()->setDragIn(false);
    }

    if (event->button() == Qt::MidButton)
        event->ignore();
    else
        event->accept();
}

void Control::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsWidget::mouseReleaseEvent(event);

    auto selectedControls = scene()->selectedControls();
    selectedControls.removeOne(scene()->mainForm());

    for (auto control : scene()->mainForm()->childControls()) {
        if (control->dragIn() && dragging() && parentControl() != control) {
            for (auto sc : selectedControls) {
                if (sc->dragging()) {
                    control->dropControl(sc);
                }
            }
            scene()->clearSelection();
            control->setSelected(true);
        }
        control->setDragIn(false);
    }

    if (scene()->mainForm()->dragIn() && dragging() &&
        parentControl() != scene()->mainForm()) {
        scene()->mainForm()->dropControl(this);
        scene()->clearSelection();
        scene()->mainForm()->setSelected(true);
    }
    scene()->mainForm()->setDragIn(false);

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
    qreal dpr = DPR;
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

const QList<PropertyNode>& Control::properties() const
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

DesignerScene* Control::scene() const
{
    return static_cast<DesignerScene*>(QGraphicsWidget::scene());
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
      QRectF(QPointF(0, 0), size() * DPR));

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
              QPointF(0, 0), size() * DPR));
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

    public:
        Form* parent;
};

FormPrivate::FormPrivate(Form* parent)
    : QObject(parent)
    , parent(parent)
{
}

//! ********************** [Form] **********************

Form::Form(const QString& url, const QString& uid, Form* parent)
    : Control(url, uid, parent)
    , _d(new FormPrivate(this))
{
    _clip = false;
    setFlag(Control::ItemIsMovable, false);
}

void Form::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    auto innerRect = rect().adjusted(0.5, 0.5, -0.5, -0.5);

    painter->setBrush(BACKGROUND_COLOR);
    painter->drawRect(innerRect);

    Control::paint(painter, option, widget);

    if (!isSelected() && !showOutline()) {
        QPen pen;
        pen.setWidthF(fit::fx(1));
        pen.setJoinStyle(Qt::MiterJoin);
        pen.setStyle(Qt::DotLine);
        pen.setColor(OUTLINE_COLOR);

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
    return QRectF(QPointF(-size().width() / 2.0, -size().height() / 2.0), size());
}

bool Form::main() const
{
    return _main;
}

void Form::setMain(bool value)
{
    _main = value;
}

#include "control.moc"
