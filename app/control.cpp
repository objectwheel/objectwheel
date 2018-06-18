#include <form.h>
#include <dpr.h>
#include <resizer.h>
#include <saveutils.h>
#include <suppressor.h>
#include <filemanager.h>
#include <parserutils.h>
#include <designerscene.h>
#include <controlmonitoringmanager.h>
#include <controlpreviewingmanager.h>

#include <QtMath>
#include <QDebug>
#include <QCursor>
#include <QPainter>
#include <QMimeData>
#include <QRegularExpression>
#include <QGraphicsSceneDragDropEvent>

extern const char* TOOL_KEY;

namespace {
    qreal getZ(const PreviewResult& result);
    QImage initialPreview(const QSizeF& size);
    QRectF getRect(const PreviewResult& result);
    QList<Resizer*> initializeResizers(Control* control);
    void setInitialProperties(Control* control);
    void drawCenter(QImage& dest, const QImage& source, const QSizeF& size);
}

QList<Control*> Control::m_controls;

Control::Control(const QString& url, Control* parent) : QGraphicsWidget(parent)
  , m_gui(false)
  , m_clip(false)
  , m_dragIn(false)
  , m_hoverOn(false)
  , m_dragging(false)
  , m_refreshingDisabled(false)
  , m_url(url)
  , m_uid(SaveUtils::uid(dir()))
  , m_resizers(initializeResizers(this))
{
    m_controls << this;

    setAcceptDrops(true);
    setAcceptHoverEvents(true);
    setRefreshingDisabled(true);
    setInitialProperties(this);
    setRefreshingDisabled(false);

    setFlag(ItemIsMovable);
    setFlag(ItemIsFocusable);
    setFlag(ItemIsSelectable);
    setFlag(ItemSendsGeometryChanges);

    m_preview = initialPreview(size());

    connect(this, &Control::geometryChanged, this, [=] {
        QPointer<Control> p(this);
        Suppressor::suppress(800, "geometryChanged", [=] {
            if (p)
                emit ControlMonitoringManager::instance()->geometryChanged(this);
        });
    });

    connect(this, &Control::zChanged, this, &Control::onZValueChange);
    connect(this, &Control::zChanged, this,
            [=] { emit ControlMonitoringManager::instance()->zValueChanged(this); });
    connect(this, &Control::parentChanged, this, &Control::onParentChange);
    connect(this, &Control::parentChanged, this,
            [=] { emit ControlMonitoringManager::instance()->parentChanged(this); });

    connect(ControlPreviewingManager::instance(), &ControlPreviewingManager::previewReady, this, &Control::updatePreview);
}

Control::~Control()
{
    m_controls.removeOne(this);
}

void Control::onSizeChange()
{
    if (hasErrors() || !gui() || m_refreshingDisabled)
        return;

    ControlPreviewingManager::updateCache(uid(), "width", size().width());
    ControlPreviewingManager::updateCache(uid(), "height", size().height());
}

void Control::onParentChange()
{
    if (!parentControl() || (!parentControl()->gui() && gui()) || m_refreshingDisabled)
        return;

    ControlPreviewingManager::updateParent(uid(), parentControl()->uid(), url());

    onSizeChange();
}

void Control::onZValueChange()
{
    if (hasErrors() || !gui() || m_refreshingDisabled)
        return;

    // FIXME: Do we really need this?
    ControlPreviewingManager::updateCache(uid(), "z", zValue());
}

bool Control::gui() const
{
    return m_gui;
}

bool Control::form() const
{
    return (dynamic_cast<const Form*>(this) != nullptr);
}

bool Control::clip() const
{
    return m_clip;
}

bool Control::dragIn() const
{
    return m_dragIn;
}

bool Control::dragging() const
{
    return m_dragging;
}

bool Control::hasErrors() const
{
    return !m_errors.isEmpty();
}

