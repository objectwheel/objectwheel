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
#include <resizer.h>
#include <parserutils.h>

#include <QtMath>
#include <QtWidgets>
#include <QtSvg>

#define HIGHLIGHT_COLOR     "#174C4E4D"
#define SELECTION_COLOR       "#404447"
#define OUTLINE_COLOR         "#808487"
#define BACKGROUND_COLOR      "#F0F4F7"
#define PREVIEW_REFRESH_INTERVAL    100
#define RESIZE_TRANSACTION_INTERVAL 800
#define GEOMETRY_SIGNAL_DELAY       800
#define MARGIN_TOP fit::fx(14)
#define ADJUST(x)  (x).adjusted(0.5, 0.5, -0.5, -0.5)

extern const char* TOOL_KEY;

namespace {
    void setInitialProperties(Control* control);
    QList<Resizer*> initializeResizers(Control* control);
    void drawCenter(QImage& dest, const QImage& source, const QSizeF& size);
    QRectF getRect(const PreviewResult& result);
    qreal getZ(const PreviewResult& result);
    void setRect(QList<PropertyNode>& nodes, const QRectF& rect);
    void setZ(QList<PropertyNode>& nodes, qreal z);
}

bool Control::m_showOutline = false;
QList<Control*> Control::m_controls;

Control::Control(const QString& url, const QString& uid, Control* parent) : QGraphicsWidget(parent)
  , m_clip(true)
  , m_resizers(initializeResizers(this))
  , m_uid(uid.isEmpty() ? SaveUtils::uid(dname(dname(url))) : uid)
  , m_url(url)
  , m_hoverOn(false)
  , m_dragging(false)
  , m_dragIn(false)
  , m_gui(false)
{
    m_controls << this;

    setFlag(Control::ItemIsFocusable);
    setFlag(Control::ItemIsSelectable);
    setFlag(Control::ItemSendsGeometryChanges);
    setAcceptHoverEvents(true);
    setAcceptDrops(true);

    setId(SaveUtils::id(dir()));
    setInitialProperties(this);
    m_preview = initialPreview();

    connect(PreviewerBackend::instance(), SIGNAL(previewReady(const PreviewResult&)),
      SLOT(updatePreview(const PreviewResult&)));
    connect(PreviewerBackend::instance(), SIGNAL(anchorsReady(const Anchors&)),
      SLOT(updateAnchors(const Anchors&)));

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
    m_controls.removeOne(this);
}

bool Control::showOutline()
{
    return m_showOutline;
}

void Control::setShowOutline(const bool value)
{
    m_showOutline = value;
}

void Control::updateUids()
{
    for (auto control : m_controls)
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
    const QString& hex = QCryptographicHash::hash(uidData, QCryptographicHash::Sha256).toHex();
    return hex.left(6) + hex.right(6);
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
    for (auto resizer : m_resizers) {
        resizer->hide();
    }
}

void Control::showResizers()
{
    for (auto resizer : m_resizers) {
        resizer->show();
    }
}

Control* Control::parentControl() const
{
    return dynamic_cast<Control*>(parentItem());
}

QString Control::id() const
{
    return m_id;
}

void Control::setId(const QString& id)
{
    m_id = id;
    setToolTip(id);
}

