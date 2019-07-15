#include <form.h>
#include <resizeritem.h>
#include <saveutils.h>
#include <suppressor.h>
#include <designerscene.h>
#include <controlrenderingmanager.h>
#include <controlpropertymanager.h>
#include <paintutils.h>
#include <parserutils.h>
#include <utilityfunctions.h>
#include <toolutils.h>
#include <windowmanager.h>
#include <centralwidget.h>
#include <designerview.h>
#include <mainwindow.h>
#include <designersettings.h>
#include <scenesettings.h>
#include <headlineitem.h>
#include <sceneextenditem.h>

#include <QCursor>
#include <QPainter>
#include <QMimeData>
#include <QGraphicsSceneDragDropEvent>
#include <QStyleOption>

QVector<Control*> Control::m_controls;

Control::Control(const QString& dir, Control* parent) : DesignerItem(parent)
  , m_gui(false)
  , m_clip(false)
  , m_popup(false)
  , m_window(false)
  , m_dragIn(false)
  , m_resized(false)
  , m_dir(dir)
  , m_uid(SaveUtils::controlUid(m_dir))
  , m_image(PaintUtils::renderInitialControlImage({40, 40}, ControlRenderingManager::devicePixelRatio()))
  , m_headlineItem(new HeadlineItem(this))
{
    m_controls.append(this);
    new SceneExtendItem(this);

    setAcceptDrops(true);
    setAcceptHoverEvents(true);
    setFlag(ItemIsMovable);
    setFlag(ItemIsSelectable);
    setFlag(ItemSendsGeometryChanges);

    initResizers();
    headlineItem()->setPen(QPen(Qt::white));
    headlineItem()->setBrush(DesignerScene::outlineColor());

    ControlPropertyManager::setId(this, ParserUtils::id(m_dir), ControlPropertyManager::NoOption);
    ControlPropertyManager::setIndex(this, SaveUtils::controlIndex(m_dir), ControlPropertyManager::NoOption);
    ControlPropertyManager::setSize(this, {40, 40}, ControlPropertyManager::NoOption);

    connect(ControlRenderingManager::instance(), &ControlRenderingManager::renderDone,
            this, &Control::updateImage);
    connect(this, &Control::resizedChanged,
            this, &Control::applyCachedGeometry);
    connect(this, &Control::beingDraggedChanged,
            this, &Control::applyCachedGeometry, Qt::QueuedConnection);
    //    Suppressor::suppress(150, "draggingChanged", std::bind(&Control::draggingChanged, this));
    connect(this, &Control::doubleClicked,
            this, [=] { WindowManager::mainWindow()->centralWidget()->designerView()->onControlDoubleClick(this); });
    connect(headlineItem(), &HeadlineItem::doubleClicked,
            this, [=] { WindowManager::mainWindow()->centralWidget()->designerView()->onControlDoubleClick(this); });
    connect(this, &Control::geometryChanged,
            headlineItem(), &HeadlineItem::scheduleSizeUpdate);
    connect(this, &Control::geometryChanged,
            this, [=] {
        for (ResizerItem* resizer : m_resizers)
            resizer->updatePosition();
    });
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
    return type() == Form::Type;
}

bool Control::clip() const
{
    return m_clip;
}

bool Control::popup() const
{
    return m_popup;
}

bool Control::window() const
{
    return m_window;
}

bool Control::dragIn() const
{
    return m_dragIn;
}

bool Control::resized() const
{
    return m_resized;
}

bool Control::hasErrors() const
{
    return !m_errors.isEmpty();
}

quint32 Control::index() const
{
    return m_index;
}

int Control::type() const
{
    return Type;
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
    return m_dir;
}

QMarginsF Control::margins() const
{
    return m_margins;
}

QImage Control::image() const
{
    return m_image;
}

Control* Control::parentControl() const
{
    return static_cast<Control*>(parentItem());
}

HeadlineItem* Control::headlineItem() const
{
    return m_headlineItem;
}

QVector<QmlError> Control::errors() const
{
    return m_errors;
}

QVector<QString> Control::events() const
{
    return m_events;
}

QVector<PropertyNode> Control::properties() const
{
    return m_properties;
}

QList<Control*> Control::siblings() const
{
    QList<Control*> siblings;
    if (form()) {
        if (const DesignerScene* scene = this->scene()) {
            for (Form* form : scene->forms())
                siblings.append(form);
        }
    } else {
        if (const Control* parent = parentControl())
            siblings = parent->childControls(false);
    }
    Q_ASSERT(siblings.contains(const_cast<Control*>(this)));
    siblings.removeOne(const_cast<Control*>(this));
    std::sort(siblings.begin(), siblings.end(), [] (const Control* left, const Control* right) {
        return left->index() < right->index();
    });
    return siblings;
}

QList<Control*> Control::childControls(bool recursive) const
{
    QList<Control*> controls;

    for (DesignerItem* item : childItems()) {
        if (item->type() == Form::Type || item->type() == Control::Type)
            controls.append(static_cast<Control*>(item));
    }

    std::sort(controls.begin(), controls.end(), [] (const Control* left, const Control* right) {
        return left->index() < right->index();
    });

    if (recursive) {
        const int SIBLINGS_COUNT = controls.size();
        for (int i = 0; i < SIBLINGS_COUNT; ++i)
            controls.append(controls.at(i)->childControls(true));
    }

    return controls;
}

