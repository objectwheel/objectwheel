#include <form.h>
#include <designerscene.h>
#include <utilityfunctions.h>
#include <controlpropertymanager.h>
#include <hashfactory.h>
#include <paintutils.h>
#include <toolutils.h>

#include <QPainter>
#include <QStyleOption>

Control::Control(Control* parent) : DesignerItem(parent)
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

Anchors* Control::anchors() const
{
    return m_anchors;
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
        Q_ASSERT(devicePixelRatio() == pixmap.devicePixelRatio());
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

    const QMarginsF previousMargins = m_renderInfo.margins;

    m_renderInfo = info;

    updateAnchors();
    setResizable(gui());
    setZValue(property("z").toDouble());
    setFlag(ItemClipsChildrenToShape, !DesignerScene::showClippedControls() && property("clip").toBool());
    setOpacity(property("opacity").isValid() ? property("opacity").toDouble() : 1);

    if (info.codeChanged) {
        for (Control* childControl : childControls(false))
            childControl->setTransform(QTransform::fromTranslate(margins().left(), margins().top()));
    } else {
        m_renderInfo.margins = previousMargins;
    }

    if (gui()) {
        if (m_geometrySyncKey.isEmpty() || m_geometrySyncKey == info.geometrySyncKey) {
            m_geometrySyncKey.clear();
            setGeometrySyncEnabled(true);
            syncGeometry();
        }

        if (visible() && rect().isValid() && PaintUtils::isBlankImage(m_renderInfo.image)) {
            m_renderInfo.image = PaintUtils::renderBlankControlImage(
                        rect(), id(), devicePixelRatio(),
                        DesignerScene::blankControlDecorationBrush(Qt::darkGray),
                        childControls(false).isEmpty() ? DesignerScene::outlineColor() : Qt::transparent);
        }
    } else {
        setGeometrySyncEnabled(false);
        m_geometrySyncKey.clear();

        if (size() != QSizeF(40, 40)) {
            if (!hasErrors() || size().isEmpty())
                ControlPropertyManager::setSize(this, QSizeF(40, 40), ControlPropertyManager::NoOption);
        }

        if (hasErrors() && size().isValid()) {
            m_renderInfo.image = PaintUtils::renderErrorControlImage(
                        size(), id(), devicePixelRatio(),
                        DesignerScene::blankControlDecorationBrush(QColor(203, 54, 59)),
                        QColor(203, 54, 59));
        }

        if (m_renderInfo.image.isNull() && size().isValid()) {
            m_renderInfo.image = PaintUtils::renderNonGuiControlImage(
                        ToolUtils::toolIconPath(dir()), size(), devicePixelRatio());
        }
    }

    m_renderInfo.image.setDevicePixelRatio(devicePixelRatio()); // QDataStream cannot write dpr
    setPixmap(UtilityFunctions::imageToPixmap(m_renderInfo.image));

    emit renderInfoChanged(info.codeChanged);
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

bool Control::geometrySyncEnabled() const
{
    return m_geometrySyncEnabled;
}

void Control::setGeometrySyncEnabled(bool geometrySyncEnabled)
{
    m_geometrySyncEnabled = geometrySyncEnabled;
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

void Control::updateAnchors()
{
    bool targetChanged = false;
    for (const QString& name : m_renderInfo.anchors.keys()) {
        const QStringList& pair = m_renderInfo.anchors.value(name).toStringList();
        if (pair.isEmpty())
            continue;
        if (name == "anchors.fill") {
            if (m_anchors->fill() == 0) {
                targetChanged = true;
                break;
            }
            if (m_anchors->fill()->uid() != pair.last()) {
                targetChanged = true;
                break;
            }
        } else if (name == "anchors.centerIn") {
            if (m_anchors->centerIn() == 0) {
                targetChanged = true;
                break;
            }
            if (m_anchors->centerIn()->uid() != pair.last()) {
                targetChanged = true;
                break;
            }
        } else if (name == "anchors.top") {
            if (!m_anchors->top().isValid()) {
                targetChanged = true;
                break;
            }
            if (m_anchors->top().control()->uid() != pair.last()) {
                targetChanged = true;
                break;
            }
            if (m_anchors->top().type() != anchorType(pair.first()))
                m_anchors->top().setType(anchorType(pair.first()));
        } else if (name == "anchors.bottom") {
            if (!m_anchors->bottom().isValid()) {
                targetChanged = true;
                break;
            }
            if (m_anchors->bottom().control()->uid() != pair.last()) {
                targetChanged = true;
                break;
            }
            if (m_anchors->bottom().type() != anchorType(pair.first()))
                m_anchors->bottom().setType(anchorType(pair.first()));
        } else if (name == "anchors.left") {
            if (!m_anchors->left().isValid()) {
                targetChanged = true;
                break;
            }
            if (m_anchors->left().control()->uid() != pair.last()) {
                targetChanged = true;
                break;
            }
            if (m_anchors->left().type() != anchorType(pair.first()))
                m_anchors->left().setType(anchorType(pair.first()));
        } else if (name == "anchors.right") {
            if (!m_anchors->right().isValid()) {
                targetChanged = true;
                break;
            }
            if (m_anchors->right().control()->uid() != pair.last()) {
                targetChanged = true;
                break;
            }
            if (m_anchors->right().type() != anchorType(pair.first()))
                m_anchors->right().setType(anchorType(pair.first()));
        } else if (name == "anchors.horizontalCenter") {
            if (!m_anchors->horizontalCenter().isValid()) {
                targetChanged = true;
                break;
            }
            if (m_anchors->horizontalCenter().control()->uid() != pair.last()) {
                targetChanged = true;
                break;
            }
            if (m_anchors->horizontalCenter().type() != anchorType(pair.first()))
                m_anchors->horizontalCenter().setType(anchorType(pair.first()));
        } else if (name == "anchors.verticalCenter") {
            if (!m_anchors->verticalCenter().isValid()) {
                targetChanged = true;
                break;
            }
            if (m_anchors->verticalCenter().control()->uid() != pair.last()) {
                targetChanged = true;
                break;
            }
            if (m_anchors->verticalCenter().type() != anchorType(pair.first()))
                m_anchors->verticalCenter().setType(anchorType(pair.first()));
        } else if (name == "anchors.baseline") {
            if (!m_anchors->baseline().isValid()) {
                targetChanged = true;
                break;
            }
            if (m_anchors->baseline().control()->uid() != pair.last()) {
                targetChanged = true;
                break;
            }
            if (m_anchors->baseline().type() != anchorType(pair.first()))
                m_anchors->baseline().setType(anchorType(pair.first()));
        }
    }

    m_anchors->clear();

    QHash<QString, Control*> anchorControls;
    for (Control* control : scene()->items<Control>()) {
        for (const QVariant& anchorValue : m_renderInfo.anchors) {
            const QStringList& pair = anchorValue.toStringList();
            if (!pair.isEmpty() && pair.last() == control->uid())
                anchorControls.insert(control->uid(), control);
        }
    }

    for (const QString& name : m_renderInfo.anchors.keys()) {
        if (name == "anchors.fill") {
            const QStringList& pair = m_renderInfo.anchors.value(name).toStringList();
            Q_ASSERT(pair.size() == 1);
            for (Control* control : scene()->items<Control>()) {
                if (pair.last() == control->uid())
                    m_anchors->setFill(control);
            }
        } else if (name == "anchors.centerIn") {
            const QStringList& pair = m_renderInfo.anchors.value(name).toStringList();
            Q_ASSERT(pair.size() == 1);
            for (Control* control : scene()->items<Control>()) {
                if (pair.last() == control->uid())
                    m_anchors->setCenterIn(control);
            }
        } else if (name == "anchors.top") {
            const QStringList& pair = m_renderInfo.anchors.value(name).toStringList();
            Q_ASSERT(pair.size() == 2);
            for (Control* control : scene()->items<Control>()) {
                if (pair.last() == control->uid())
                    m_anchors->setTop(AnchorLine(anchorType(pair.first()), control));
            }
        } else if (name == "anchors.bottom") {
            const QStringList& pair = m_renderInfo.anchors.value(name).toStringList();
            Q_ASSERT(pair.size() == 2);
            for (Control* control : scene()->items<Control>()) {
                if (pair.last() == control->uid())
                    m_anchors->setBottom(AnchorLine(anchorType(pair.first()), control));
            }
        } else if (name == "anchors.left") {
            const QStringList& pair = m_renderInfo.anchors.value(name).toStringList();
            Q_ASSERT(pair.size() == 2);
            for (Control* control : scene()->items<Control>()) {
                if (pair.last() == control->uid())
                    m_anchors->setLeft(AnchorLine(anchorType(pair.first()), control));
            }
        } else if (name == "anchors.right") {
            const QStringList& pair = m_renderInfo.anchors.value(name).toStringList();
            Q_ASSERT(pair.size() == 2);
            for (Control* control : scene()->items<Control>()) {
                if (pair.last() == control->uid())
                    m_anchors->setRight(AnchorLine(anchorType(pair.first()), control));
            }
        } else if (name == "anchors.horizontalCenter") {
            const QStringList& pair = m_renderInfo.anchors.value(name).toStringList();
            Q_ASSERT(pair.size() == 2);
            for (Control* control : scene()->items<Control>()) {
                if (pair.last() == control->uid())
                    m_anchors->setHorizontalCenter(AnchorLine(anchorType(pair.first()), control));
            }
        } else if (name == "anchors.verticalCenter") {
            const QStringList& pair = m_renderInfo.anchors.value(name).toStringList();
            Q_ASSERT(pair.size() == 2);
            for (Control* control : scene()->items<Control>()) {
                if (pair.last() == control->uid())
                    m_anchors->setVerticalCenter(AnchorLine(anchorType(pair.first()), control));
            }
        } else if (name == "anchors.baseline") {
            const QStringList& pair = m_renderInfo.anchors.value(name).toStringList();
            Q_ASSERT(pair.size() == 2);
            for (Control* control : scene()->items<Control>()) {
                if (pair.last() == control->uid())
                    m_anchors->setBaseline(AnchorLine(anchorType(pair.first()), control));
            }
        } else if (name == "anchors.margins") {
            m_anchors->setMargins(m_renderInfo.anchors.value(name).toReal());
        } else if (name == "anchors.topMargin") {
            m_anchors->setTopMargin(m_renderInfo.anchors.value(name).toReal());
        } else if (name == "anchors.bottomMargin") {
            m_anchors->setBottomMargin(m_renderInfo.anchors.value(name).toReal());
        } else if (name == "anchors.leftMargin") {
            m_anchors->setLeftMargin(m_renderInfo.anchors.value(name).toReal());
        } else if (name == "anchors.rightMargin") {
            m_anchors->setRightMargin(m_renderInfo.anchors.value(name).toReal());
        } else if (name == "anchors.horizontalCenterOffset") {
            m_anchors->setHorizontalCenterOffset(m_renderInfo.anchors.value(name).toReal());
        } else if (name == "anchors.verticalCenterOffset") {
            m_anchors->setVerticalCenterOffset(m_renderInfo.anchors.value(name).toReal());
        } else if (name == "anchors.baselineOffset") {
            m_anchors->setBaselineOffset(m_renderInfo.anchors.value(name).toReal());
        } else if (name == "anchors.alignWhenCentered") {
            m_anchors->setAlignWhenCentered(m_renderInfo.anchors.value(name).toReal());
        }
    }
}

QVariant Control::property(const QString& propertyName) const
{
    return UtilityFunctions::getProperty(propertyName, m_renderInfo.properties);
}

Control* Control::parentControl() const
{
    return static_cast<Control*>(parentItem());
}

QList<Control*> Control::siblings() const
{
    QList<Control*> siblings;
    if (type() == Form::Type) {
        if (const DesignerScene* scene = this->scene()) {
            for (Form* form : scene->forms())
                siblings.append(form);
        }
    } else if (parentControl()) {
        siblings = parentControl()->childControls(false);
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

QRectF Control::contentRect() const
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
    QRectF rect(contentRect().topLeft(), pixmapSize);
    if (qAbs(contentRect().width() - pixmapSize.width()) > 2
            || qAbs(contentRect().height() - pixmapSize.height()) > 2) {
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
    if (DesignerScene::outlineMode() == DesignerScene::NoOutline)
        return;
    if (DesignerScene::outlineMode() == DesignerScene::ClippingDashLine)
        return DesignerScene::drawDashRect(painter, DesignerScene::outerRect(rect()));
    if (DesignerScene::outlineMode() == DesignerScene::BoundingDashLine)
        return DesignerScene::drawDashRect(painter, DesignerScene::outerRect(contentRect()));

    painter->setBrush(Qt::NoBrush);
    painter->setPen(DesignerScene::pen(Qt::darkGray));

    if (DesignerScene::outlineMode() == DesignerScene::ClippingSolidLine)
        return painter->drawRect(DesignerScene::outerRect(rect()));
    if (DesignerScene::outlineMode() == DesignerScene::BoundingSolidLine)
        return painter->drawRect(DesignerScene::outerRect(contentRect()));
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
