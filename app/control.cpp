#include <form.h>
#include <resizer.h>
#include <saveutils.h>
#include <suppressor.h>
#include <filemanager.h>
#include <designerscene.h>
#include <controlpreviewingmanager.h>
#include <windowmanager.h>
#include <mainwindow.h>
#include <inspectorpane.h>
#include <propertiespane.h>
#include <centralwidget.h>
#include <issuesbox.h>
#include <designerwidget.h>
#include <controlpropertymanager.h>
#include <paintutils.h>
#include <parserutils.h>

#include <QtMath>
#include <QCursor>
#include <QPainter>
#include <QMimeData>
#include <QRegularExpression>
#include <QGraphicsSceneDragDropEvent>
#include <QApplication>

extern const char* TOOL_KEY;

namespace {

QRectF getGeometryFromProperties(const QList<PropertyNode>& properties)
{
    QRectF geometry;
    for (const PropertyNode& propertyNode : properties) {
        for (const QString& propertyName : propertyNode.properties.keys()) {
            if (propertyName == "x")
                geometry.moveLeft(propertyNode.properties.value(propertyName).toReal());
            else if (propertyName == "y")
                geometry.moveTop(propertyNode.properties.value(propertyName).toReal());
            else if (propertyName == "width")
                geometry.setWidth(propertyNode.properties.value(propertyName).toReal());
            else if (propertyName == "height")
                geometry.setHeight(propertyNode.properties.value(propertyName).toReal());
        }
    }
    return geometry;
}

QList<Resizer*> initializeResizers(Control* control)
{
    QList<Resizer*> resizers;
    for (int i = 0; i < 8; ++i)
        resizers.append(new Resizer(control, Resizer::Placement(i)));
    return resizers;
}
}

QList<Control*> Control::m_controls;
const QSizeF Control::BASE_SIZE = {40.0, 40.0};
Control::Control(const QString& url, Control* parent) : QGraphicsWidget(parent)
  , m_gui(false)
  , m_clip(false)
  , m_window(false)
  , m_dragIn(false)
  , m_hoverOn(false)
  , m_dragging(false)
  , m_resizing(false)
  , m_url(url)
  , m_uid(SaveUtils::uid(dir()))
  , m_image(PaintUtils::renderInitialControlImage(BASE_SIZE))
  , m_resizers(initializeResizers(this))
{
    m_controls << this;

    setAcceptDrops(true);
    setAcceptHoverEvents(true);
    setFlag(ItemIsMovable);
    setFlag(ItemIsFocusable);
    setFlag(ItemIsSelectable);
    setFlag(ItemSendsGeometryChanges);

    ControlPropertyManager::setId(this, ParserUtils::id(m_url), ControlPropertyManager::NoOption);
    ControlPropertyManager::setSize(this, BASE_SIZE, ControlPropertyManager::NoOption);

    connect(ControlPreviewingManager::instance(), &ControlPreviewingManager::previewDone,
            this, &Control::updatePreview);

    connect(this, &Control::resizingChanged,
            this, &Control::applyCachedGeometry);
    connect(this, &Control::draggingChanged,
            this, &Control::applyCachedGeometry);
}

