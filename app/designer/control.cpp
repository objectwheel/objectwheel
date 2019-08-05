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
  , m_geometrySyncEnabled(false)
{
    m_renderInfo.gui = false;
    m_renderInfo.popup = false;
    m_renderInfo.window = false;
    m_renderInfo.visible = true;

    setResizable(true);
    setAcceptHoverEvents(true);
    setCursor(Qt::OpenHandCursor);

    setFlag(ItemIsMovable);
    setFlag(ItemIsSelectable);
    setFlag(ItemSendsGeometryChanges);
    setFlag(ItemNegativeZStacksBehindParent);
}

int Control::type() const
{
    return Type;
}

bool Control::gui() const
{
    return m_renderInfo.gui;
}

bool Control::popup() const
{
    return m_renderInfo.popup;
}

bool Control::window() const
{
    return m_renderInfo.window;
}

bool Control::visible() const
{
    return m_renderInfo.visible;
}

bool Control::hasErrors() const
{
    return !m_renderInfo.errors.isEmpty();
}

QMarginsF Control::margins() const
{
    return m_renderInfo.margins;
}

QVector<QString> Control::events() const
{
    return m_renderInfo.events;
}

QVector<QmlError> Control::errors() const
{
    return m_renderInfo.errors;
}

QVector<PropertyNode> Control::properties() const
{
    return m_renderInfo.properties;
}

quint32 Control::index() const
{
    return m_index;
}

void Control::setIndex(quint32 index)
{
    m_index = index;
}

