#include <paintlayer.h>
#include <designerscene.h>
#include <resizeritem.h>
#include <gadgetlayer.h>
#include <anchorlayer.h>
#include <paintutils.h>
#include <anchorline.h>

#include <QPainter>
#include <QtMath>

enum { AngleDegree = 16 };

static const QStringList anchorLineNames {
    "anchors.top",
    "anchors.bottom",
    "anchors.left",
    "anchors.right",
    "anchors.horizontalCenter",
    "anchors.verticalCenter",
    "anchors.baseline",
    "anchors.fill",
    "anchors.centerIn"
};

struct AnchorData {
    QPointF startPoint;
    QPointF firstControlPoint;
    QPointF secondControlPoint;
    QPointF endPoint;
    QPointF sourceAnchorLineFirstPoint;
    QPointF sourceAnchorLineSecondPoint;
    QPointF targetAnchorLineFirstPoint;
    QPointF targetAnchorLineSecondPoint;
    AnchorLine::Type sourceAnchorLineType = AnchorLine::Invalid;
    AnchorLine::Type targetAnchorLineType = AnchorLine::Invalid;
};

static QPointF createParentAnchorPoint(const DesignerItem* parentItem, AnchorLine::Type anchorLineType, const DesignerItem* childItem)
{
    QRectF parentBoundingRect = parentItem->mapRectToScene(parentItem->rect());
    QRectF childBoundingRect = childItem->mapRectToScene(childItem->rect());

    QPointF anchorPoint;

    switch (anchorLineType) {
    case AnchorLine::Top:
        anchorPoint = QPointF(childBoundingRect.center().x(), parentBoundingRect.top());
        break;
    case AnchorLine::Bottom:
        anchorPoint = QPointF(childBoundingRect.center().x(), parentBoundingRect.bottom());
        break;
    case AnchorLine::Left:
        anchorPoint = QPointF(parentBoundingRect.left(), childBoundingRect.center().y());
        break;
    case AnchorLine::Right:
        anchorPoint = QPointF(parentBoundingRect.right(), childBoundingRect.center().y());
        break;
    default:
        break;
    }

    return anchorPoint;
}

static QPointF createAnchorPoint(const DesignerItem* item, AnchorLine::Type anchorLineType)
{
    QRectF boundingRect = item->mapRectToScene(item->rect());

    QPointF anchorPoint;

    switch (anchorLineType) {
    case AnchorLine::Top:
        anchorPoint = QPointF(boundingRect.center().x(), boundingRect.top());
        break;
    case AnchorLine::Bottom:
        anchorPoint = QPointF(boundingRect.center().x(), boundingRect.bottom());
        break;
    case AnchorLine::Left:
        anchorPoint = QPointF(boundingRect.left(), boundingRect.center().y());
        break;
    case AnchorLine::Right:
        anchorPoint = QPointF(boundingRect.right(), boundingRect.center().y());
        break;
    default:
        break;
    }

    return anchorPoint;
}

static QPointF createControlPoint(const QPointF &firstEditPoint, AnchorLine::Type anchorLineType, const QPointF &secondEditPoint)
{
    QPointF controlPoint = firstEditPoint;

    switch (anchorLineType) {
    case AnchorLine::Top:
    case AnchorLine::Bottom:
        controlPoint.ry() += (secondEditPoint.y() - firstEditPoint.y()) / 2.0;
        break;
    case AnchorLine::Left:
    case AnchorLine::Right:
        controlPoint.rx() += (secondEditPoint.x() - firstEditPoint.x()) / 2.0;
        break;
    default:
        break;
    }

    return controlPoint;
}

static void updateAnchorLinePoints(QPointF *firstPoint, QPointF *secondPoint, const AnchorLine &anchorLine)
{
    QRectF boundingRectangle = anchorLine.control()->mapRectToScene(anchorLine.control()->rect());

    switch (anchorLine.type()) {
    case AnchorLine::Top:
        *firstPoint = boundingRectangle.topLeft();
        *secondPoint = boundingRectangle.topRight();
        break;
    case AnchorLine::Bottom:
        *firstPoint = boundingRectangle.bottomLeft();
        *secondPoint = boundingRectangle.bottomRight();
        break;
    case AnchorLine::Left:
        *firstPoint = boundingRectangle.topLeft();
        *secondPoint = boundingRectangle.bottomLeft();
        break;
    case AnchorLine::Right:
        *firstPoint = boundingRectangle.topRight();
        *secondPoint = boundingRectangle.bottomRight();
        break;
    default:
        break;
    }
}

