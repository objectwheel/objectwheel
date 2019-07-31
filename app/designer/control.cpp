#include <form.h>
#include <saveutils.h>
#include <suppressor.h>
#include <designerscene.h>
#include <controlrenderingmanager.h>
#include <controlpropertymanager.h>
#include <controlcreationmanager.h>
#include <paintutils.h>
#include <parserutils.h>
#include <utilityfunctions.h>
#include <toolutils.h>
#include <designersettings.h>
#include <scenesettings.h>
#include <hashfactory.h>

#include <QCursor>
#include <QPainter>
#include <QMimeData>
#include <QGraphicsSceneDragDropEvent>
#include <QStyleOption>

QVector<Control*> Control::m_controls;

Control::Control(const QString& dir, Control* parent) : DesignerItem(parent)
  , m_gui(false)
  , m_popup(false)
  , m_window(false)
  , m_dragIn(false)
  , m_visible(true)
  , m_dir(dir)
  , m_uid(SaveUtils::controlUid(m_dir))
  , m_snapMargin(QSizeF(0, 0))
{
    m_controls.append(this);

    setAcceptDrops(true);
    setAcceptHoverEvents(true);
    setCursor(Qt::OpenHandCursor);
    setFlag(ItemIsMovable);
    setFlag(ItemIsSelectable);
    setFlag(ItemSendsGeometryChanges);
    setFlag(QGraphicsItem::ItemNegativeZStacksBehindParent, true);

    ControlPropertyManager::setId(this, ParserUtils::id(m_dir), ControlPropertyManager::NoOption);
    ControlPropertyManager::setIndex(this, SaveUtils::controlIndex(m_dir), ControlPropertyManager::NoOption);

    // Made it Qt::QueuedConnection in order to prevent
    // ungrabMouseEvent to call beingDraggedChanged or
    // beingResizedChanged, thus preventing
    // applyGeometryCorrection to be called before
    // dropControl called, since it resets geometry correction
    // otherwise may setSceneRect() in DesignerScene
    // triggers and extends scene rect
    connect(this, &Control::beingDraggedChanged,
            this, &Control::applyGeometryCorrection, Qt::QueuedConnection);
    connect(this, &Control::beingResizedChanged,
            this, &Control::applyGeometryCorrection, Qt::QueuedConnection);

    connect(ControlRenderingManager::instance(), &ControlRenderingManager::renderDone,
            this, &Control::updateRenderInfo);
    connect(this, &Control::doubleClicked,
            this, [=] { ControlPropertyManager::instance()->doubleClicked(this); });
}