QString Control::id() const
{
    return m_id;
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

QString Control::uid() const
{
    return m_uid;
}

void Control::setUid(const QString& uid)
{
    m_uid = uid;
}

QString Control::dir() const
{
    return m_dir;
}

void Control::setDir(const QString& dir)
{
    m_dir = dir;
}

QPixmap Control::pixmap() const
{
    return m_pixmap;
}

void Control::setPixmap(const QPixmap& pixmap)
{
    if (m_pixmap.cacheKey() != pixmap.cacheKey()) {
        m_pixmap = pixmap;
        m_pixmap.setDevicePixelRatio(devicePixelRatio());
        update();
    }
}

qreal Control::devicePixelRatio() const
{
    return m_devicePixelRatio;
}

void Control::setDevicePixelRatio(const qreal& devicePixelRatio)
{
    if (m_devicePixelRatio != devicePixelRatio) {
        m_devicePixelRatio = devicePixelRatio;
        m_pixmap.setDevicePixelRatio(devicePixelRatio);
        update();
    }
}

RenderInfo Control::renderInfo() const
{
    return m_renderInfo;
}

void Control::setRenderInfo(const RenderInfo& info)
{
    if (m_uid != info.uid)
        return;

    m_renderInfo = info;

    setResizable(gui());
    setZValue(property("z").toDouble());
    setFlag(ItemClipsChildrenToShape, property("clip").toBool());
    setOpacity(property("opacity").isValid() ? property("opacity").toDouble() : 1);

    for (Control* childControl : childControls(false))
        childControl->setTransform(QTransform::fromTranslate(margins().left(), margins().top()));

    if (gui()) {
        if (m_geometrySyncKey.isEmpty() || m_geometrySyncKey == info.geometrySyncKey) {
            m_geometrySyncKey.clear();
            setGeometrySyncEnabled(true);
            syncGeometry();
        }

        if (visible() && PaintUtils::isBlankImage(m_renderInfo.image))
            m_renderInfo.image = PaintUtils::renderBlankControlImage(rect(), id(), devicePixelRatio());
    } else {
        setGeometrySyncEnabled(false);
        m_geometrySyncKey.clear();

        if (size() != QSizeF(40, 40)) {
            if (!hasErrors() || size().isEmpty())
                ControlPropertyManager::setSize(this, QSizeF(40, 40), ControlPropertyManager::NoOption);
        }

        if (hasErrors())
            m_renderInfo.image = PaintUtils::renderErrorControlImage(size(), id(), devicePixelRatio());

        if (m_renderInfo.image.isNull())
            m_renderInfo.image = PaintUtils::renderNonGuiControlImage(ToolUtils::toolIconPath(m_dir), size(), devicePixelRatio());
    }

    setPixmap(QPixmap::fromImage(m_renderInfo.image));

    emit renderInfoChanged(info.codeChanged);
}

QList<Control*> Control::siblings() const
{
    QList<Control*> siblings;
    if (type() == Form::Type) {
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

QVariant Control::itemChange(int change, const QVariant& value)
{    
    if (change == ItemPositionChange && beingDragged()) {
        const QPointF& snapPos = DesignerScene::snapPosition(value.toPointF());
        const QPointF& snapMargin = value.toPointF() - snapPos;
        m_snapMargin = QSizeF(snapMargin.x(), snapMargin.y());
        if (type() != Form::Type) {
            ControlPropertyManager::Options options = ControlPropertyManager::SaveChanges
                    | ControlPropertyManager::CompressedCall
                    | ControlPropertyManager::DontApplyDesigner;
            if (gui()) {
                setGeometrySyncEnabled(false);
                m_geometrySyncKey = HashFactory::generate();
                options |= ControlPropertyManager::UpdateRenderer;
            }
            ControlPropertyManager::setPos(this, snapPos, options, m_geometrySyncKey);
        }
        return snapPos;
    } else if (change == ItemSizeChange && beingResized()) {
        const QSizeF snapSize = DesignerScene::snapSize(pos(), value.toSizeF() + m_snapMargin);
        m_snapMargin = QSizeF(0, 0);
        if (gui()) {
            setGeometrySyncEnabled(false);
            m_geometrySyncKey = HashFactory::generate();
            ControlPropertyManager::setSize(this, snapSize, ControlPropertyManager::SaveChanges
                                            | ControlPropertyManager::UpdateRenderer
                                            | ControlPropertyManager::CompressedCall
                                            | ControlPropertyManager::DontApplyDesigner, m_geometrySyncKey);
        }
        return snapSize;
    }

    return DesignerItem::itemChange(change, value);
}

bool Control::geometrySyncEnabled() const
{
    return m_geometrySyncEnabled;
}

void Control::setGeometrySyncEnabled(bool geometrySyncEnabled)
{
    m_geometrySyncEnabled = geometrySyncEnabled;
}

void Control::paintContent(QPainter* painter)
{
    if (m_pixmap.isNull())
        return;
    QRectF r(paintRect().topLeft(), QSizeF(m_pixmap.size()) / m_pixmap.devicePixelRatioF());
    painter->drawPixmap(r, m_pixmap, m_pixmap.rect());
}

void Control::paintHighlight(QPainter* painter)
{
    painter->setCompositionMode(QPainter::CompositionMode_SourceAtop);
    painter->fillRect(rect(), QColor(0, 0, 0, 20));
    painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
}

void Control::paintOutline(QPainter* painter)
{
    if (DesignerScene::outlineMode() == DesignerScene::NoOutline)
        return;
    if (DesignerScene::outlineMode() == DesignerScene::ClippingDashLine)
        return DesignerScene::drawDashRect(painter, DesignerScene::outerRect(rect()));
    if (DesignerScene::outlineMode() == DesignerScene::BoundingDashLine)
        return DesignerScene::drawDashRect(painter, DesignerScene::outerRect(paintRect()));

    painter->setBrush(Qt::NoBrush);
    painter->setPen(DesignerScene::pen(Qt::darkGray));

    if (DesignerScene::outlineMode() == DesignerScene::ClippingSolidLine)
        return painter->drawRect(DesignerScene::outerRect(rect()));
    if (DesignerScene::outlineMode() == DesignerScene::BoundingSolidLine)
        return painter->drawRect(DesignerScene::outerRect(paintRect()));
}

void Control::paintHoverOutline(QPainter* painter)
{
    painter->setBrush(Qt::NoBrush);
    painter->setPen(DesignerScene::pen());
    painter->drawRect(DesignerScene::outerRect(rect()));
}

void Control::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget*)
{
    if (beingResized())
        painter->setClipRect(rect());
    paintContent(painter);
    painter->setClipping(false);

    paintOutline(painter);

    if (beingHighlighted())
        paintHighlight(painter);
    if (DesignerScene::showMouseoverOutline() && option->state & QStyle::State_MouseOver)
        paintHoverOutline(painter);
}

void Control::syncGeometry()
{
    if (!gui())
        return;

    if (beingDragged())
        return;

    if (beingResized())
        return;

    if (!geometrySyncEnabled())
        return;

    const QRectF& geometry = UtilityFunctions::getGeometryFromProperties(m_renderInfo.properties);
    if (geometry.isValid()) {
        if (type() != Form::Type)
            ControlPropertyManager::setPos(this, geometry.topLeft(), ControlPropertyManager::NoOption);
        ControlPropertyManager::setSize(this, geometry.size(), ControlPropertyManager::NoOption);
    }

    setGeometrySyncEnabled(false);
}

QRectF Control::paintRect() const
{
    if (m_renderInfo.surroundingRect.isNull())
        return rect();
    return m_renderInfo.surroundingRect;
}

Control* Control::parentControl() const
{
    return static_cast<Control*>(parentItem());
}