static void updateAnchorData(AnchorData& data, const AnchorLine& sourceAnchorLine, const AnchorLine& targetAnchorLine, DesignerScene* scene)
{
    if (sourceAnchorLine.control() && targetAnchorLine.control()) {
        data.sourceAnchorLineType = sourceAnchorLine.type();
        data.targetAnchorLineType = targetAnchorLine.type();

        data.startPoint = createAnchorPoint(sourceAnchorLine.control(), sourceAnchorLine.type());

        if (targetAnchorLine.control() == sourceAnchorLine.control()->parentControl())
            data.endPoint = createParentAnchorPoint(targetAnchorLine.control(), targetAnchorLine.type(), sourceAnchorLine.control());
        else if (sourceAnchorLine.control()->beingDragged() && scene && targetAnchorLine.control() == scene->parentBeforeDrag())
            data.endPoint = createParentAnchorPoint(targetAnchorLine.control(), targetAnchorLine.type(), sourceAnchorLine.control());
        else
            data.endPoint = createAnchorPoint(targetAnchorLine.control(), targetAnchorLine.type());

        data.firstControlPoint = createControlPoint(data.startPoint, sourceAnchorLine.type(), data.endPoint);
        data.secondControlPoint = createControlPoint(data.endPoint, targetAnchorLine.type(), data.startPoint);

        updateAnchorLinePoints(&data.sourceAnchorLineFirstPoint, &data.sourceAnchorLineSecondPoint, sourceAnchorLine);
        updateAnchorLinePoints(&data.targetAnchorLineFirstPoint, &data.targetAnchorLineSecondPoint, targetAnchorLine);
    }
}

static int startAngleForAnchorLine(const AnchorLine::Type &anchorLineType)
{
    switch (anchorLineType) {
    case AnchorLine::Top:
        return 0;
    case AnchorLine::Bottom:
        return 180 * AngleDegree;
    case AnchorLine::Left:
        return 90 * AngleDegree;
    case AnchorLine::Right:
        return 270 * AngleDegree;
    default:
        return 0;
    }
}

static int anchorPixmapAngle(const AnchorData& data)
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

void PaintLayer::paintAnchors(QPainter* painter)
{
    Q_ASSERT(scene());
    const qreal z = DesignerScene::zoomLevel();
    const qreal m = 10 / z;
    QRectF bumpRectangle(0, 0, m, m);
    for (Control* selectedControl : scene()->selectedControls()) {
        for (const QString& sourceLineName : selectedControl->anchors().keys()) {
            if (anchorLineNames.contains(sourceLineName)) {
                AnchorData data;
                AnchorLine sourceLine(makeAnchorLine({sourceLineName, selectedControl->uid()}, scene()));
                AnchorLine targetLine(makeAnchorLine(selectedControl->anchors().value(sourceLineName).value<QStringList>(), scene()));
                updateAnchorData(data, sourceLine, targetLine, scene());

                DesignerScene::drawDashLine(painter, {data.startPoint, data.firstControlPoint});
                DesignerScene::drawDashLine(painter, {data.firstControlPoint, data.secondControlPoint});
                DesignerScene::drawDashLine(painter, {data.secondControlPoint, data.endPoint});

                bumpRectangle.moveTo(data.startPoint.x() - m / 2, data.startPoint.y() - m / 2);
                painter->setPen(DesignerScene::pen(DesignerScene::outlineColor(), 2));
                painter->setBrush(painter->pen().color());
                painter->setRenderHint(QPainter::Antialiasing, true);
                painter->drawChord(bumpRectangle, startAngleForAnchorLine(data.sourceAnchorLineType), 180 * AngleDegree);
                painter->setRenderHint(QPainter::Antialiasing, false);

                painter->drawLine(data.targetAnchorLineFirstPoint, data.targetAnchorLineSecondPoint);

                bumpRectangle.moveTo(data.endPoint.x() - m / 2, data.endPoint.y() - m / 2);
                painter->setBrush(painter->pen().color());
                painter->setRenderHint(QPainter::Antialiasing, true);
                painter->drawRoundedRect(bumpRectangle, bumpRectangle.width() / 2, bumpRectangle.height() / 2);
                const qreal angle = anchorPixmapAngle(data);
                QPixmap p = PaintUtils::renderOverlaidPixmap(":/images/anchor.svg", Qt::white, devicePixelRatio());
                const QPointF& tr = bumpRectangle.center();
                bumpRectangle.moveTopLeft({-bumpRectangle.width() * 0.5, -bumpRectangle.height() * 0.5});
                painter->translate(tr);
                painter->rotate(angle);
                painter->drawPixmap(bumpRectangle.toRect(), p, p.rect());
                painter->rotate(-angle);
                painter->translate(-tr);
                painter->setRenderHint(QPainter::Antialiasing, false);

                painter->setPen(Qt::white);
                painter->setBrush(DesignerScene::outlineColor());
                paintLabelOverLine(painter, QString::number(sourceLine.margin()), {data.firstControlPoint, data.secondControlPoint});
            }
        }
    }
    painter->setClipping(false);
}

