#include <paintlayer.h>
#include <designerscene.h>
#include <resizeritem.h>
#include <gadgetlayer.h>
#include <anchorlayer.h>
#include <paintutils.h>
#include <utilityfunctions.h>
#include <control.h>
#include <designersettings.h>
#include <scenesettings.h>

#include <QPainter>
#include <QtMath>

// TODO: Do not treat "baseline" anchors as "top"

enum { AngleDegree = 16 };

static int sourceAngle(const PaintLayer::AnchorData& data)
{
    const QPointF& offset = data.endPoint - data.startPoint;
    switch (data.sourceAnchorLineType) {
    case AnchorLine::Baseline:
    case AnchorLine::Top:
        return 0;
    case AnchorLine::Bottom:
        return 180 * AngleDegree;
    case AnchorLine::Left:
        return 90 * AngleDegree;
    case AnchorLine::Right:
        return 270 * AngleDegree;
    case AnchorLine::VerticalCenter:
        if (offset.y() < 0)
            return 180 * AngleDegree;
        return 0;
    case AnchorLine::HorizontalCenter:
        if (offset.x() < 0)
            return 270 * AngleDegree;
        return 90 * AngleDegree;
    default:
        return 0;
    }
}

static int targetAngle(const PaintLayer::AnchorData& data)
{
    if (data.targetAnchorLineType == AnchorLine::Invalid)
        return 0;

    const QPointF& offset = data.endPoint - data.startPoint;
    if (AnchorLine::isVertical(data.targetAnchorLineType)) {
        if (offset.x() < 0)
            return 90;
        return -90;
    } else {
        if (offset.y() < 0)
            return 180;
        return 0;
    }
}

static qreal marginOffset(const PaintLayer::AnchorData& data)
{
    switch (data.sourceAnchorLineType) {
    case AnchorLine::Top:
        if (data.anchors->topMargin())
            return data.anchors->topMargin();
        return data.anchors->margins();
    case AnchorLine::Bottom:
        if (data.anchors->bottomMargin())
            return data.anchors->bottomMargin();
        return data.anchors->margins();
    case AnchorLine::Left:
        if (data.anchors->leftMargin())
            return data.anchors->leftMargin();
        return data.anchors->margins();
    case AnchorLine::Right:
        if (data.anchors->rightMargin())
            return data.anchors->rightMargin();
        return data.anchors->margins();
    case AnchorLine::VerticalCenter:
        return data.anchors->verticalCenterOffset();
    case AnchorLine::HorizontalCenter:
        return data.anchors->horizontalCenterOffset();
    case AnchorLine::Baseline:
        return data.anchors->baselineOffset();
    default:
        return 0;
    }
}

static QFont marginOffsetFont()
{
    QFont font; // App default font
    font.setPixelSize(font.pixelSize() - 4);
    return font;
}

static QPointF createParentAnchorPoint(const Control* childControl, const Control* parentControl,
                                       AnchorLine::Type anchorLineType)
{
    const QRectF& parentBoundingRect = DesignerScene::contentRect(parentControl);
    const QRectF& childBoundingRect = DesignerScene::rect(childControl);
    switch (anchorLineType) {
    case AnchorLine::Baseline:
    case AnchorLine::Top:
        return QPointF(childBoundingRect.center().x(), parentBoundingRect.top());
    case AnchorLine::Bottom:
        return QPointF(childBoundingRect.center().x(), parentBoundingRect.bottom());
    case AnchorLine::Left:
        return QPointF(parentBoundingRect.left(), childBoundingRect.center().y());
    case AnchorLine::Right:
        return QPointF(parentBoundingRect.right(), childBoundingRect.center().y());
    case AnchorLine::VerticalCenter:
        return QPointF(childBoundingRect.center().x(), parentBoundingRect.center().y());
    case AnchorLine::HorizontalCenter:
        return QPointF(parentBoundingRect.center().x(), childBoundingRect.center().y());
    default:
        return QPointF();
    }
}