QVector<Control*> Control::controls()
{
    return m_controls;
}

void Control::setClip(bool clip)
{
    m_clip = clip;
    update();
}

void Control::setId(const QString& id)
{
    m_id = id;
    m_headlineItem->setText(id);
    setToolTip(id);
}

void Control::setDir(const QString& dir)
{
    m_dir = dir;
}

void Control::setDragIn(bool dragIn)
{
    m_dragIn = dragIn;
}

void Control::setResized(bool resized)
{
    m_resized = resized;
    // FIXME:
    Suppressor::suppress(150, "resizedChanged", std::bind(&Control::resizedChanged, this));
    //    emit resizedChanged();
}

void Control::setIndex(quint32 index)
{
    m_index = index;
}

QRectF Control::outerRect(const QRectF& rect) const
{
    return rect.adjusted(-0.5 / scene()->zoomLevel(), -0.5 / scene()->zoomLevel(), 0, 0);
}

QVariant::Type Control::propertyType(const QString& propertyName) const
{
    for (const PropertyNode& propertyNode : m_properties) {
        for (const QString& property : propertyNode.properties.keys()) {
            const QVariant& propertyValue = propertyNode.properties.value(property);
            if (property == propertyName)
                return propertyValue.type();
        }
    }
    return QVariant::Invalid;
}

void Control::dropControl(Control* control)
{
    Q_ASSERT(!control->form());

    // NOTE: Do not move this assignment below setParent, because parent change effects the newPos result
    const QPointF& newPos = mapFromItem(control->parentItem(), control->pos());
    ControlPropertyManager::setParent(control, this, ControlPropertyManager::SaveChanges
                                      | ControlPropertyManager::UpdateRenderer);
    ControlPropertyManager::setPos(control, newPos, ControlPropertyManager::SaveChanges
                                   | ControlPropertyManager::UpdateRenderer
                                   | ControlPropertyManager::CompressedCall);
    // NOTE: We compress setPos because there might be some other compressed setPos'es in the list
    // We want the setPos that happens after reparent operation to take place at the very last

    update();
}

void Control::dropEvent(QGraphicsSceneDragDropEvent* event)
{
    // FIXME: Move this out of Control.cpp. Move everything unrelated with a Control
    // to DesignerView or DesignerScene or somewhere else that is related to.
    const QMimeData* mimeData = event->mimeData();
    if (mimeData->hasFormat(QStringLiteral("application/x-objectwheel-tool"))) {
        m_dragIn = false;
        event->acceptProposedAction();
        QString dir;
        UtilityFunctions::pull(mimeData->data(QStringLiteral("application/x-objectwheel-tool")), dir);
        Q_ASSERT(!dir.isEmpty());
        WindowManager::mainWindow()->centralWidget()->designerView()->onControlDrop(
                    this, dir, event->pos() - QPointF(5, 5));
        update();
    }
}

void Control::dragMoveEvent(QGraphicsSceneDragDropEvent* event)
{
    event->accept();
}

void Control::dragEnterEvent(QGraphicsSceneDragDropEvent* event)
{
    if (event->mimeData()->hasFormat(QStringLiteral("application/x-objectwheel-tool"))) {
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

void Control::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    DesignerItem::mousePressEvent(event);
}

void Control::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    DesignerItem::mouseMoveEvent(event);

    if (!startDragDistanceExceeded())
        return;

    Control* control = nullptr;
    const QList<Control*>& controlsUnderCursor = scene()->controlsAt(event->scenePos());

    if (controlsUnderCursor.size() > 1)
        control = controlsUnderCursor.at(1);

    if (control && control != this && !control->dragIn() && beingDragged()) {
        control->setDragIn(true);

        for (Control* childControls : scene()->currentForm()->childControls())
            if (childControls != control)
                childControls->setDragIn(false);

        if (scene()->currentForm() != control)
            scene()->currentForm()->setDragIn(false);
    }
}

void Control::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    DesignerItem::mouseReleaseEvent(event);

    auto selectedControls = scene()->selectedControls();
    selectedControls.removeOne(scene()->currentForm());

    for (auto control : scene()->currentForm()->childControls()) {
        if (control->dragIn() && beingDragged() && parentControl() != control) {
            for (auto sc : selectedControls) {
                if (sc->beingDragged())
                    control->dropControl(sc);
            }
            scene()->clearSelection();
            control->setSelected(true);
        }
        control->setDragIn(false);
    }

    if (scene()->currentForm()->dragIn() && beingDragged() &&
            parentControl() != scene()->currentForm()) {
        scene()->currentForm()->dropControl(this);
        scene()->clearSelection();
        scene()->currentForm()->setSelected(true);
    }
    scene()->currentForm()->setDragIn(false);
}