Control::~Control()
{
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

bool Control::hasErrors() const
{
    return !m_errors.isEmpty();
}

bool Control::visible() const
{
    return m_visible;
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

QVariant Control::property(const QString& propertyName) const
{
    return UtilityFunctions::getProperty(propertyName, m_properties);
}

QVector<Control*> Control::controls()
{
    return m_controls;
}

void Control::setId(const QString& id)
{
    if (m_id != id) {
        m_id = id;
        setToolTip(id);
        setObjectName(id);
        update();
    }
}

void Control::setDir(const QString& dir)
{
    m_dir = dir;
}

void Control::setDragIn(bool dragIn)
{
    m_dragIn = dragIn;
    update();
}

void Control::setIndex(quint32 index)
{
    m_index = index;
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

    ControlPropertyManager::Options options = ControlPropertyManager::SaveChanges
            | ControlPropertyManager::CompressedCall;

    if (control->gui())
        options |= ControlPropertyManager::UpdateRenderer;
    // NOTE: Do not move this assignment below setParent,
    // because parent change effects the newPos result
    control->m_geometryCorrection = QRectF();
    control->m_geometryHash = HashFactory::generate();
    const QPointF& newPos = DesignerScene::snapPosition(control->mapToItem(this, QPointF()));
    ControlPropertyManager::setParent(control, this, ControlPropertyManager::SaveChanges
                                      | ControlPropertyManager::UpdateRenderer);
    ControlPropertyManager::setPos(control, newPos, options, control->m_geometryHash);
    // NOTE: We compress setPos because there might be some other
    // compressed setPos'es in the list, We want the setPos that
    // happens after reparent operation to take place at the very last
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
        RenderResult result;
        UtilityFunctions::pull(mimeData->data(QStringLiteral("application/x-objectwheel-tool")), dir);
        UtilityFunctions::pull(mimeData->data(QStringLiteral("application/x-objectwheel-render-result")), result);

        if (scene())
            scene()->clearSelection();
        // NOTE: Use actual Control position for scene, since createControl deals with margins
        auto newControl = ControlCreationManager::createControl(
                    this, dir, DesignerScene::snapPosition(event->pos() - QPointF(5, 5)),
                    result.boundingRect.size(), result.image);
        if (newControl) {
            newControl->setSelected(true);
        } else {
            UtilityFunctions::showMessage(0, tr("Oops"),
                                          tr("Operation failed, control has got problems."),
                                          QMessageBox::Critical);
        }
        update();
    }
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
    Q_UNUSED(event)
    m_dragIn = false;
    update();
}

void Control::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    DesignerItem::mouseMoveEvent(event);

    if (!dragAccepted())
        return;

    if (!scene())
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

void Control::ungrabMouseEvent(QEvent* event)
{
    if (!scene())
        return DesignerItem::ungrabMouseEvent(event);

    // FIXME: items may also contain a form
    const QList<DesignerItem*>& items = scene()->draggedResizedSelectedItems();
    DesignerItem::ungrabMouseEvent(event); // Clears beingDragged state

    for (auto control : scene()->currentForm()->childControls()) {
        if (control->dragIn() && parentControl() != control) {
            for (auto sc : items) {
                if (sc->parentItem() != control)
                    control->dropControl((Control*)sc);
            }
            scene()->clearSelection();
            control->setSelected(true);
        }
        control->setDragIn(false);
    }

    if (scene()->currentForm()->dragIn() && parentItem() != scene()->currentForm()) {
        scene()->currentForm()->dropControl(this);
        scene()->clearSelection();
        scene()->currentForm()->setSelected(true);
    }
    scene()->currentForm()->setDragIn(false);
}

QVariant Control::itemChange(int change, const QVariant& value)
{    
    if (change == ItemPositionChange && beingDragged()) {
        const QPointF& snapPos = DesignerScene::snapPosition(value.toPointF());
        const QPointF& snapMargin = value.toPointF() - snapPos;
        m_snapMargin = QSizeF(snapMargin.x(), snapMargin.y());
        if (!form()) {
            ControlPropertyManager::Options options = ControlPropertyManager::SaveChanges
                    | ControlPropertyManager::CompressedCall
                    | ControlPropertyManager::DontApplyDesigner;
            if (gui()) {
                m_geometryCorrection = QRectF();
                m_geometryHash = HashFactory::generate();
                options |= ControlPropertyManager::UpdateRenderer;
            }
            ControlPropertyManager::setPos(this, snapPos, options, m_geometryHash);
        }
        return snapPos;
    } else if (change == ItemSizeChange && beingResized()) {
        const QSizeF snapSize = DesignerScene::snapSize(pos(), value.toSizeF() + m_snapMargin);
        m_snapMargin = QSizeF(0, 0);
        if (gui()) {
            m_geometryCorrection = QRectF();
            m_geometryHash = HashFactory::generate();
            ControlPropertyManager::setSize(this, snapSize, ControlPropertyManager::SaveChanges
                                            | ControlPropertyManager::UpdateRenderer
                                            | ControlPropertyManager::CompressedCall
                                            | ControlPropertyManager::DontApplyDesigner, m_geometryHash);
        }
        return snapSize;
    }

    return DesignerItem::itemChange(change, value);
}

void Control::paintImage(QPainter* painter)
{
    if (beingResized())
        painter->setClipRect(rect());
    painter->drawImage(m_frame.toRect(), m_image);
    painter->setClipping(false);
}

void Control::paintHighlight(QPainter* painter)
{
    painter->setCompositionMode(QPainter::CompositionMode_SourceAtop);
    painter->fillRect(rect(), QColor(0, 0, 0, 15));
    painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
}

void Control::paintHoverOutline(QPainter* painter)
{
    if (scene()) {
        painter->setBrush(Qt::NoBrush);
        painter->setPen(DesignerScene::pen());
        painter->drawRect(scene()->outerRect(rect()));
    }
}

void Control::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget*)
{
    const SceneSettings* settings = DesignerSettings::sceneSettings();

    if (!image().isNull())
        paintImage(painter);

    if (settings->controlOutline != 0 && scene())
        scene()->paintOutline(painter, scene()->outerRect(settings->controlOutline == 1 ? rect() : frame()));

    if (settings->showMouseoverOutline && option->state & QStyle::State_MouseOver)
        paintHoverOutline(painter);

    if (dragIn())
        paintHighlight(painter);
}