static QPointF createAnchorPoint(const Control* control, AnchorLine::Type anchorLineType, bool source)
{
    const QRectF& boundingRect = source ? DesignerScene::rect(control)
                                        : DesignerScene::contentRect(control);
    switch (anchorLineType) {
    case AnchorLine::Baseline:
    case AnchorLine::Top:
        return UtilityFunctions::topCenter(boundingRect);
    case AnchorLine::Bottom:
        return UtilityFunctions::bottomCenter(boundingRect);
    case AnchorLine::Left:
        return UtilityFunctions::leftCenter(boundingRect);
    case AnchorLine::Right:
        return UtilityFunctions::rightCenter(boundingRect);
    case AnchorLine::VerticalCenter:
    case AnchorLine::HorizontalCenter:
        return boundingRect.center();
    default:
        return QPointF();
    }
}

static QPointF createControlPoint(const QPointF& firstEditPoint, AnchorLine::Type anchorLineType,
                                  const QPointF& secondEditPoint)
{
    QPointF controlPoint = firstEditPoint;
    if (AnchorLine::isVertical(anchorLineType))
        controlPoint.rx() += (secondEditPoint.x() - firstEditPoint.x()) / 2.0;
    else
        controlPoint.ry() += (secondEditPoint.y() - firstEditPoint.y()) / 2.0;
    return controlPoint;
}

static void updateAnchorLinePoints(QPointF& firstPoint, QPointF& secondPoint,
                                   const AnchorLine& anchorLine, bool source)
{
    QRectF boundingRectangle = source ? DesignerScene::rect(anchorLine.control())
                                      : DesignerScene::contentRect(anchorLine.control());
    switch (anchorLine.type()) {
    case AnchorLine::VerticalCenter:
        firstPoint = UtilityFunctions::leftCenter(boundingRectangle);
        secondPoint = UtilityFunctions::rightCenter(boundingRectangle);
        break;
    case AnchorLine::HorizontalCenter:
        firstPoint = UtilityFunctions::topCenter(boundingRectangle);
        secondPoint = UtilityFunctions::bottomCenter(boundingRectangle);
        break;
    case AnchorLine::Top:
    case AnchorLine::Baseline:
        firstPoint = boundingRectangle.topLeft();
        secondPoint = boundingRectangle.topRight();
        break;
    case AnchorLine::Bottom:
        firstPoint = boundingRectangle.bottomLeft();
        secondPoint = boundingRectangle.bottomRight();
        break;
    case AnchorLine::Left:
        firstPoint = boundingRectangle.topLeft();
        secondPoint = boundingRectangle.bottomLeft();
        break;
    case AnchorLine::Right:
        firstPoint = boundingRectangle.topRight();
        secondPoint = boundingRectangle.bottomRight();
        break;
    default:
        break;
    }
}

static void updateAnchorData(PaintLayer::AnchorData& data, const AnchorLine& sourceAnchorLine,
                             const AnchorLine& targetAnchorLine, DesignerScene* scene)
{
    if (sourceAnchorLine.isValid() && targetAnchorLine.isValid()) {
        data.sourceAnchorLineType = sourceAnchorLine.type();
        data.targetAnchorLineType = targetAnchorLine.type();
        data.startPoint = createAnchorPoint(sourceAnchorLine.control(), sourceAnchorLine.type(), true);

        if (targetAnchorLine.control() == sourceAnchorLine.control()->parentControl() ||
                (scene && sourceAnchorLine.control()->beingDragged() && targetAnchorLine.control() == scene->parentBeforeDrag())) {
            data.endPoint = createParentAnchorPoint(sourceAnchorLine.control(), targetAnchorLine.control(), targetAnchorLine.type());
        } else {
            data.endPoint = createAnchorPoint(targetAnchorLine.control(), targetAnchorLine.type(), false);
        }

        data.firstControlPoint = createControlPoint(data.startPoint, sourceAnchorLine.type(), data.endPoint);
        data.secondControlPoint = createControlPoint(data.endPoint, targetAnchorLine.type(), data.startPoint);

        updateAnchorLinePoints(data.sourceAnchorLineFirstPoint, data.sourceAnchorLineSecondPoint, sourceAnchorLine, true);
        updateAnchorLinePoints(data.targetAnchorLineFirstPoint, data.targetAnchorLineSecondPoint, targetAnchorLine, false);
    }
}

