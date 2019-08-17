#include <paintlayer.h>
#include <designerscene.h>
#include <resizeritem.h>
#include <gadgetlayer.h>
#include <anchorlayer.h>
#include <paintutils.h>
#include <QPainter>

enum { AngleDegree = 16 };
enum AnchorLineType {
    AnchorLineInvalid = 0x0,
    AnchorLineNoAnchor = AnchorLineInvalid,
    AnchorLineLeft = 0x01,
    AnchorLineRight = 0x02,
    AnchorLineTop = 0x04,
    AnchorLineBottom = 0x08,
    AnchorLineHorizontalCenter = 0x10,
    AnchorLineVerticalCenter = 0x20,
    AnchorLineBaseline = 0x40,

    AnchorLineFill =  AnchorLineLeft | AnchorLineRight | AnchorLineTop | AnchorLineBottom,
    AnchorLineCenter = AnchorLineVerticalCenter | AnchorLineHorizontalCenter,
    AnchorLineVerticalMask = AnchorLineLeft | AnchorLineRight | AnchorLineHorizontalCenter,
    AnchorLineHorizontalMask = AnchorLineTop | AnchorLineBottom | AnchorLineVerticalCenter | AnchorLineBaseline,
    AnchorLineAllMask = AnchorLineVerticalMask | AnchorLineHorizontalMask
};

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

class AnchorLine
{
public:
    AnchorLine();
    AnchorLine(Control* control, AnchorLineType type);
    AnchorLineType type() const;
    qreal margin() const;
    bool isValid() const;

    static bool isHorizontalAnchorLine(AnchorLineType anchorline);
    static bool isVerticalAnchorLine(AnchorLineType anchorline);

    Control* control() const;

private:
    Control* m_control;
    AnchorLineType m_type;
};

AnchorLine::AnchorLine()
    : m_control(nullptr)
    , m_type(AnchorLineInvalid)
{}

AnchorLine::AnchorLine(Control* control, AnchorLineType type)
    : m_control(control),
      m_type(type)
{}

AnchorLineType AnchorLine::type() const
{
    return m_type;
}

qreal AnchorLine::margin() const
{
    if (!isValid())
        return 0;

    qreal margin = 0;

    if (m_type == AnchorLineTop)
        margin = m_control->anchors().value("anchors.topMargin").toReal();
    else if (m_type == AnchorLineBottom)
        margin = m_control->anchors().value("anchors.bottomMargin").toReal();
    else if (m_type == AnchorLineLeft)
        margin = m_control->anchors().value("anchors.leftMargin").toReal();
    else if (m_type == AnchorLineRight)
        margin = m_control->anchors().value("anchors.rightMargin").toReal();

    if (margin == 0)
        return m_control->anchors().value("anchors.margins").toReal();

    return margin;
}

bool AnchorLine::isValid() const
{
    return m_type != AnchorLineInvalid && m_control;
}

bool AnchorLine::isHorizontalAnchorLine(AnchorLineType anchorline)
{
    return anchorline & AnchorLineHorizontalMask;
}

bool AnchorLine::isVerticalAnchorLine(AnchorLineType anchorline)
{
    return anchorline & AnchorLineVerticalMask;
}

Control* AnchorLine::control() const
{
    return m_control;
}

struct AnchorData {
    QPointF startPoint;
    QPointF firstControlPoint;
    QPointF secondControlPoint;
    QPointF endPoint;
    QPointF sourceAnchorLineFirstPoint;
    QPointF sourceAnchorLineSecondPoint;
    QPointF targetAnchorLineFirstPoint;
    QPointF targetAnchorLineSecondPoint;
    AnchorLineType sourceAnchorLineType = AnchorLineInvalid;
    AnchorLineType targetAnchorLineType = AnchorLineInvalid;
};

static QPointF createParentAnchorPoint(const DesignerItem* parentItem, AnchorLineType anchorLineType, const DesignerItem* childItem)
{
    QRectF parentBoundingRect = parentItem->mapRectToScene(parentItem->rect());
    QRectF childBoundingRect = childItem->mapRectToScene(childItem->rect());

    QPointF anchorPoint;

    switch (anchorLineType) {
    case AnchorLineTop:
        anchorPoint = QPointF(childBoundingRect.center().x(), parentBoundingRect.top());
        break;
    case AnchorLineBottom:
        anchorPoint = QPointF(childBoundingRect.center().x(), parentBoundingRect.bottom());
        break;
    case AnchorLineLeft:
        anchorPoint = QPointF(parentBoundingRect.left(), childBoundingRect.center().y());
        break;
    case AnchorLineRight:
        anchorPoint = QPointF(parentBoundingRect.right(), childBoundingRect.center().y());
        break;
    default:
        break;
    }

    return anchorPoint;
}

