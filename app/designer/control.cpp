#include <form.h>
#include <designerscene.h>
#include <utilityfunctions.h>
#include <controlpropertymanager.h>
#include <hashfactory.h>
#include <paintutils.h>
#include <toolutils.h>
#include <designersettings.h>
#include <scenesettings.h>
#include <generalsettings.h>
#include <interfacesettings.h>

#include <QPainter>
#include <QStyleOption>

QList<Control*> Control::s_controls;

Control::Control(Control* parent) : DesignerItem(parent)
  , m_anchors(new Anchors(this))
  , m_snapMargin(QSizeF(0, 0))
  , m_geometrySyncEnabled(false)
  , m_updateAnchorsScheduled(false)
{
    s_controls.append(this);
    m_renderInfo.gui = false;
    m_renderInfo.popup = false;
    m_renderInfo.window = false;
    m_renderInfo.visible = true;
    m_renderInfo.overlayPopup = false;

    setResizable(true);
    setAcceptHoverEvents(true);
    setCursor(Qt::OpenHandCursor);
    setMousePressCursorShape(Qt::ClosedHandCursor);

    setFlag(ItemIsMovable);
    setFlag(ItemIsSelectable);
    setFlag(ItemSendsGeometryChanges);
}

Control::~Control()
{
    s_controls.removeOne(this);
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

bool Control::overlayPopup() const
{
    return m_renderInfo.overlayPopup;
}

bool Control::hasWindowAncestor() const
{
    Control* parent = parentControl();
    while (parent) {
        if (parent->window())
            return true;
        parent = parent->parentControl();
    }
    return false;
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
    if (m_dir != dir) {
        m_dir = dir;
        updateIcon();
    }
}

QIcon Control::icon() const
{
    return m_icon;
}

void Control::updateIcon()
{
    m_icon = QIcon(ToolUtils::toolIconPath(m_dir));
}

QPixmap Control::pixmap() const
{
    return m_pixmap;
}

void Control::setPixmap(const QPixmap& pixmap)
{
    if (m_pixmap.cacheKey() != pixmap.cacheKey()) {
        m_pixmap = pixmap;
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

    enum { NONGUI_ICON_SIZE = 32 };
    const SceneSettings* settings = DesignerSettings::sceneSettings();
    const QMarginsF previousMargins = m_renderInfo.margins;
    const bool previouslyOverlayPopup = overlayPopup();

    m_renderInfo = info;

    const QVariant& scale = itemProperty("scale");
    const QVariant& rotation = itemProperty("rotation");
    const QVariant& transformOrigin = itemProperty("transformOrigin");

    // FIXME: Fix this whenever we are able to manage raising or lowering
    // Controls based on their indexes,
    // FIXME: Fix scene() usage and test
    setFlag(ItemStacksBehindParent, itemProperty("z").toDouble() < 0
            && !(parentControl() && (scene()->isLayerItem(parentControl()) ||
                                     parentControl()->window() ||
                                     parentControl()->popup())));

    updateAnchors();
    setResizable(gui() && (!rotation.isValid() || rotation.toReal() == 0));
    setZValue(itemProperty("z").toDouble());
    setFlag(ItemClipsChildrenToShape, !settings->showClippedControls && itemProperty("clip").toBool());
    setOpacity(itemProperty("opacity").isValid() ? itemProperty("opacity").toDouble() : 1);

    if (info.codeChanged) {
        updateIcon();
        for (Control* childControl : childControls(false))
            childControl->setTransform(QTransform::fromTranslate(margins().left(), margins().top()));
    } else {
        m_renderInfo.margins = previousMargins;
    }

    if (overlayPopup() != previouslyOverlayPopup) {
        if (overlayPopup())
            resetTransform();
        else if (parentControl())
            setTransform(QTransform::fromTranslate(parentControl()->margins().left(), parentControl()->margins().top()));
    }

    if (gui()) {
        if (m_geometrySyncKey.isEmpty() || m_geometrySyncKey == info.geometrySyncKey) {
            m_geometrySyncKey.clear();
            setGeometrySyncEnabled(true);
            syncGeometry();
        }

        if (visible() && rect().isValid() && PaintUtils::isBlankImage(m_renderInfo.image)) {
            // TODO: Can't get here after changing SceneSettings, so highlightColor
            // toBlankControlDecorationBrush() settings are not applied even if
            // they change. User just need to refresh the control or move it a bit.
            if (childControls(false).isEmpty()) {
                m_renderInfo.image = PaintUtils::renderBlankControlImage(
                            rect(), id(), devicePixelRatio(),
                            settings->toBlankControlDecorationBrush(Qt::darkGray),
                            GeneralSettings::interfaceSettings()->highlightColor);
            } else {
                m_renderInfo.image = QImage();
            }
        }
    } else {
        setGeometrySyncEnabled(false);
        m_geometrySyncKey.clear();

        if (size() != QSizeF(NONGUI_ICON_SIZE, NONGUI_ICON_SIZE)) {
            if (!hasErrors() || size().isEmpty()) {
                ControlPropertyManager::setSize(this, QSizeF(NONGUI_ICON_SIZE, NONGUI_ICON_SIZE),
                                                ControlPropertyManager::NoOption);
            }
        }

        if (hasErrors() && size().isValid()) {
            m_renderInfo.image = PaintUtils::renderErrorControlImage(
                        size(), id(),
                        settings->toBlankControlDecorationBrush(QColor(203, 54, 59)),
                        QColor(203, 54, 59), scene() ? (QWidget*) scene()->view() : 0);
        }

        if (m_renderInfo.image.isNull() && size().isValid()) {
            m_renderInfo.image = PaintUtils::renderNonGuiControlImage(
                        ToolUtils::toolIconPath(dir()), size(), scene() ? (QWidget*) scene()->view() : 0);
        }
    }


    m_renderInfo.image.setDevicePixelRatio(devicePixelRatio()); // QDataStream cannot write dpr
    setPixmap(UtilityFunctions::imageToPixmap(m_renderInfo.image));

    // Make sure this is called after setting size of the control
    if (transformOrigin.isValid()) {
        const Enum& e = transformOrigin.value<Enum>();
        if (e.value == QStringLiteral("TopLeft"))
            setTransformOriginPoint(rect().topLeft());
        else if (e.value == QStringLiteral("Top"))
            setTransformOriginPoint(UtilityFunctions::topCenter(rect()));
        else if (e.value == QStringLiteral("TopRight"))
            setTransformOriginPoint(rect().topRight());
        else if (e.value == QStringLiteral("Left"))
            setTransformOriginPoint(UtilityFunctions::leftCenter(rect()));
        else if (e.value == QStringLiteral("Center"))
            setTransformOriginPoint(rect().center());
        else if (e.value == QStringLiteral("Right"))
            setTransformOriginPoint(UtilityFunctions::rightCenter(rect()));
        else if (e.value == QStringLiteral("BottomLeft"))
            setTransformOriginPoint(rect().bottomLeft());
        else if (e.value == QStringLiteral("Botttom"))
            setTransformOriginPoint(UtilityFunctions::bottomCenter(rect()));
        else if (e.value == QStringLiteral("BottomRight"))
            setTransformOriginPoint(rect().bottomRight());
    } else {
        setTransformOriginPoint(QPointF(0, 0));
    }
    if (scale.isValid())
        setScale(scale.toReal());
    else
        setScale(1);
    if (rotation.isValid())
        setRotation(rotation.toReal());
    else
        setRotation(0);

    emit renderInfoChanged(info.codeChanged);
}

static AnchorLine::Type anchorType(const QString& anchorName)
{
    if (anchorName == "top")
        return AnchorLine::Top;
    if (anchorName == "bottom")
        return AnchorLine::Bottom;
    if (anchorName == "left")
        return AnchorLine::Left;
    if (anchorName == "right")
        return AnchorLine::Right;
    if (anchorName == "horizontalCenter")
        return AnchorLine::HorizontalCenter;
    if (anchorName == "verticalCenter")
        return AnchorLine::VerticalCenter;
    if (anchorName == "baseline")
        return AnchorLine::Baseline;
    return AnchorLine::Invalid;
}

Anchors* Control::anchors() const
{
    return m_anchors;
}

void Control::updateAnchors()
{
    if (m_updateAnchorsScheduled) {
        if (beingDragged()) {
            return;
        } else {
            m_updateAnchorsScheduled = false;
            disconnect(this, &Control::beingDraggedChanged, this, &Control::updateAnchors);
        }
    } else {
        if (beingDragged()) {
            m_updateAnchorsScheduled = true;
            connect(this, &Control::beingDraggedChanged, this, &Control::updateAnchors);
            return;
        }
    }

    for (const QString& name : UtilityFunctions::anchorPropertyNames()) {
        const qreal value = m_renderInfo.anchors.value(name).toReal();
        if (name == "anchors.margins")
            m_anchors->setMargins(value);
        else if (name == "anchors.topMargin")
            m_anchors->setTopMargin(value);
        else if (name == "anchors.bottomMargin")
            m_anchors->setBottomMargin(value);
        else if (name == "anchors.leftMargin")
            m_anchors->setLeftMargin(value);
        else if (name == "anchors.rightMargin")
            m_anchors->setRightMargin(value);
        else if (name == "anchors.horizontalCenterOffset")
            m_anchors->setHorizontalCenterOffset(value);
        else if (name == "anchors.verticalCenterOffset")
            m_anchors->setVerticalCenterOffset(value);
        else if (name == "anchors.baselineOffset")
            m_anchors->setBaselineOffset(value);
        else if (name == "anchors.alignWhenCentered")
            m_anchors->setAlignWhenCentered(m_renderInfo.anchors.contains(name) ? value : true);
    }

    QList<QPair<QString, QString>> changedAnchors; // anchors.name, uid
    for (const QString& name : UtilityFunctions::anchorLineNames()) {
        const QStringList& pair = m_renderInfo.anchors.value(name).toStringList();
        if (pair.isEmpty()) {
            if (name == "anchors.fill")
                m_anchors->setFill(nullptr);
            else if (name == "anchors.centerIn")
                m_anchors->setCenterIn(nullptr);
            else if (name == "anchors.top")
                m_anchors->setTop(AnchorLine());
            else if (name == "anchors.bottom")
                m_anchors->setBottom(AnchorLine());
            else if (name == "anchors.left")
                m_anchors->setLeft(AnchorLine());
            else if (name == "anchors.right")
                m_anchors->setRight(AnchorLine());
            else if (name == "anchors.horizontalCenter")
                m_anchors->setHorizontalCenter(AnchorLine());
            else if (name == "anchors.verticalCenter")
                m_anchors->setVerticalCenter(AnchorLine());
            else if (name == "anchors.baseline")
                m_anchors->setBaseline(AnchorLine());
            continue;
        }
        const AnchorLine::Type type = anchorType(pair.first());
        if (name == "anchors.fill") {
            if (m_anchors->fill() == 0 || m_anchors->fill()->uid() != pair.last())
                changedAnchors.append(QPair<QString, QString>(name, pair.last()));
        } else if (name == "anchors.centerIn") {
            if (m_anchors->centerIn() == 0 || m_anchors->centerIn()->uid() != pair.last())
                changedAnchors.append(QPair<QString, QString>(name, pair.last()));
        } else if (name == "anchors.top") {
            if (!m_anchors->top().isValid() || m_anchors->top().control()->uid() != pair.last())
                changedAnchors.append(QPair<QString, QString>(name, pair.last()));
            m_anchors->setTop(AnchorLine(type, m_anchors->top().control()));
        } else if (name == "anchors.bottom") {
            if (!m_anchors->bottom().isValid() || m_anchors->bottom().control()->uid() != pair.last())
                changedAnchors.append(QPair<QString, QString>(name, pair.last()));
            m_anchors->setBottom(AnchorLine(type, m_anchors->bottom().control()));
        } else if (name == "anchors.left") {
            if (!m_anchors->left().isValid() || m_anchors->left().control()->uid() != pair.last())
                changedAnchors.append(QPair<QString, QString>(name, pair.last()));
            m_anchors->setLeft(AnchorLine(type, m_anchors->left().control()));
        } else if (name == "anchors.right") {
            if (!m_anchors->right().isValid() || m_anchors->right().control()->uid() != pair.last())
                changedAnchors.append(QPair<QString, QString>(name, pair.last()));
            m_anchors->setRight(AnchorLine(type, m_anchors->right().control()));
        } else if (name == "anchors.horizontalCenter") {
            if (!m_anchors->horizontalCenter().isValid() || m_anchors->horizontalCenter().control()->uid() != pair.last())
                changedAnchors.append(QPair<QString, QString>(name, pair.last()));
            m_anchors->setHorizontalCenter(AnchorLine(type, m_anchors->horizontalCenter().control()));
        } else if (name == "anchors.verticalCenter") {
            if (!m_anchors->verticalCenter().isValid() || m_anchors->verticalCenter().control()->uid() != pair.last())
                changedAnchors.append(QPair<QString, QString>(name, pair.last()));
            m_anchors->setVerticalCenter(AnchorLine(type, m_anchors->verticalCenter().control()));
        } else if (name == "anchors.baseline") {
            if (!m_anchors->baseline().isValid() || m_anchors->baseline().control()->uid() != pair.last())
                changedAnchors.append(QPair<QString, QString>(name, pair.last()));
            m_anchors->setBaseline(AnchorLine(type, m_anchors->baseline().control()));
        }
    }

    if (changedAnchors.isEmpty())
        return;

    QList<QPair<QString, Control*>> changedControls; // anchors.name, Control*
    QList<Control*> allControls(topLevelControl()->childControls());
    allControls.append(topLevelControl());
    for (Control* control : qAsConst(allControls)) {
        for (const QPair<QString, QString>& pair : changedAnchors) {
            if (pair.second == control->uid())
                changedControls.append(QPair<QString, Control*>(pair.first, control));
        }
    }

    for (const QPair<QString, Control*>& pair : changedControls) {
        const QString& name = pair.first;
        Control* control = pair.second;
        if (name == "anchors.fill")
            m_anchors->setFill(control);
        else if (name == "anchors.centerIn")
            m_anchors->setCenterIn(control);
        else if (name == "anchors.top")
            m_anchors->setTop(AnchorLine(m_anchors->top().type(), control));
        else if (name == "anchors.bottom")
            m_anchors->setBottom(AnchorLine(m_anchors->bottom().type(), control));
        else if (name == "anchors.left")
            m_anchors->setLeft(AnchorLine(m_anchors->left().type(), control));
        else if (name == "anchors.right")
            m_anchors->setRight(AnchorLine(m_anchors->right().type(), control));
        else if (name == "anchors.horizontalCenter")
            m_anchors->setHorizontalCenter(AnchorLine(m_anchors->horizontalCenter().type(), control));
        else if (name == "anchors.verticalCenter")
            m_anchors->setVerticalCenter(AnchorLine(m_anchors->verticalCenter().type(), control));
        else if (name == "anchors.baseline")
            m_anchors->setBaseline(AnchorLine(m_anchors->baseline().type(), control));
    }
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

    // FIXME: Fix scene() usage and test
    setFlag(ItemStacksBehindParent, itemProperty("z").toDouble() < 0
            && !(parentControl() && (scene()->isLayerItem(parentControl()) ||
                                     parentControl()->window() ||
                                     parentControl()->popup())));

    const QRectF& geometry = UtilityFunctions::itemGeometry(m_renderInfo.properties);
    if (geometry.isValid()) {
        if (type() != Form::Type)
            ControlPropertyManager::setPos(this, geometry.topLeft(), ControlPropertyManager::NoOption);
        ControlPropertyManager::setSize(this, geometry.size(), ControlPropertyManager::NoOption);
    }

    setGeometrySyncEnabled(false);
}

bool Control::geometrySyncEnabled() const
{
    return m_geometrySyncEnabled;
}

void Control::setGeometrySyncEnabled(bool geometrySyncEnabled)
{
    m_geometrySyncEnabled = geometrySyncEnabled;
}

QVariant Control::itemProperty(const QString& propertyName) const
{
    return UtilityFunctions::itemProperty(propertyName, m_renderInfo.properties);
}

Control* Control::parentControl() const
{
    return static_cast<Control*>(parentItem());
}

Control* Control::topLevelControl() const
{
    QGraphicsItem* topItem = topLevelItem();
    Q_ASSERT(topItem && topItem->type() >= Control::Type);
    return static_cast<Control*>(topItem);
}

QList<Control*> Control::siblings() const
{
    QList<Control*> siblings;
    if (type() == Form::Type) {
        // FIXME: Fix scene() usage and test
        Q_ASSERT(scene());
        if (!scene())
            return siblings;
        if (const DesignerScene* scene = this->scene()) {
            for (Form* form : scene->forms())
                siblings.append(form);
        }
    } else if (parentControl()) {
        siblings = parentControl()->childControls(false);
    }
    Q_ASSERT(siblings.contains(const_cast<Control*>(this)));
    if (!siblings.isEmpty()) {
        siblings.removeOne(const_cast<Control*>(this));
        std::sort(siblings.begin(), siblings.end(), [] (const Control* left, const Control* right) {
            return left->index() < right->index();
        });
    }
    return siblings;
}

QList<Control*> Control::childControls(bool recursive) const
{
    QList<Control*> controls;

    for (QGraphicsItem* item : QGraphicsObject::childItems()) {
        if (item->type() >= Control::Type)
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

QList<Control*> Control::controls()
{
    return s_controls;
}

QRectF Control::surroundingRect() const
{
    if (m_renderInfo.surroundingRect.isNull())
        return rect();
    return m_renderInfo.surroundingRect;
}

void Control::paintContent(QPainter* painter)
{
    if (m_pixmap.isNull())
        return;
    const QSizeF pixmapSize(m_pixmap.size() / m_pixmap.devicePixelRatio());
    QRectF rect(surroundingRect().topLeft(), pixmapSize);
    if (qAbs(surroundingRect().width() - pixmapSize.width()) > 2
            || qAbs(surroundingRect().height() - pixmapSize.height()) > 2) {
        rect.moveTopLeft(QPointF());
    }
    painter->drawPixmap(rect, m_pixmap, m_pixmap.rect());
}

void Control::paintHighlight(QPainter* painter)
{
    painter->setCompositionMode(QPainter::CompositionMode_SourceAtop);
    painter->fillRect(rect(), QColor(0, 0, 0, 20));
    painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
}

void Control::paintOutline(QPainter* painter)
{
    if (!scene())
        return;

    const SceneSettings* settings = DesignerSettings::sceneSettings();

    if (settings->controlOutlineDecoration == 0) // NoOutline
        return;
    if (settings->controlOutlineDecoration == 1) // ClippingDashLine
        return DesignerScene::drawDashRect(painter, scene()->outerRect(rect()));
    if (settings->controlOutlineDecoration == 2) // BoundingDashLine
        return DesignerScene::drawDashRect(painter, scene()->outerRect(surroundingRect()));

    painter->setBrush(Qt::NoBrush);
    painter->setPen(DesignerScene::pen(Qt::darkGray));

    if (settings->controlOutlineDecoration == 3) // ClippingSolidLine
        return painter->drawRect(scene()->outerRect(rect()));
    if (settings->controlOutlineDecoration == 4) // BoundingSolidLine
        return painter->drawRect(scene()->outerRect(surroundingRect()));
}

void Control::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
{
    if (beingResized())
        painter->setClipRect(rect());
    paintContent(painter);
    painter->setClipping(false);
    paintOutline(painter);
    if (beingHighlighted())
        paintHighlight(painter);
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