QVariant Control::itemChange(int change, const QVariant& value)
{    
    if (change == ItemSelectedHasChanged) {
        bool selected = value.toBool();
        for (ResizerItem* resizer : m_resizers)
            resizer->setVisible(selected);
        if (type() == Control::Type && !selected)
            m_headlineItem->setVisible(false);
    } else if (change == ItemPositionChange && beingDragged()) {
        const QPointF& snapPos = scene()->snapPosition(value.toPointF());
        if (!form()) {
            ControlPropertyManager::setPos(this, snapPos, ControlPropertyManager::SaveChanges
                                           | ControlPropertyManager::UpdateRenderer
                                           | ControlPropertyManager::CompressedCall
                                           | ControlPropertyManager::DontApplyDesigner);
        }
        return snapPos;
    } else if (change == ItemSizeChange && beingResized()) {
        const QSizeF& snapSize = scene()->snapSize(pos(), value.toSizeF());
        ControlPropertyManager::setSize(this, snapSize, ControlPropertyManager::SaveChanges
                                        | ControlPropertyManager::UpdateRenderer
                                        | ControlPropertyManager::CompressedCall
                                        | ControlPropertyManager::DontApplyDesigner);
        return snapSize;
    }

    return DesignerItem::itemChange(change, value);
}

void Control::restrainPaintRegion(QPainter* painter)
{
    const QRectF& boundingRect = m_frame.adjusted(-1, -1, 1, 1);
    const QRectF& parentRect = parentControl()->rect().adjusted(1, 1, -1, -1);
    const QRectF& parentGeometry = parentControl()->mapToItem(this, parentRect).boundingRect();
    painter->setClipRect(boundingRect.intersected(parentGeometry));
}

void Control::paintImage(QPainter* painter)
{
    painter->drawImage(m_frame.toRect(), m_image);
}

void Control::paintHighlight(QPainter* painter)
{
    painter->setCompositionMode(QPainter::CompositionMode_SourceAtop);
    painter->fillRect(rect(), QColor(0, 0, 0, 15));
    painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
}

void Control::paintHoverOutline(QPainter* painter)
{
    painter->setBrush(Qt::NoBrush);
    painter->setPen(scene()->pen());
    painter->drawRect(outerRect(rect()));
}

void Control::paintSelectionOutline(QPainter* painter)
{
    painter->setPen(scene()->pen(scene()->outlineColor(), 2));
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(rect());
}

void Control::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget*)
{
    const SceneSettings* settings = DesignerSettings::sceneSettings();

    if (parentControl() && parentControl()->clip() && !beingDragged())
        restrainPaintRegion(painter);

    if (!image().isNull())
        paintImage(painter);

    if (settings->controlOutline != 0)
        scene()->paintOutline(painter, outerRect(settings->controlOutline == 1 ? rect() : frame()));

    if (settings->showMouseoverOutline && option->state & QStyle::State_MouseOver)
        paintHoverOutline(painter);

    if (isSelected())
        paintSelectionOutline(painter);

    if (dragIn())
        paintHighlight(painter);
}

void Control::updateImage(const RenderResult& result)
{
    if (result.uid != uid())
        return;

    m_errors = result.errors;
    m_gui = result.gui;
    m_popup = result.popup;
    m_window = result.window;
    m_events = result.events;
    m_properties = result.properties;
    setClip(UtilityFunctions::getProperty("clip", result.properties).toBool());

    if (result.codeChanged)
        m_margins = UtilityFunctions::getMarginsFromProperties(result.properties);
    m_cachedGeometry = UtilityFunctions::getGeometryFromProperties(result.properties);

    if (!beingDragged() && !resized())
        applyCachedGeometry();

    m_frame = result.boundingRect.isNull() ? rect() : result.boundingRect;
    m_image = hasErrors()
            ? PaintUtils::renderErrorControlImage(size(), ControlRenderingManager::devicePixelRatio())
            : result.image;
    m_image.setDevicePixelRatio(ControlRenderingManager::devicePixelRatio());

    if (m_image.isNull() && !m_gui) {
        m_image = PaintUtils::renderNonGuiControlImage(
                    ToolUtils::toolIconPath(m_dir), size(),
                    ControlRenderingManager::devicePixelRatio());
    }

    for (auto resizer : m_resizers)
        resizer->setEnabled(gui());

    update();

    ControlPropertyManager::instance()->imageChanged(this, result.codeChanged);
}

void Control::applyCachedGeometry()
{
    if (!beingDragged() && !resized()) {
        if (form()) {
            ControlPropertyManager::setSize(this, m_cachedGeometry.size(),
                                            ControlPropertyManager::NoOption);
        } else {
            ControlPropertyManager::setGeometry(
                        this, ControlPropertyManager::geoWithMargin(this, m_cachedGeometry, true),
                        ControlPropertyManager::NoOption);
        }
    }
}

void Control::initResizers()
{
    for (int i = 0; i < 8; ++i) {
        auto resizer = new ResizerItem(ResizerItem::Placement(i), this);
        resizer->setPen(DesignerScene::pen());
        m_resizers.append(resizer);
    }
}

QRectF Control::frame() const
{
    return m_frame;
}