static QPainterPath highlightPath(const Control* control, bool source)
{
    QPainterPath parentPath;
    if (!control->scene())
        return parentPath;
    parentPath.addRect(control->scene()->outerRect(source ? DesignerScene::rect(control)
                                                          : DesignerScene::contentRect(control)));
    QPainterPath chilrenPath;
    chilrenPath.setFillRule(Qt::WindingFill);
    for (Control* childControl : control->childControls(false))
        chilrenPath.addRect(control->scene()->outerRect(DesignerScene::rect(childControl)));
    return parentPath - chilrenPath;
}

PaintLayer::PaintLayer(DesignerItem* parent) : DesignerItem(parent)
  , m_geometryUpdateScheduled(false)
{
}

void PaintLayer::updateGeometry()
{
    if (m_geometryUpdateScheduled)
        return;
    m_geometryUpdateScheduled = true;
    QMetaObject::invokeMethod(this, [=] {
        Q_ASSERT(scene());
        setGeometry(QPointF(), scene()->sceneRect().size());
        m_geometryUpdateScheduled = false;
    }, Qt::QueuedConnection);
}

void PaintLayer::paintMarginOffset(QPainter* painter, const AnchorData& data)
{
    Q_ASSERT(scene());

    painter->save();

    const QFontMetrics fm(marginOffsetFont());
    const QString& label = QString::number(marginOffset(data));
    const QLineF line(data.firstControlPoint, data.secondControlPoint);
    const qreal z = scene()->zoomLevel();
    const qreal w = fm.horizontalAdvance(label) + fm.height() / 2.0;
    const qreal h = fm.height();

    if (label == "0") {
        painter->restore();
        return;
    }

    qreal angle = 180 - line.angle();
    if (qAbs(angle) == 180 || qAbs(angle) == 360)
        angle = 0;
    if (qAbs(angle) == 90 || qAbs(angle) == 270)
        angle = 90;

    painter->setFont(marginOffsetFont());
    painter->setPen(Qt::NoPen);
    painter->setBrush(DesignerSettings::sceneSettings()->anchorColor);
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->translate(line.center());
    painter->rotate(angle);
    painter->drawRoundedRect(QRectF(-w/2/z, -h/2/z, w/z, h/z), h/2/z, h/2/z);
    painter->setPen(Qt::white);
    painter->scale(1/z, 1/z);
    painter->drawText(QRectF(-w/2, -h/2, w, h), label, QTextOption(Qt::AlignCenter));
    painter->restore();
}

void PaintLayer::paintAnchor(QPainter* painter, const PaintLayer::AnchorData& data)
{
    Q_ASSERT(scene());
    SceneSettings* settings = DesignerSettings::sceneSettings();
    painter->save();
    painter->setPen(DesignerScene::pen(settings->anchorColor, 2));
    painter->drawLine(data.sourceAnchorLineFirstPoint, data.sourceAnchorLineSecondPoint);
    painter->drawLine(data.targetAnchorLineFirstPoint, data.targetAnchorLineSecondPoint);

    DesignerScene::drawDashLine(painter, {data.startPoint, data.firstControlPoint});
    DesignerScene::drawDashLine(painter, {data.firstControlPoint, data.secondControlPoint});
    DesignerScene::drawDashLine(painter, {data.secondControlPoint, data.endPoint});

    static const QPixmap& anchorPixmap = PaintUtils::renderOverlaidPixmap(":/images/anchor.svg", Qt::white, devicePixelRatio());
    const qreal z = scene()->zoomLevel();
    const qreal m = 10;
    QRectF bumpRectangle(0, 0, m/z, m/z);

    painter->setPen(Qt::NoPen);
    painter->setBrush(settings->anchorColor);
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setRenderHint(QPainter::SmoothPixmapTransform);
    bumpRectangle.moveTo(data.startPoint.x() - m/2/z, data.startPoint.y() - m/2/z);
    painter->drawChord(bumpRectangle, sourceAngle(data), 180 * AngleDegree);

    bumpRectangle.moveTo(data.endPoint.x() - m/2/z, data.endPoint.y() - m/2/z);
    painter->setPen(DesignerScene::pen(settings->anchorColor, 2));
    painter->drawRoundedRect(bumpRectangle, m/2/z, m/2/z);
    painter->translate(bumpRectangle.center());
    painter->rotate(targetAngle(data));
    painter->scale(1/z, 1/z);
    painter->drawPixmap(QRect(-m/2, -m/2, m, m), anchorPixmap, anchorPixmap.rect());
    painter->restore();
}