void PaintLayer::paintAnchorConnector(QPainter* painter)
{
    Q_ASSERT(scene());

    if (!scene()->anchorLayer()->activated())
        return;

    const QLineF line(scene()->anchorLayer()->mousePressPoint(), scene()->anchorLayer()->mouseLastPoint());
    const qreal z = DesignerScene::zoomLevel();
    const qreal m = 10 / z;
    QRectF bumpRectangle(0, 0, m, m);

    painter->setBrush(Qt::NoBrush);
    painter->setPen(DesignerScene::pen());

    QPainterPath scenePath;
    scenePath.addRect(scene()->sceneRect());
    const Control* sourceControl = scene()->topLevelControl(line.p1());
    const Control* destinationControl = scene()->topLevelControl(line.p2());
    if (sourceControl) {
        const QRectF& r = DesignerScene::outerRect(sourceControl->mapRectToScene(sourceControl->rect()));
        QPainterPath p;
        p.addRect(r);
        scenePath = scenePath.subtracted(p);
        painter->drawRect(r);
    }
    if (destinationControl) {
        const QRectF& r = DesignerScene::outerRect(destinationControl->mapRectToScene(destinationControl->rect()));
        QPainterPath p;
        p.addRect(r);
        scenePath = scenePath.subtracted(p);
        painter->drawRect(r);
    }
    painter->fillPath(scenePath, QColor(0, 0, 0, 50));
    painter->drawRect(scene()->sceneRect());

    painter->setRenderHint(QPainter::Antialiasing, true);

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
    painter->setPen(DesignerScene::pen("#555555", 1, false));
    painter->drawPath(curve);

    bumpRectangle.moveTo(line.p1().x() - m / 2, line.p1().y() - m / 2);
    painter->setPen(DesignerScene::pen("#555555", 1, false));
    painter->setBrush(Qt::white);
    bumpRectangle.adjust(2, 2, -2, -2);
    painter->drawRoundedRect(bumpRectangle, bumpRectangle.width() / 2, bumpRectangle.height() / 2);
    bumpRectangle.adjust(-2, -2, 2, 2);

    bumpRectangle.moveTo(line.p2().x() - m / 2, line.p2().y() - m / 2);
    painter->setBrush(painter->pen().color());
    painter->drawRoundedRect(bumpRectangle, bumpRectangle.width() / 2, bumpRectangle.height() / 2);
    const qreal angle = -90 - line.angle() - 45 * qSin(M_PI * (int(line.angle()) % 90) / 90.) * (twist ? -1 : 1);
    QPixmap p = PaintUtils::renderOverlaidPixmap(":/images/anchor.svg", Qt::white, devicePixelRatio());
    const QPointF& tr = bumpRectangle.center();
    bumpRectangle.moveTopLeft({-bumpRectangle.width() * 0.5, -bumpRectangle.height() * 0.5});
    painter->translate(tr);
    painter->rotate(angle);
    painter->drawPixmap(bumpRectangle.toRect(), p, p.rect());
    painter->rotate(-angle);
    painter->translate(-tr);

    painter->setRenderHint(QPainter::Antialiasing, false);
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

void PaintLayer::paintLabelOverLine(QPainter* painter, const QString& label, const QLineF& line)
{
    QFont f;
    f.setPixelSize(f.pixelSize() - 4);
    const QFontMetrics fm(f); // App default font
    qreal angle = 180 - line.angle();
    if (qAbs(angle) == 180 || qAbs(angle) == 360)
        angle = 0;
    if (qAbs(angle) == 90 || qAbs(angle) == 270)
        angle = 90;
    QRectF rect(0, 0, fm.horizontalAdvance(label) + fm.height() / 2., fm.height());
    rect.moveCenter(line.center());
    const QPointF& tr = rect.center();
    rect.moveTopLeft({-rect.width() * 0.5, -rect.height() * 0.5});
    painter->setFont(f);
    painter->translate(tr);
    painter->rotate(angle);
    painter->setRenderHint(QPainter::Antialiasing, true);
    auto p = painter->pen();
    painter->setPen(Qt::NoPen);
    painter->drawRoundedRect(rect, rect.height() / 2, rect.height() / 2);
    painter->setPen(p);
    painter->drawText(rect, label, QTextOption(Qt::AlignCenter));
    painter->setRenderHint(QPainter::Antialiasing, false);
    painter->rotate(-angle);
    painter->translate(-tr);
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

void PaintLayer::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
{
    paintHoverOutline(painter);
    paintSelectionOutlines(painter);
    paintMovingSelectionOutline(painter);
    paintAnchors(painter);
    paintGuidelines(painter);
    paintAnchorConnector(painter);
}
