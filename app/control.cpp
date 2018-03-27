#include <form.h>
#include <fit.h>
#include <dpr.h>
#include <resizer.h>
#include <saveutils.h>
#include <suppressor.h>
#include <filemanager.h>
#include <parserutils.h>
#include <designerscene.h>
#include <controlwatcher.h>
#include <previewerbackend.h>

#include <QtMath>
#include <QDebug>
#include <QCursor>
#include <QPainter>
#include <QMimeData>
#include <QRegularExpression>
#include <QGraphicsSceneDragDropEvent>

extern const char* TOOL_KEY;

namespace {
    void setInitialProperties(Control* control);
    QList<Resizer*> initializeResizers(Control* control);
    void drawCenter(QImage& dest, const QImage& source, const QSizeF& size);
    QRectF getRect(const PreviewResult& result);
    qreal getZ(const PreviewResult& result);
    void setRect(QList<PropertyNode>& nodes, const QRectF& rect);
    void setZ(QList<PropertyNode>& nodes, qreal z);
    QImage initialPreview();
}

QList<Control*> Control::m_controls;

Control::Control(const QString& url, Control* parent) : QGraphicsWidget(parent)
  , m_gui(false)
  , m_clip(true)
  , m_dragIn(false)
  , m_hoverOn(false)
  , m_dragging(false)
  , m_uid(SaveUtils::uid(dname(dname(url))))
  , m_url(url)
  , m_preview(initialPreview())
  , m_resizers(initializeResizers(this))
{
    m_controls << this;

    setAcceptDrops(true);
    setAcceptHoverEvents(true);
    setInitialProperties(this);
    setFlag(Control::ItemIsFocusable);
    setFlag(Control::ItemIsSelectable);
    setFlag(Control::ItemSendsGeometryChanges);

    connect(this, &Control::geometryChanged, this, [=] {
        QPointer<Control> p(this);
        Suppressor::suppress(800, "geometryChanged", [=] {
            if (p)
                emit cW->geometryChanged(this);
        });
    });

    connect(this, &Control::zChanged, this, [=] { emit cW->zValueChanged(this); });
    connect(this, &Control::parentChanged, this, [=] { emit cW->parentChanged(this); });
    connect(PreviewerBackend::instance(), &PreviewerBackend::anchorsReady, this, &Control::updateAnchors);
    connect(PreviewerBackend::instance(), &PreviewerBackend::previewReady, this, &Control::updatePreview);
}

Control::~Control()
{
    m_controls.removeOne(this);
}

void Control::updateUid()
{
    m_uid = SaveUtils::uid(dir());
}

void Control::updateUids()
{
    for (auto control : m_controls)
        control->updateUid();
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

void Control::centralize()
{
    setPos(-size().width() / 2.0, -size().height() / 2.0);
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
    if (scene())
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
    gradient.setColorAt(0, QColor("#174C4E4D").lighter(110));
    gradient.setColorAt(1, QColor("#174C4E4D").darker(110));

    if (m_dragIn) {
        if (scene()->showOutlines()) {
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

    if (isSelected() || scene()->showOutlines()) {
        QPen pen;
        pen.setWidthF(fit::fx(1));
        pen.setStyle(Qt::DotLine);
        painter->setBrush(Qt::transparent);

        if (isSelected()) {
            pen.setColor("#404447");
        } else if (scene()->showOutlines()) {
            if (m_hoverOn)
                pen.setStyle(Qt::SolidLine);
            pen.setColor("#808487");
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

namespace {
    QImage initialPreview()
    {
        QImage preview(
            qCeil(fit::fx(50) * DPR),
            qCeil(fit::fx(50) * DPR),
            QImage::Format_ARGB32_Premultiplied
        );

        preview.setDevicePixelRatio(DPR);
        preview.fill(Qt::transparent);

        QImage wait(":/resources/images/wait.png");
        wait.setDevicePixelRatio(DPR);

        drawCenter(
            preview,
            wait.scaled(
                fit::fx(24) * DPR,
                fit::fx(24) * DPR,
                Qt::IgnoreAspectRatio,
                Qt::SmoothTransformation
            ),
            fit::fx(QSizeF(50, 50))
        );

        return preview;
    }

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
        control->setId(SaveUtils::id(control->dir()));

        if (control->size().isNull())
            control->resize(fit::fx(QSizeF(50, 50)));
    }

    /*
     * Fills the restricted area by the size with pattern into
     * the transparent dest. Then draws source into the center of the dest.
    */
    void drawCenter(QImage& dest, const QImage& source, const QSizeF& size)
    {
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
        rect_2.setWidth(source.width() / DPR);
        rect_2.setHeight(source.height() / DPR);
        rect_2.moveCenter(rect.center());

        painter.drawRect(rect.adjusted(0.5, 0.5, -0.5, -0.5));
        painter.drawImage(rect_2, source);
    }

    QList<Resizer*> initializeResizers(Control* control)
    {
        QList<Resizer*> resizers;
        int i = 0;
        for (;i < 8;)
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