void PaintLayer::paintFillAnchor(QPainter* painter, Control* control)
{
    auto fill = new Anchors;
    fill->setTop(AnchorLine(AnchorLine::Top, control->anchors()->fill()));
    fill->setBottom(AnchorLine(AnchorLine::Bottom, control->anchors()->fill()));
    fill->setLeft(AnchorLine(AnchorLine::Left, control->anchors()->fill()));
    fill->setRight(AnchorLine(AnchorLine::Right, control->anchors()->fill()));
    fill->setMargins(control->anchors()->margins());
    fill->setTopMargin(control->anchors()->topMargin());
    fill->setBottomMargin(control->anchors()->bottomMargin());
    fill->setLeftMargin(control->anchors()->leftMargin());
    fill->setRightMargin(control->anchors()->rightMargin());
    AnchorData data;
    data.anchors = fill;
    updateAnchorData(data, {AnchorLine::Top, control}, data.anchors->top(), scene());
    paintAnchor(painter, data);
    paintMarginOffset(painter, data);
    updateAnchorData(data, {AnchorLine::Bottom, control}, data.anchors->bottom(), scene());
    paintAnchor(painter, data);
    paintMarginOffset(painter, data);
    updateAnchorData(data, {AnchorLine::Right, control}, data.anchors->right(), scene());
    paintAnchor(painter, data);
    paintMarginOffset(painter, data);
    updateAnchorData(data, {AnchorLine::Left, control}, data.anchors->left(), scene());
    paintAnchor(painter, data);
    paintMarginOffset(painter, data);
    delete fill;
}