QString Control::url() const
{
    return m_url;
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

void Control::updatePreview(const PreviewResult& result)
{
    if (result.uid != uid())
        return;

    m_preview = result.preview;
    m_errors = result.errors;
    m_gui = result.gui;
    m_properties = result.propertyNodes;
    m_events = result.events;

    if (!result.hasError()) {
        if (result.gui) {
            if (!form())
                m_clip = result.property("clip").toBool();

            if (result.repreviewed) {
                const auto& rect = getRect(result);
                qreal z = getZ(result);
                blockSignals(true);
                setRect(m_properties, rect);
                setZ(m_properties, z);
                resize(rect.size());
                if (!form())
                    setPos(rect.topLeft());
                setZValue(z);
                blockSignals(false);
            }
        }
    }

    if (!form())
        setFlag(Control::ItemIsMovable, !hasErrors());

    update();
    if (result.hasError()) {
        emit errorOccurred();
        emit cW->errorOccurred(this);
    }
    emit previewChanged();
    emit cW->previewChanged(this);
}

void Control::updateAnchors(const Anchors& anchors)
{
    if (anchors.uid != uid() || hasErrors())
        return;

    qDebug() << anchors.bottom.id;
    qDebug() << anchors.top.id;
    qDebug() << anchors.left.id;
    qDebug() << anchors.right.id;
    qDebug() << anchors.verticalCenter.id;
    qDebug() << anchors.horizontalCenter.id;
    qDebug() << "-----------------------";
}

void Control::refresh(bool repreview)
{
    PreviewerBackend::instance()->requestPreview(size(), dir(), repreview);
//    QTimer::singleShot(0, std::bind(&PreviewerBackend::requestAnchors, PreviewerBackend::instance(), dir()));
}

void Control::updateUid()
{
    m_uid = SaveUtils::uid(dir());
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
        m_dragIn = true;
        event->accept();
    } else {
        event->ignore();
    }
    update();
}

void Control::dragLeaveEvent(QGraphicsSceneDragDropEvent* event)
{
    m_dragIn = false;
    event->accept();
    update();
}

void Control::dragMoveEvent(QGraphicsSceneDragDropEvent* event)
{
    event->accept();
}

void Control::dropEvent(QGraphicsSceneDragDropEvent* event)
{
    m_dragIn = false;
    event->accept();
    emit controlDropped(event->pos(),
      event->mimeData()->urls().first().toLocalFile());
    emit cW->controlDropped(this, event->pos(),
      event->mimeData()->urls().first().toLocalFile());
    update();
}

void Control::dropControl(Control* control)
{
    control->setPos(mapFromItem(control->parentItem(), control->pos()));
    control->setParentItem(this);
    update();
}

void Control::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
{
    QGraphicsWidget::hoverEnterEvent(event);
    m_hoverOn = true;
    update();
}

void Control::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
    QGraphicsWidget::hoverLeaveEvent(event);
    m_hoverOn = false;
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
    if (m_dragging && items.size() > 1 && (control = dynamic_cast<Control*>(items[1])) &&
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
    for (auto resizer : m_resizers)
        resizer->correct();
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
    return m_errors;
}

bool Control::hasErrors() const
{
    return !m_errors.isEmpty();
}

QList<Control*>& Control::controls()
{
    return m_controls;
}

void Control::setUrl(const QString& url)
{
    m_url = url;
}

QString Control::dir() const // Returns root path
{
    return dname(dname(m_url));
}

QString Control::uid() const
{
    return m_uid;
}

bool Control::gui() const
{
    return m_gui;
}

const QList<QString>& Control::events() const
{
    return m_events;
}

const QList<PropertyNode>& Control::properties() const
{
    return m_properties;
}

bool Control::clip() const
{
    return m_clip;
}

bool Control::dragIn() const
{
    return m_dragIn;
}

void Control::setDragIn(bool dragIn)
{
    m_dragIn = dragIn;
}

DesignerScene* Control::scene() const
{
    return static_cast<DesignerScene*>(QGraphicsWidget::scene());
}

bool Control::dragging() const
{
    return m_dragging;
}