Control::~Control()
{
    // In order to prevent any kind of signals being emitted while deletion in progress
    blockSignals(true);
    m_controls.removeOne(this);
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

bool Control::window() const
{
    return m_window;
}

bool Control::dragIn() const
{
    return m_dragIn;
}

bool Control::dragging() const
{
    return m_dragging;
}

bool Control::resizing() const
{
    return m_resizing;
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
    return SaveUtils::toParentDir(m_url);
}

DesignerScene* Control::scene() const
{
    return static_cast<DesignerScene*>(QGraphicsWidget::scene());
}

Control* Control::parentControl() const
{
    return dynamic_cast<Control*>(parentItem());
}

const QList<QQmlError>& Control::errors() const
{
    return m_errors;
}

const QList<QString>& Control::events() const
{
    return m_events;
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

    // FIXME:
    Suppressor::suppress(150, "draggingChanged", std::bind(&Control::draggingChanged, this));
    //    emit draggingChanged();
}

void Control::setResizing(bool resizing)
{
    m_resizing = resizing;
    // FIXME:
    Suppressor::suppress(150, "resizingChanged", std::bind(&Control::resizingChanged, this));
    //    emit resizingChanged();
}

void Control::updateUid()
{
    m_uid = SaveUtils::uid(dir());
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

void Control::updateUids()
{
    for (Control* control : m_controls)
        control->updateUid();
}

QVariant::Type Control::propertyType(const QString& propertyName) const
{
    for (const PropertyNode& propertyNode : m_properties) {
        const QMap<QString, QVariant>& propertyMap = propertyNode.properties;
        for (const QString& property : propertyMap.keys()) {
            const QVariant& propertyValue = propertyMap.value(property);
            if (property == propertyName)
                return propertyValue.type();
        }
    }
    return QVariant::Invalid;
}

void Control::dropControl(Control* control)
{
    Q_ASSERT(!control->form());

    const QPointF& newPos = mapFromItem(control->parentItem(), control->pos());
    ControlPropertyManager::setParent(control, this, ControlPropertyManager::SaveChanges
                                      | ControlPropertyManager::UpdatePreviewer);
    ControlPropertyManager::setPos(control, newPos,  ControlPropertyManager::SaveChanges
                                   | ControlPropertyManager::UpdatePreviewer
                                   | ControlPropertyManager::CompressedCall);
    // NOTE: We compress setPos because there might be some other compressed setPos'es in the list
    // We want the setPos that is happened after reparent operation to take place at the very last

    //  FIXME:  ControlMonitoringManager::instance()->geometryChanged(control);
    //  WindowManager::mainWindow()->inspectorPane()->handleControlParentChange(control);

    update();
}

void Control::dropEvent(QGraphicsSceneDragDropEvent* event)
{
    m_dragIn = false;
    event->accept();
    WindowManager::mainWindow()->centralWidget()->designerWidget()->onControlDrop(
                this, event->pos(),
                event->mimeData()->urls().first().toLocalFile());
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

void Control::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsWidget::mouseMoveEvent(event);

    Control* control = nullptr;
    const QList<Control*>& controlsUnderCursor = scene()->controlsAt(event->scenePos());

    if (controlsUnderCursor.size() > 1)
        control = controlsUnderCursor.at(1);

    if (control && control != this && !control->dragIn() && m_dragging) {
        control->setDragIn(true);

        for (Control* childControls : scene()->currentForm()->childControls())
            if (childControls != control)
                childControls->setDragIn(false);

        if (scene()->currentForm() != control)
            scene()->currentForm()->setDragIn(false);
    }

    if (event->button() == Qt::MidButton)
        event->ignore();
    else
        event->accept();

    if (!form()) {
        ControlPropertyManager::setPos(this, pos(), ControlPropertyManager::SaveChanges
                                       | ControlPropertyManager::UpdatePreviewer
                                       | ControlPropertyManager::CompressedCall);
    }
}

void Control::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsWidget::mousePressEvent(event);

    if (event->button() == Qt::MidButton)
        event->ignore();
    else
        event->accept();
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
    WindowManager::mainWindow()->centralWidget()->designerWidget()->onControlDoubleClick(this);
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
}

void Control::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
{
    if (m_image.isNull())
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
    painter->drawImage(rect(), m_image, QRectF(QPointF(0, 0), size() * qApp->devicePixelRatio()));

    QLinearGradient gradient(rect().center().x(), rect().y(), rect().center().x(), rect().bottom());
    gradient.setColorAt(0, QColor("#174C4C4C").lighter(110));
    gradient.setColorAt(1, QColor("#174C4C4C").darker(120));

    if (m_dragIn) {
        if (scene()->showOutlines()) {
            painter->fillRect(rect(), gradient);
        } else {
            QImage highlight(m_image);
            QPainter p(&highlight);
            p.setCompositionMode(QPainter::CompositionMode_SourceAtop);
            p.fillRect(m_image.rect(), gradient);
            p.end();
            painter->drawImage(rect(), highlight, QRectF(QPointF(0, 0), size() * qApp->devicePixelRatio()));
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

// FIXME
void Control::updatePreview(const PreviewResult& result)
{
    if (result.uid != uid())
        return;

    ControlPropertyManager::setId(this, result.id, ControlPropertyManager::NoOption);

    m_errors = result.errors;
    m_gui = result.gui;
    m_window = result.window;
    m_events = result.events;
    m_properties = result.properties;

    m_cachedGeometry = getGeometryFromProperties(result.properties);
    if (!dragging() && !resizing())
        applyCachedGeometry();

    m_image = hasErrors()
            ? PaintUtils::renderErrorControlImage(size())
            : result.image;

    if (m_image.isNull()) {
        if (m_gui)
            m_image = PaintUtils::renderInvisibleControlImage(size());
        else
            m_image = PaintUtils::renderNonGuiControlImage(SaveUtils::toIcon(dir()), size());
    }

    //    if (!result.errors.isEmpty()) {
    //        //        setRefreshingDisabled(true);
    //        resize(QSizeF(50, 50));
    //        //        setRefreshingDisabled(false);
    //        setPos(pos());
    //        //    FIXME:    ControlMonitoringManager::instance()->geometryChanged(this);
    //        setZValue(0);
    //    } else {
    //        if (result.gui) {
    //            if (!form())
    //                //                m_clip = result.property("clip").toBool();

    //                if (!m_dragging && !Resizer::resizing()/* BUG && !ControlPreviewingManager::contains(uid())*/) {
    //                    const auto& rect = getGeometryFromProperties(result.properties);
    //                    qreal z = getZ(result);
    //                    resize(rect.size());
    //                    //                    setRefreshingDisabled(true);
    //                    if (!form())
    //                        setPos(rect.topLeft());
    //                    blockSignals(true);
    //                    setZValue(z);
    //                    blockSignals(false);
    //                    //                    setRefreshingDisabled(false);
    //                }
    //        } else {
    //            //            setRefreshingDisabled(true);
    //            resize(QSizeF(50, 50));
    //            //            setRefreshingDisabled(false);
    //            setPos(pos());
    //            //     FIXME:       ControlMonitoringManager::instance()->geometryChanged(this);
    //            setZValue(0);
    //        }
    //    }

    for (auto resizer : m_resizers)
        resizer->setDisabled(!gui());

    update();

    //    if (!result.errors.isEmpty())
    //        WindowManager::mainWindow()->centralWidget()->outputPane()->issuesBox()->handleErrors(this);
    //    if (isSelected())
    //        WindowManager::mainWindow()->propertiesPane()->refreshList();
    //    WindowManager::mainWindow()->inspectorPane()->handleControlPreviewChange(this);
    ControlPropertyManager::instance()->previewChanged(this, result.codeChanged);
}

void Control::applyCachedGeometry()
{
    if (!dragging() && !resizing()) {
        if (form()) {
            ControlPropertyManager::setSize(this, m_cachedGeometry.size(),
                                            ControlPropertyManager::NoOption);
        } else {
            ControlPropertyManager::setGeometry(this, m_cachedGeometry,
                                                ControlPropertyManager::NoOption);
        }
    }
}