static QPointF createAnchorPoint(const DesignerItem* item, AnchorLineType anchorLineType)
{
    QRectF boundingRect = item->mapRectToScene(item->rect());

    QPointF anchorPoint;

    switch (anchorLineType) {
    case AnchorLineTop:
        anchorPoint = QPointF(boundingRect.center().x(), boundingRect.top());
        break;
    case AnchorLineBottom:
        anchorPoint = QPointF(boundingRect.center().x(), boundingRect.bottom());
        break;
    case AnchorLineLeft:
        anchorPoint = QPointF(boundingRect.left(), boundingRect.center().y());
        break;
    case AnchorLineRight:
        anchorPoint = QPointF(boundingRect.right(), boundingRect.center().y());
        break;
    default:
        break;
    }

    return anchorPoint;
}

static QPointF createControlPoint(const QPointF &firstEditPoint, AnchorLineType anchorLineType, const QPointF &secondEditPoint)
{
    QPointF controlPoint = firstEditPoint;

    switch (anchorLineType) {
    case AnchorLineTop:
    case AnchorLineBottom:
        controlPoint.ry() += (secondEditPoint.y() - firstEditPoint.y()) / 2.0;
        break;
    case AnchorLineLeft:
    case AnchorLineRight:
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
    case AnchorLineTop:
        *firstPoint = boundingRectangle.topLeft();
        *secondPoint = boundingRectangle.topRight();
        break;
    case AnchorLineBottom:
        *firstPoint = boundingRectangle.bottomLeft();
        *secondPoint = boundingRectangle.bottomRight();
        break;
    case AnchorLineLeft:
        *firstPoint = boundingRectangle.topLeft();
        *secondPoint = boundingRectangle.bottomLeft();
        break;
    case AnchorLineRight:
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

static int startAngleForAnchorLine(const AnchorLineType &anchorLineType)
{
    switch (anchorLineType) {
    case AnchorLineTop:
        return 0;
    case AnchorLineBottom:
        return 180 * AngleDegree;
    case AnchorLineLeft:
        return 90 * AngleDegree;
    case AnchorLineRight:
        return 270 * AngleDegree;
    default:
        return 0;
    }
}

static int anchorPixmapAngle(const AnchorData& data)
{
    if (data.targetAnchorLineType == AnchorLineInvalid)
        return 0;

    const QPointF& offset = data.endPoint - data.startPoint;
    if (AnchorLine::isVerticalAnchorLine(data.targetAnchorLineType)) {
        if (offset.x() < 0)
            return 90;
        return -90;
    } else {
        if (offset.y() < 0)
            return 180;
        return 0;
    }
}

static AnchorLine makeAnchorLine(const QStringList& anchorPair, DesignerScene* scene)
{
    if (anchorPair.size() != 2)
        return AnchorLine();
    AnchorLineType type;
    if (anchorPair.first() == anchorLineNames.at(0))
        type = AnchorLineTop;
    else if (anchorPair.first() == anchorLineNames.at(1))
        type = AnchorLineBottom;
    else if (anchorPair.first() == anchorLineNames.at(2))
        type = AnchorLineLeft;
    else if (anchorPair.first() == anchorLineNames.at(3))
        type = AnchorLineRight;
    else if (anchorPair.first() == anchorLineNames.at(4))
        type = AnchorLineHorizontalCenter;
    else if (anchorPair.first() == anchorLineNames.at(5))
        type = AnchorLineVerticalCenter;
    else if (anchorPair.first() == anchorLineNames.at(6))
        type = AnchorLineBaseline;
    else if (anchorPair.first() == anchorLineNames.at(7))
        type = AnchorLineFill;
    else if (anchorPair.first() == anchorLineNames.at(8))
        type = AnchorLineCenter;
    for (Control* control : scene->items<Control>()) {
        if (control->uid() == anchorPair.last())
            return AnchorLine(control, type);
    }
    return AnchorLine();
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
    QPainterPath scenePath;
    scenePath.addRect(scene()->sceneRect());
    for (Control* selectedControl : scene()->selectedControls()) {
        QPainterPath resizersPath;
        for (ResizerItem* resizer : scene()->gadgetLayer()->resizers(selectedControl)) {
            if (resizer->isVisible()) {
                resizersPath.addRect(QRectF(resizer->scenePos() + resizer->rect().topLeft() / z,
                                            resizer->size() / z));
            }
        }

        for (const QString& sourceLineName : selectedControl->anchors().keys()) {
            if (anchorLineNames.contains(sourceLineName)) {
                AnchorData data;
                AnchorLine sourceLine(makeAnchorLine({sourceLineName, selectedControl->uid()}, scene()));
                AnchorLine targetLine(makeAnchorLine(selectedControl->anchors().value(sourceLineName).value<QStringList>(), scene()));
                updateAnchorData(data, sourceLine, targetLine, scene());

                for (ResizerItem* resizer : scene()->gadgetLayer()->resizers(targetLine.control())) {
                    if (resizer->isVisible()) {
                        resizersPath.addRect(QRectF(resizer->scenePos() + resizer->rect().topLeft() / z,
                                                    resizer->size() / z));
                    }
                }
                painter->setClipPath(scenePath.subtracted(resizersPath));

                DesignerScene::drawDashLine(painter, {data.startPoint, data.firstControlPoint});
                DesignerScene::drawDashLine(painter, {data.firstControlPoint, data.secondControlPoint});
                DesignerScene::drawDashLine(painter, {data.secondControlPoint, data.endPoint});

                painter->setPen(Qt::white);
                painter->setBrush(DesignerScene::outlineColor());
                paintLabelOverLine(painter, QString::number(sourceLine.margin()), {data.firstControlPoint, data.secondControlPoint});

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
                painter->setRenderHint(QPainter::SmoothPixmapTransform, true);
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
                painter->setRenderHint(QPainter::SmoothPixmapTransform, false);
                painter->setRenderHint(QPainter::Antialiasing, false);
            }
        }
    }
    painter->setClipping(false);
}
#include <QDebug>
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
    scenePath.setFillRule(Qt::WindingFill);
    scenePath.addRect(scene()->sceneRect());
    const Control* sourceControl = scene()->topLevelControl(line.p1());
    const Control* destinationControl = scene()->topLevelControl(line.p2());
    if (sourceControl) {
        const QRectF& r = DesignerScene::outerRect(sourceControl->mapRectToScene(sourceControl->rect()));
        QPainterPath p;
        p.addRect(r);
        scenePath = scenePath.subtracted(p);
        for (const Control* childControl : sourceControl->childControls(false))
            scenePath.addRect(DesignerScene::outerRect(childControl->mapRectToScene(childControl->rect())));
        painter->drawRect(r);
    }
    if (destinationControl) {
        const QRectF& r = DesignerScene::outerRect(destinationControl->mapRectToScene(destinationControl->rect()));
        QPainterPath p;
        p.addRect(r);
        scenePath = scenePath.subtracted(p);
        for (const Control* childControl : destinationControl->childControls(false)) {
            if (childControl != sourceControl)
                scenePath.addRect(DesignerScene::outerRect(childControl->mapRectToScene(childControl->rect())));
        }
        painter->drawRect(r);
    }
    painter->setBrush(QColor(0, 0, 0, 20));
    painter->drawPath(scenePath);


    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setRenderHint(QPainter::SmoothPixmapTransform, true);

    QPainterPath curve;
    curve.moveTo(line.p1());
    curve.cubicTo(line.center() + QPointF(50, 50), line.center() - QPointF(50, 50), line.p2());
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
    const qreal angle = -90 - line.angle();
    QPixmap p = PaintUtils::renderOverlaidPixmap(":/images/anchor.svg", Qt::white, devicePixelRatio());
    const QPointF& tr = bumpRectangle.center();
    bumpRectangle.moveTopLeft({-bumpRectangle.width() * 0.5, -bumpRectangle.height() * 0.5});
    painter->translate(tr);
    painter->rotate(angle);
    painter->drawPixmap(bumpRectangle.toRect(), p, p.rect());
    painter->rotate(-angle);
    painter->translate(-tr);

    painter->setRenderHint(QPainter::SmoothPixmapTransform, false);
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

void PaintLayer::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
{
    paintSelectionOutlines(painter);
    paintMovingSelectionOutline(painter);
    paintAnchors(painter);
    paintGuidelines(painter);
    paintAnchorConnector(painter);
}