void Control::setDragging(bool dragging)
{
    m_dragging = dragging;

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
    if (m_preview.isNull())
        return;

    auto innerRect = rect().adjusted(0.5, 0.5, -0.5, -0.5);

    if (gui() && parentControl() && parentControl()->clip() && !m_dragging)
        painter->setClipRect(rect().intersected(parentControl()->mapToItem
          (this, parentControl()->rect().adjusted(1, 1, -1, -1)).boundingRect()));

    painter->setRenderHint(QPainter::Antialiasing);
    painter->drawImage(rect(), m_preview,
      QRectF(QPointF(0, 0), size() * DPR));

    QLinearGradient gradient(innerRect.center().x(), innerRect.y(),
      innerRect.center().x(), innerRect.bottom());
    gradient.setColorAt(0, QColor(HIGHLIGHT_COLOR).lighter(110));
    gradient.setColorAt(1, QColor(HIGHLIGHT_COLOR).darker(110));

    if (m_dragIn) {
        if (m_showOutline) {
            painter->fillRect(innerRect, gradient);
        } else {
            QImage highlight(m_preview);
            QPainter p(&highlight);
            p.setCompositionMode(QPainter::CompositionMode_SourceAtop);
            p.fillRect(m_preview.rect(), gradient);
            p.end();
            painter->drawImage(rect(), highlight, QRectF(
              QPointF(0, 0), size() * DPR));
        }
    }

    if (isSelected() || m_showOutline) {
        QPen pen;
        pen.setWidthF(fit::fx(1));
        pen.setStyle(Qt::DotLine);
        painter->setBrush(Qt::transparent);

        if (isSelected()) {
            pen.setColor(SELECTION_COLOR);
        } else if (m_showOutline) {
            if (m_hoverOn)
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

Form::Form(const QString& url, const QString& uid, Form* parent) : Control(url, uid, parent)
{
    m_clip = false;
    setFlag(Control::ItemIsMovable, false);
}

void Form::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    auto innerRect = rect().adjusted(0.5, 0.5, -0.5, -0.5);

    painter->setBrush(QColor(BACKGROUND_COLOR));
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
    return m_main;
}

void Form::setMain(bool value)
{
    m_main = value;
}

namespace {
    void setInitialProperties(Control *control)
    {
        qreal x = ParserUtils::property(control->url(), "x").remove(QRegularExpression("[\\r\\n\\t\\f\\v ]")).toDouble();
        qreal y = ParserUtils::property(control->url(), "y").remove(QRegularExpression("[\\r\\n\\t\\f\\v ]")).toDouble();
        qreal z = ParserUtils::property(control->url(), "z").remove(QRegularExpression("[\\r\\n\\t\\f\\v ]")).toDouble();
        qreal width = ParserUtils::property(control->url(), "width").remove(QRegularExpression("[\\r\\n\\t\\f\\v ]")).toDouble();
        qreal height = ParserUtils::property(control->url(), "height").remove(QRegularExpression("[\\r\\n\\t\\f\\v ]")).toDouble();

        control->setZValue(z);
        control->setPos(x, y);
        control->resize(width, height);

        if (control->size().isNull())
            control->resize(fit::fx(QSizeF(50, 50)));
    }

    /*
     * Fills the restricted area by the size with pattern into
     * the transparent dest. Then draws source into the center of the dest.
    */
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

    QList<Resizer*> initializeResizers(Control* control)
    {
        QList<Resizer*> resizers;
        int i = 0;
        for (; i < 8; )
            resizers << new Resizer(control, Resizer::Placement(i++));

        return resizers;
    }

    QRectF getRect(const PreviewResult& result)
    {
        QRectF rect;
        const QList<PropertyNode> nodes = result.propertyNodes;

        for (const auto& node : nodes) {
            for (const auto& property : node.properties.keys()) {
                if (property == "x")
                    rect.moveLeft(node.properties.value(property).toReal());
                else if (property == "y")
                    rect.moveTop(node.properties.value(property).toReal());
                else if (property == "width")
                    rect.setWidth(node.properties.value(property).toReal());
                else if (property == "height")
                    rect.setHeight(node.properties.value(property).toReal());
            }
        }

        return rect;
    }

    qreal getZ(const PreviewResult& result)
    {
        const QList<PropertyNode> nodes = result.propertyNodes;

        for (const auto& node : nodes) {
            for (const auto& property : node.properties.keys()) {
                if (property == "z")
                    return node.properties.value(property).toReal();
            }
        }

        return 0.0;
    }

    void setRect(QList<PropertyNode>& nodes, const QRectF& rect)
    {
        for (auto& node : nodes) {
            for (auto& property : node.properties.keys()) {
                if (property == "x")
                    node.properties[property] = rect.x();
                else if (property == "y")
                    node.properties[property] = rect.y();
                else if (property == "width")
                    node.properties[property] = rect.width();
                else if (property == "height")
                    node.properties[property] = rect.height();
            }
        }
    }

    void setZ(QList<PropertyNode>& nodes, qreal z)
    {
        for (auto& node : nodes) {
            for (auto& property : node.properties.keys()) {
                if (property == "z")
                    node.properties[property] = z;
            }
        }
    }
}