void PaintLayer::paintCenterAnchor(QPainter* painter, Control* control)
{
    Q_ASSERT(scene());

    painter->save();
    SceneSettings* settings = DesignerSettings::sceneSettings();
    AnchorData data;
    data.anchors = control->anchors();
    data.startPoint = createAnchorPoint(control, AnchorLine::VerticalCenter, true);
    if (control->overlayPopup())
        data.endPoint = data.startPoint;
    else
        data.endPoint = createAnchorPoint(control->anchors()->centerIn(), AnchorLine::VerticalCenter, false);
    data.firstControlPoint = createControlPoint(data.startPoint, AnchorLine::VerticalCenter, data.endPoint);
    data.secondControlPoint = createControlPoint(data.endPoint, AnchorLine::VerticalCenter, data.startPoint);
    data.targetAnchorLineType = AnchorLine::VerticalCenter;

    DesignerScene::drawDashLine(painter, {data.startPoint, data.firstControlPoint});
    DesignerScene::drawDashLine(painter, {data.firstControlPoint, data.secondControlPoint});
    DesignerScene::drawDashLine(painter, {data.secondControlPoint, data.endPoint});

    static const QPixmap& anchorPixmap = PaintUtils::renderOverlaidPixmap(":/images/anchor.svg", Qt::white, devicePixelRatio());
    const qreal z = scene()->zoomLevel();
    const qreal m = 10;
    QRectF bumpRectangle(0, 0, m/z, m/z);

    painter->setPen(Qt::NoPen);
    painter->setBrush(settings->anchorColor);
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setRenderHint(QPainter::SmoothPixmapTransform);
    bumpRectangle.moveTo(data.startPoint.x() - m/2/z, data.startPoint.y() - m/2/z);
    painter->drawRoundedRect(bumpRectangle, bumpRectangle.width() / 2, bumpRectangle.height() / 2);
    bumpRectangle.adjust(2/z, 2/z, -2/z, -2/z);
    painter->setPen(DesignerScene::pen(Qt::white, 2));
    painter->setBrush(Qt::NoBrush);
    painter->drawRoundedRect(bumpRectangle, bumpRectangle.width() / 2, bumpRectangle.height() / 2);

    painter->setPen(Qt::NoPen);
    painter->setBrush(settings->anchorColor);
    bumpRectangle.adjust(-4/z, -4/z, 4/z, 4/z);
    bumpRectangle.moveTo(data.endPoint.x() - bumpRectangle.width() / 2, data.endPoint.y() - bumpRectangle.height() / 2);
    painter->drawRoundedRect(bumpRectangle, bumpRectangle.width() / 2, bumpRectangle.height() / 2);
    bumpRectangle.adjust(1/z, 1/z, -1/z, -1/z);
    painter->setPen(DesignerScene::pen(Qt::white, 1.5));
    painter->setBrush(Qt::NoBrush);
    painter->drawRoundedRect(bumpRectangle, bumpRectangle.width() / 2, bumpRectangle.height() / 2);
    painter->translate(bumpRectangle.center());
    painter->rotate(targetAngle(data));
    painter->scale(1/z, 1/z);
    painter->drawPixmap(QRect(-m/2, -m/2, m, m), anchorPixmap, anchorPixmap.rect());
    painter->restore();

    painter->save();
    const QFontMetrics fm(marginOffsetFont());
    painter->translate(0, -fm.height()/2/z);
    data.sourceAnchorLineType = AnchorLine::VerticalCenter;
    paintMarginOffset(painter, data);
    painter->translate(0, fm.height()/z);
    data.sourceAnchorLineType = AnchorLine::HorizontalCenter;
    paintMarginOffset(painter, data);
    painter->restore();
}

void PaintLayer::paintAnchors(QPainter* painter)
{
    Q_ASSERT(scene());
    const QList<Control*>& anchorControls = scene()->showAllAnchors()
            ? scene()->items<Control>()
            : scene()->selectedControls();
    for (Control* selectedControl : anchorControls) {
        if (selectedControl->anchors()->fill()) {
            paintFillAnchor(painter, selectedControl);
        } else if (selectedControl->anchors()->centerIn()) {
            paintCenterAnchor(painter, selectedControl);
        } else {
            AnchorData data;
            data.anchors = selectedControl->anchors();
            if (data.anchors->baseline().isValid()) {
                updateAnchorData(data, {AnchorLine::Baseline, selectedControl}, data.anchors->baseline(), scene());
                paintAnchor(painter, data);
                paintMarginOffset(painter, data);
            }
            if (data.anchors->top().isValid()) {
                updateAnchorData(data, {AnchorLine::Top, selectedControl}, data.anchors->top(), scene());
                paintAnchor(painter, data);
                paintMarginOffset(painter, data);
            }
            if (data.anchors->bottom().isValid()) {
                updateAnchorData(data, {AnchorLine::Bottom, selectedControl}, data.anchors->bottom(), scene());
                paintAnchor(painter, data);
                paintMarginOffset(painter, data);
            }
            if (data.anchors->right().isValid()) {
                updateAnchorData(data, {AnchorLine::Right, selectedControl}, data.anchors->right(), scene());
                paintAnchor(painter, data);
                paintMarginOffset(painter, data);
            }
            if (data.anchors->left().isValid()) {
                updateAnchorData(data, {AnchorLine::Left, selectedControl}, data.anchors->left(), scene());
                paintAnchor(painter, data);
                paintMarginOffset(painter, data);
            }
            if (data.anchors->horizontalCenter().isValid()) {
                updateAnchorData(data, {AnchorLine::HorizontalCenter, selectedControl}, data.anchors->horizontalCenter(), scene());
                paintAnchor(painter, data);
                paintMarginOffset(painter, data);
            }
            if (data.anchors->verticalCenter().isValid()) {
                updateAnchorData(data, {AnchorLine::VerticalCenter, selectedControl}, data.anchors->verticalCenter(), scene());
                paintAnchor(painter, data);
                paintMarginOffset(painter, data);
            }
        }
    }
}

