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
#include <scenesettings.h>
#include <hashfactory.h>

#include <QCursor>
#include <QPainter>
#include <QMimeData>
#include <QGraphicsSceneDragDropEvent>
#include <QStyleOption>

Control::Control(const QString& dir, Control* parent) : DesignerItem(parent)
  , m_gui(false)
  , m_popup(false)
  , m_window(false)
  , m_visible(true)
  , m_dir(dir)
  , m_uid(SaveUtils::controlUid(m_dir))
  , m_snapMargin(QSizeF(0, 0))
{
    setAcceptDrops(true);
    setAcceptHoverEvents(true);
    setCursor(Qt::OpenHandCursor);

    setFlag(ItemIsMovable);
    setFlag(ItemIsSelectable);
    setFlag(ItemSendsGeometryChanges);
    setFlag(ItemNegativeZStacksBehindParent);

    // Made it Qt::QueuedConnection in order to prevent
    // mouseUngrabEvent to call beingDraggedChanged or
    // beingResizedChanged, thus preventing
    // applyGeometryCorrection to be called before
    // dropControl called, since it resets geometry correction
    // otherwise may setSceneRect() in DesignerScene
    // triggers and extends scene rect
    connect(this, &Control::beingDraggedChanged,
            this, &Control::applyGeometryCorrection, Qt::QueuedConnection);
    connect(this, &Control::beingResizedChanged,
            this, &Control::applyGeometryCorrection, Qt::QueuedConnection);
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

void Control::setIndex(quint32 index)
{
    m_index = index;
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
    if (m_image.isNull())
        return;
    if (beingResized())
        painter->setClipRect(rect());
    painter->drawImage(m_outerRect, m_image);
    painter->setClipping(false);
}

void Control::paintHighlight(QPainter* painter)
{
    if (beingHighlighted()) {
        painter->setCompositionMode(QPainter::CompositionMode_SourceAtop);
        painter->fillRect(rect(), QColor(0, 0, 0, 20));
        painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
    }
}

void Control::paintOutline(QPainter* painter)
{
    if (DesignerScene::outlineMode() == DesignerScene::NoOutline)
        return;
    if (DesignerScene::outlineMode() == DesignerScene::ClippingDashLine)
        return DesignerScene::drawDashRect(painter, DesignerScene::outerRect(rect()));
    if (DesignerScene::outlineMode() == DesignerScene::BoundingDashLine)
        return DesignerScene::drawDashRect(painter, DesignerScene::outerRect(outerRect()));

    painter->setBrush(Qt::NoBrush);
    painter->setPen(DesignerScene::pen(Qt::darkGray));

    if (DesignerScene::outlineMode() == DesignerScene::ClippingSolidLine)
        return painter->drawRect(DesignerScene::outerRect(rect()));
    if (DesignerScene::outlineMode() == DesignerScene::BoundingSolidLine)
        return painter->drawRect(DesignerScene::outerRect(outerRect()));
}

void Control::paintHoverOutline(QPainter* painter, bool hovered)
{
    if (DesignerScene::showMouseoverOutline() && hovered) {
        painter->setBrush(Qt::NoBrush);
        painter->setPen(DesignerScene::pen());
        painter->drawRect(DesignerScene::outerRect(rect()));
    }
}

void Control::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget*)
{
    paintImage(painter);
    paintOutline(painter);
    paintHighlight(painter);
    paintHoverOutline(painter, option->state & QStyle::State_MouseOver);
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

    m_outerRect = result.boundingRect.isNull() ? rect() : result.boundingRect;

    if (!gui() && size() != QSizeF(40, 40)) {
        if (!hasErrors() || size().isEmpty()) {
            ControlPropertyManager::setSize(this, QSizeF(40, 40), ControlPropertyManager::NoOption);
            m_outerRect = QRectF(0, 0, 40, 40);
        }
    }

    m_image = hasErrors() ? PaintUtils::renderErrorControlImage(size(), id(), dpr) : result.image;
    if (m_image.isNull() && !gui())
        m_image = PaintUtils::renderNonGuiControlImage(ToolUtils::toolIconPath(m_dir), size(), dpr);
    if (visible() && gui() && PaintUtils::isBlankImage(m_image)) {
        m_outerRect = rect();
        m_image = PaintUtils::renderBlankControlImage(rect(), id(), dpr);
    }
    m_image.setDevicePixelRatio(dpr);

    setImage(m_image); // FIXME

    emit renderInfoChanged(result.codeChanged);
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


void Control::setOuterRect(const QRectF& outerRect)
{
    if (m_outerRect != outerRect) {
        m_outerRect = outerRect;
        update();
    }
}

void Control::setImage(const QImage& image)
{
    m_image = image;
    update();
}

QRectF Control::outerRect() const
{
    return m_outerRect;
}
