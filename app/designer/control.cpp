#include <form.h>
#include <designerscene.h>
#include <utilityfunctions.h>
#include <controlpropertymanager.h>
#include <hashfactory.h>
#include <paintutils.h>
#include <toolutils.h>

#include <QCursor>
#include <QPainter>
#include <QStyleOption>

Control::Control(Control* parent) : DesignerItem(parent)
  , m_devicePixelRatio(1)
  , m_snapMargin(QSizeF(0, 0))
{
    m_renderInfo.gui = false;
    m_renderInfo.popup = false;
    m_renderInfo.window = false;
    m_renderInfo.visible = true;

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
    return m_renderInfo.gui;
}

bool Control::form() const
{
    return type() == Form::Type;
}

bool Control::popup() const
{
    return m_renderInfo.popup;
}

bool Control::window() const
{
    return m_renderInfo.window;
}

bool Control::hasErrors() const
{
    return !m_renderInfo.errors.isEmpty();
}

bool Control::visible() const
{
    return m_renderInfo.visible;
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

Control* Control::parentControl() const
{
    return static_cast<Control*>(parentItem());
}

QVector<QmlError> Control::errors() const
{
    return m_renderInfo.errors;
}

QVector<QString> Control::events() const
{
    return m_renderInfo.events;
}

QVector<PropertyNode> Control::properties() const
{
    return m_renderInfo.properties;
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
    return UtilityFunctions::getProperty(propertyName, m_renderInfo.properties);
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

void Control::setPixmap(const QPixmap& pixmap)
{
    if (m_pixmap.cacheKey() != pixmap.cacheKey()) {
        m_pixmap = pixmap;
        m_pixmap.setDevicePixelRatio(devicePixelRatio());
        update();
    }
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

void Control::paintContent(QPainter* painter)
{
    if (m_pixmap.isNull())
        return;
    if (beingResized())
        painter->setClipRect(rect());
    QRectF r(m_renderInfo.boundingRect.topLeft(), QSizeF(m_pixmap.size()) / m_pixmap.devicePixelRatioF());
    painter->drawPixmap(r, m_pixmap, m_pixmap.rect());
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
        return DesignerScene::drawDashRect(painter, DesignerScene::outerRect(m_renderInfo.boundingRect));

    painter->setBrush(Qt::NoBrush);
    painter->setPen(DesignerScene::pen(Qt::darkGray));

    if (DesignerScene::outlineMode() == DesignerScene::ClippingSolidLine)
        return painter->drawRect(DesignerScene::outerRect(rect()));
    if (DesignerScene::outlineMode() == DesignerScene::BoundingSolidLine)
        return painter->drawRect(DesignerScene::outerRect(m_renderInfo.boundingRect));
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
    paintContent(painter);
    paintOutline(painter);
    paintHighlight(painter);
    paintHoverOutline(painter, option->state & QStyle::State_MouseOver);
}

void Control::setRenderInfo(const RenderInfo& info)
{
    if (info.uid != uid())
        return;

    m_renderInfo = info;

    setResizable(gui());
    setZValue(property("z").toDouble());
    setFlag(QGraphicsItem::ItemClipsChildrenToShape, property("clip").toBool());
    setOpacity(property("opacity").isValid() ? property("opacity").toDouble() : 1);

    if (info.codeChanged)
        m_margins = UtilityFunctions::getMarginsFromProperties(info.properties);

    if (!gui() || hasErrors()) {
        m_geometryCorrection = QRectF();
        m_geometryHash.clear();
    }

    if (gui() && (m_geometryHash.isEmpty() || info.geometryHash == m_geometryHash)) {
        m_geometryHash.clear();
        m_geometryCorrection = UtilityFunctions::getGeometryFromProperties(info.properties);
        if (!beingDragged() && !beingResized())
            applyGeometryCorrection();
    }

    if (m_renderInfo.boundingRect.isNull())
        m_renderInfo.boundingRect = rect();

    if (!gui() && size() != QSizeF(40, 40)) {
        if (!hasErrors() || size().isEmpty()) {
            ControlPropertyManager::setSize(this, QSizeF(40, 40), ControlPropertyManager::NoOption);
            m_renderInfo.boundingRect = QRectF(0, 0, 40, 40);
        }
    }

    if (hasErrors())
        m_renderInfo.image = PaintUtils::renderErrorControlImage(size(), id(), devicePixelRatio());
    if (m_renderInfo.image.isNull() && !gui())
        m_renderInfo.image = PaintUtils::renderNonGuiControlImage(ToolUtils::toolIconPath(m_dir), size(), devicePixelRatio());
    if (visible() && gui() && PaintUtils::isBlankImage(m_renderInfo.image)) {
        m_renderInfo.boundingRect = rect();
        m_renderInfo.image = PaintUtils::renderBlankControlImage(rect(), id(), devicePixelRatio());
    }

    setPixmap(QPixmap::fromImage(m_renderInfo.image));

    emit renderInfoChanged(info.codeChanged);
}

QPixmap Control::pixmap() const
{
    return m_pixmap;
}

qreal Control::devicePixelRatio() const
{
    return m_devicePixelRatio;
}

void Control::setDevicePixelRatio(const qreal& devicePixelRatio)
{
    m_devicePixelRatio = devicePixelRatio;
    update();
}

void Control::setUid(const QString& uid)
{
    m_uid = uid;
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

void Control::setBoundingRect(const QRectF& boundingRect)
{
    if (m_renderInfo.boundingRect != boundingRect) {
        m_renderInfo.boundingRect = boundingRect;
        update();
    }
}