void Control::updateRenderInfo(const RenderResult& result)
{
    if (result.uid != uid())
        return;

    const qreal dpr = ControlRenderingManager::devicePixelRatio();
    m_gui = result.gui;
    m_popup = result.popup;
    m_window = result.window;
    m_visible = result.visible;
    m_errors = result.errors;
    m_events = result.events;
    m_properties = result.properties;

    setResizable(gui());
    setZValue(property("z").toDouble());
    setFlag(QGraphicsItem::ItemClipsChildrenToShape, property("clip").toBool());
    if (property("opacity").isValid())
        setOpacity(property("opacity").toDouble());
    else
        setOpacity(1.0);

    if (result.codeChanged)
        m_margins = UtilityFunctions::getMarginsFromProperties(result.properties);

    if (!gui() || hasErrors()) {
        m_geometryCorrection = QRectF();
        m_geometryHash.clear();
    }

    if (gui() && (m_geometryHash.isEmpty() || result.geometryHash == m_geometryHash)) {
        m_geometryHash.clear();
        m_geometryCorrection = UtilityFunctions::getGeometryFromProperties(result.properties);
        if (!beingDragged() && !beingResized())
            applyGeometryCorrection();
    }

    m_frame = result.boundingRect.isNull() ? rect() : result.boundingRect;

    if (!gui() && size() != QSizeF(40, 40)) {
        if (!hasErrors() || size().isEmpty()) {
            ControlPropertyManager::setSize(this, QSizeF(40, 40), ControlPropertyManager::NoOption);
            m_frame = QRectF(0, 0, 40, 40);
        }
    }

    m_image = hasErrors() ? PaintUtils::renderErrorControlImage(size(), id(), dpr) : result.image;
    if (m_image.isNull() && !gui())
        m_image = PaintUtils::renderNonGuiControlImage(ToolUtils::toolIconPath(m_dir), size(), dpr);
    if (visible() && gui() && PaintUtils::isBlankImage(m_image)) {
        m_frame = rect();
        m_image = PaintUtils::renderBlankControlImage(rect(), id(), dpr);
    }
    m_image.setDevicePixelRatio(dpr);

    update();

    ControlPropertyManager::instance()->renderInfoChanged(this, result.codeChanged);
}

void Control::applyGeometryCorrection()
{
    if (m_geometryCorrection.isNull() || !gui() || beingDragged() || beingResized())
        return;

    ControlPropertyManager::setSize(this, m_geometryCorrection.size(), ControlPropertyManager::NoOption);

    if (!form()) {
        ControlPropertyManager::setPos(
                    this, ControlPropertyManager::posWithMargin(this, m_geometryCorrection.topLeft(), true),
                    ControlPropertyManager::NoOption);
    }

    m_geometryCorrection = QRectF();
}


void Control::setFrame(const QRectF& frame)
{
    m_frame = frame;
    update();
}

void Control::setImage(const QImage& image)
{
    m_image = image;
    update();
}

QRectF Control::frame() const
{
    return m_frame;
}