void PaintLayer::paintAnchorConnection(QPainter* painter)
{
    Q_ASSERT(scene());

    if (!scene()->anchorLayer()->activated() || false) // FIXME: also paint when anchor editor is open
        return;

    painter->save();

    SceneSettings* settings = DesignerSettings::sceneSettings();
    const Control* sourceControl = scene()->anchorLayer()->sourceControl();
    const Control* targetControl = scene()->anchorLayer()->targetControl();

    if (!DesignerScene::isAnchorViable(sourceControl, targetControl))
        targetControl = nullptr;

    QPainterPath scenePath;
    scenePath.addRect(scene()->sceneRect());
    QPainterPath highlightedPath;
    if (sourceControl)
        highlightedPath += highlightPath(sourceControl, true);
    if (targetControl)
        highlightedPath += highlightPath(targetControl, false);
    painter->fillPath(scenePath - highlightedPath, QColor(0, 0, 0, 35));

    painter->setBrush(Qt::NoBrush);
    painter->setPen(DesignerScene::pen(settings->outlineColor));
    if (sourceControl)
        painter->drawRect(scene()->outerRect(DesignerScene::rect(sourceControl)));
    if (targetControl)
        painter->drawRect(scene()->outerRect(DesignerScene::contentRect(targetControl)));

    const QLineF line(scene()->anchorLayer()->sourceScenePos(), scene()->anchorLayer()->targetScenePos());
    bool twist = line.angle() < 90 || (line.angle() > 180 && line.angle() < 270);
    auto normal = line.normalVector();
    if (twist)
        normal.setAngle(normal.angle() + 180);
    normal.setLength(0.7 * normal.length() * qSin(M_PI * (int(line.angle()) % 90) / 90.));
    normal.translate(line.center() - line.p1());
    normal.translate(normal.p1() - normal.center());
    QPainterPath curve;
    curve.moveTo(line.p1());
    curve.cubicTo(normal.p2(), normal.p1(), line.p2());
    painter->setBrush(Qt::NoBrush);
    painter->setPen(DesignerScene::pen(settings->outlineColor, 2));
    painter->setRenderHint(QPainter::Antialiasing); // No SmoothPixmapTransform
    painter->drawPath(curve);

    static const QPixmap& anchorPixmap = PaintUtils::renderOverlaidPixmap(":/images/anchor.svg", Qt::white, devicePixelRatio());
    const qreal z = scene()->zoomLevel();
    const qreal m = 10;
    const qreal angle = -90 - line.angle() - 45 * qSin(M_PI * (int(line.angle()) % 90) / 90.) * (twist ? -1 : 1);
    QRectF bumpRectangle(0, 0, m/z, m/z);

    bumpRectangle.moveTo(line.p1().x() - m/2/z, line.p1().y() - m/2/z);
    painter->setPen(DesignerScene::pen(settings->outlineColor, 2));
    painter->setBrush(Qt::white);
    bumpRectangle.adjust(2/z, 2/z, -2/z, -2/z);
    painter->drawRoundedRect(bumpRectangle, bumpRectangle.width() / 2, bumpRectangle.height() / 2);
    bumpRectangle.adjust(-2/z, -2/z, 2/z, 2/z);

    bumpRectangle.moveTo(line.p2().x() - m/2/z, line.p2().y() - m/2/z);
    painter->setBrush(painter->pen().color());
    painter->drawRoundedRect(bumpRectangle, bumpRectangle.width() / 2, bumpRectangle.height() / 2);
    painter->translate(bumpRectangle.center());
    painter->rotate(angle);
    if (targetControl) {
        painter->scale(1/z, 1/z);
        painter->drawPixmap(QRect(-m/2, -m/2, m, m), anchorPixmap, anchorPixmap.rect());
    } else {
        painter->setBrush(Qt::white);
        painter->setPen(DesignerScene::pen("#c00000", 3));
        painter->drawRoundedRect(QRect(-m/2/z, -m/2/z, m/z, m/z), m/2/z, m/2/z);
        painter->drawLine(QPointF(-m/2/z + 2/z, -m/2/z + 2/z), QPointF(m/2/z - 2/z, m/2/z - 2/z));
    }
    painter->restore();
}