bool Control::contains(const QString& id) const
{
    for (auto control : childControls()) {
        if (control->id() == id)
            return true;
    }
    return false;
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

QString Control::url() const
{
    return m_url;
}

QString Control::id() const
{
    return m_id;
}

QString Control::uid() const
{
    return m_uid;
}

QString Control::dir() const
{
    return dname(dname(m_url));
}

DesignerScene* Control::scene() const
{
    return static_cast<DesignerScene*>(QGraphicsWidget::scene());
}

Control* Control::parentControl() const
{
    return dynamic_cast<Control*>(parentItem());
}

const QList<QString>& Control::events() const
{
    return m_events;
}

const QList<QQmlError>& Control::errors() const
{
    return m_errors;
}

const QList<PropertyNode>& Control::properties() const
{
    return m_properties;
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

const QList<Control*>& Control::controls()
{
    return m_controls;
}

void Control::setClip(bool clip)
{
    m_clip = clip;
}

void Control::setId(const QString& id)
{
    m_id = id;
    setToolTip(id);
}

void Control::setUrl(const QString& url)
{
    m_url = url;
}

void Control::setDragIn(bool dragIn)
{
    m_dragIn = dragIn;
}

void Control::setDragging(bool dragging)
{
    m_dragging = dragging;

    if (dragging)
        setCursor(Qt::SizeAllCursor);
    else
        setCursor(Qt::ArrowCursor);
}

void Control::setRefreshingDisabled(bool disabled)
{
    m_refreshingDisabled = disabled;
}

void Control::updateUid()
{
    m_uid = SaveUtils::uid(dir());
}

void Control::centralize()
{
    setRefreshingDisabled(true);
    blockSignals(true);
    setPos(-size().width() / 2.0, -size().height() / 2.0);
    blockSignals(false);
    setRefreshingDisabled(false);
}

void Control::hideResizers()
{
    for (auto resizer : m_resizers)
        resizer->hide();
}

void Control::showResizers()
{
    for (auto resizer : m_resizers)
        resizer->show();
}

void Control::refresh(bool repreview)
{
    if (!m_refreshingDisabled)
        ControlPreviewingManager::requestPreview(dir(), repreview);
}

void Control::updateUids()
{
    for (auto control : m_controls)
        control->updateUid();
}

void Control::dropControl(Control* control)
{
    if (!gui() && control->gui())
        return;

    control->setPos(mapFromItem(control->parentItem(), control->pos()));
    control->setParentItem(this);
    update();
}

void Control::dropEvent(QGraphicsSceneDragDropEvent* event)
{
    m_dragIn = false;
    event->accept();
    emit controlDropped(event->pos(),
      event->mimeData()->urls().first().toLocalFile());
    emit ControlMonitoringManager::instance()->controlDropped(this, event->pos(),
      event->mimeData()->urls().first().toLocalFile());
    update();
}

void Control::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsWidget::mouseMoveEvent(event);

    Control* control = nullptr;
    auto items = scene()->items(event->scenePos());
    if (m_dragging && items.size() > 1 && (control = dynamic_cast<Control*>(items[1])) &&
        control != this && !control->dragIn()) {
        control->setDragIn(true);

        for (auto c : scene()->currentForm()->childControls())
            if (c != control)
                c->setDragIn(false);

        if (scene()->currentForm() != control)
            scene()->currentForm()->setDragIn(false);
    }

    if (event->button() == Qt::MidButton)
        event->ignore();
    else
        event->accept();
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

void Control::dragMoveEvent(QGraphicsSceneDragDropEvent* event)
{
    event->accept();
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

void Control::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsWidget::mouseReleaseEvent(event);

    auto selectedControls = scene()->selectedControls();
    selectedControls.removeOne(scene()->currentForm());

    for (auto control : scene()->currentForm()->childControls()) {
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

    if (scene()->currentForm()->dragIn() && dragging() &&
        parentControl() != scene()->currentForm()) {
        scene()->currentForm()->dropControl(this);
        scene()->clearSelection();
        scene()->currentForm()->setSelected(true);
    }
    scene()->currentForm()->setDragIn(false);

    event->accept();
}

void Control::mouseDoubleClickEvent(QGraphicsSceneMouseEvent*)
{
    emit doubleClicked();
    emit ControlMonitoringManager::instance()->doubleClicked(this);
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

void Control::resizeEvent(QGraphicsSceneResizeEvent* event)
{
    QGraphicsWidget::resizeEvent(event);

    for (auto resizer : m_resizers)
        resizer->correct();

    onSizeChange();
}

void Control::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsWidget::mousePressEvent(event);

    if (event->button() == Qt::MidButton)
        event->ignore();
    else
        event->accept();
}

void Control::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
{
    if (m_preview.isNull())
        return;

    if (parentControl() && parentControl()->clip() && !m_dragging) {
        painter->setClipRect(
           rect().intersected(
               parentControl()->mapToItem(
                   this,
                   parentControl()->rect().adjusted(1, 1, -1, -1)
               ).boundingRect()
           )
        );
    }

    painter->setRenderHint(QPainter::Antialiasing);
    painter->drawImage(rect(), m_preview, QRectF(QPointF(0, 0), size() * DPR));

    QLinearGradient gradient(rect().center().x(), rect().y(), rect().center().x(), rect().bottom());
    gradient.setColorAt(0, QColor("#174C4C4C").lighter(110));
    gradient.setColorAt(1, QColor("#174C4C4C").darker(120));

    if (m_dragIn) {
        if (scene()->showOutlines()) {
            painter->fillRect(rect(), gradient);
        } else {
            QImage highlight(m_preview);
            QPainter p(&highlight);
            p.setCompositionMode(QPainter::CompositionMode_SourceAtop);
            p.fillRect(m_preview.rect(), gradient);
            p.end();
            painter->drawImage(rect(), highlight, QRectF(QPointF(0, 0), size() * DPR));
        }
    }

    if (!form() && (isSelected() || scene()->showOutlines())) {
        QPen pen;
        pen.setStyle(Qt::DotLine);
        painter->setBrush(Qt::transparent);
        painter->setClipping(false);

        if (isSelected()) {
            pen.setColor(Qt::black);
        } else if (scene()->showOutlines()) {
            if (m_hoverOn) {
                pen.setStyle(Qt::SolidLine);
                pen.setColor("#4BA2FF");
            } else {
                pen.setColor("#777777");
            }
        }

        painter->setPen(pen);
        painter->drawRect(rect().adjusted(0.5, 0.5, -0.5, -0.5));
    }
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

    if (result.hasError()) {
        setRefreshingDisabled(true);
        blockSignals(true);
        resize(QSizeF(50, 50));
        blockSignals(false);
        setRefreshingDisabled(false);
        setPos(pos());
        setZValue(0);
    } else {
        if (result.gui) {
            if (!form())
                m_clip = result.property("clip").toBool();

            if (!m_dragging && !Resizer::resizing() && !ControlPreviewingManager::contains(uid())) {
                const auto& rect = getRect(result);
                qreal z = getZ(result);
                setRefreshingDisabled(true);
                blockSignals(true);
                resize(rect.size());
                if (!form())
                    setPos(rect.topLeft());
                setZValue(z);
                blockSignals(false);
                setRefreshingDisabled(false);
            }
        } else {
            setRefreshingDisabled(true);
            blockSignals(true);
            resize(QSizeF(50, 50));
            blockSignals(false);
            setRefreshingDisabled(false);
            setPos(pos());
            setZValue(0);
        }
    }

    for (auto resizer : m_resizers)
        resizer->setDisabled(hasErrors() || !gui());

    update();

    if (result.hasError()) {
        emit errorOccurred();
        emit ControlMonitoringManager::instance()->errorOccurred(this);
    }

    emit previewChanged();
    emit ControlMonitoringManager::instance()->previewChanged(this);
}

namespace {
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

    QImage initialPreview(const QSizeF& size)
    {
        auto min = qMin(24.0, qMin(size.width(), size.height()));

        QImage preview(
            qCeil(size.width() * DPR),
            qCeil(size.height() * DPR),
            QImage::Format_ARGB32_Premultiplied
        );

        preview.setDevicePixelRatio(DPR);
        preview.fill(Qt::transparent);

        QImage wait(":/images/wait.png");
        wait.setDevicePixelRatio(DPR);

        drawCenter(
            preview,
            wait.scaled(
                min * DPR,
                min * DPR,
                Qt::IgnoreAspectRatio,
                Qt::SmoothTransformation
            ),
            size
        );

        return preview;
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

    QList<Resizer*> initializeResizers(Control* control)
    {
        QList<Resizer*> resizers;
        int i = 0;
        for (;i < 8;)
            resizers << new Resizer(control, Resizer::Placement(i++));

        return resizers;
    }

    void setInitialProperties(Control *control)
    {
        qreal x = 0;
        qreal y = 0;
        qreal z = 0;
        qreal width = 50;
        qreal height = 50;

        if (ParserUtils::exists(control->url(), "x"))
            x = ParserUtils::property(control->url(), "x").toDouble();
        else
            x = SaveUtils::x(control->dir());

        if (ParserUtils::exists(control->url(), "y"))
            y = ParserUtils::property(control->url(), "y").toDouble();
        else
            y = SaveUtils::y(control->dir());

        if (ParserUtils::exists(control->url(), "width"))
            width = ParserUtils::property(control->url(), "width").toDouble();

        if (ParserUtils::exists(control->url(), "height"))
            height = ParserUtils::property(control->url(), "height").toDouble();

        if (ParserUtils::exists(control->url(), "z"))
            z = ParserUtils::property(control->url(), "z").toDouble();

        control->blockSignals(true);
        control->setId(SaveUtils::id(control->dir()));        
        control->setPos(x, y);
        control->setZValue(z);
        control->resize(width, height);
        control->blockSignals(false);
    }

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
}