void PaintLayer::paintHoverOutline(QPainter* painter)
{
    Q_ASSERT(scene());
    if (DesignerScene::showMouseoverOutline()) {
        if (const DesignerItem* item = scene()->topLevelControl(scene()->cursorPos())) {
            if (!item->isSelected()) {
                painter->setBrush(Qt::NoBrush);
                painter->setPen(DesignerScene::pen());
                painter->drawRect(DesignerScene::outerRect(item->mapRectToScene(item->rect())));
            }
        }
    }
}

void PaintLayer::paintGuidelines(QPainter* painter)
{
    Q_ASSERT(scene());
    const QVector<QLineF>& lines = scene()->guidelines();
    if (!lines.isEmpty()) {
        painter->setBrush(DesignerScene::outlineColor());
        painter->setPen(DesignerScene::pen());
        painter->drawLines(lines);
        for (const QLineF& line : lines) {
            painter->drawRoundedRect(QRectF(line.p1() - QPointF(1.5, 1.5), QSizeF(3.0, 3.0)), 1.5, 1.5);
            painter->drawRoundedRect(QRectF(line.p2() - QPointF(1.5, 1.5), QSizeF(3.0, 3.0)), 1.5, 1.5);
        }
    }
}

void PaintLayer::paintSelectionOutlines(QPainter* painter)
{
    Q_ASSERT(scene());
    const qreal z = DesignerScene::zoomLevel();
    const qreal m = 0.5 / z;
    QPainterPath outlinesPath, resizersPath;
    resizersPath.setFillRule(Qt::WindingFill);
    for (DesignerItem* selectedItem : scene()->selectedItems()) {
        const QRectF& rect = selectedItem->mapRectToScene(selectedItem->rect());
        QPainterPath outlinePath;
        outlinePath.addRect(rect.adjusted(-m, -m, m, m));
        outlinePath.addRect(rect.adjusted(m, m, -m, -m));
        outlinesPath |= outlinePath;
        for (ResizerItem* resizer : scene()->gadgetLayer()->resizers(selectedItem)) {
            if (resizer->isVisible()) {
                resizersPath.addRect(QRectF(resizer->scenePos() + resizer->rect().topLeft() / z,
                                            resizer->size() / z));
            }
        }
    }
    painter->setPen(Qt::NoPen);
    painter->setBrush(DesignerScene::outlineColor());
    painter->drawPath(outlinesPath.subtracted(resizersPath));
}

void PaintLayer::paintMovingSelectionOutline(QPainter* painter)
{
    Q_ASSERT(scene());
    // Only one item can be resized at a time, so this piece of code
    // wouldn't be triggered for resize operations, also a form could
    // only be included when the form is resized, forms don't move. So
    // we don't have to remove form instances from the list either.
    // In short, this piece of code is only triggered for dragged childs.
    const QList<DesignerItem*>& items = scene()->draggedResizedSelectedItems();
    if (items.size() > 1) // Multiple items moving
        DesignerScene::drawDashRect(painter, DesignerScene::outerRect(DesignerScene::itemsBoundingRect(items)));
}

void PaintLayer::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
{
    paintHoverOutline(painter);
    paintSelectionOutlines(painter);
    paintMovingSelectionOutline(painter);
    paintGuidelines(painter);
    paintAnchors(painter);
    paintAnchorConnection(painter);
}
