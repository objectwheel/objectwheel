#include <paintlayer.h>
#include <designerscene.h>
#include <resizeritem.h>
#include <gadgetlayer.h>
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
    AnchorLineHorizontalMask = AnchorLineLeft | AnchorLineRight | AnchorLineHorizontalCenter,
    AnchorLineVerticalMask = AnchorLineTop | AnchorLineBottom | AnchorLineVerticalCenter | AnchorLineBaseline,
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

static QPointF createParentAnchorPoint(const Control* parentControl, AnchorLineType anchorLineType, const Control* childControl)
{
    QRectF parentBoundingRect = parentControl->mapRectToScene(parentControl->rect());
    QRectF childBoundingRect = childControl->mapRectToScene(childControl->rect());

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

static QPointF createAnchorPoint(const Control* control, AnchorLineType anchorLineType)
{
    QRectF boundingRect = control->mapRectToScene(control->rect());

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

static void updateAnchorData(AnchorData& data, const AnchorLine& sourceAnchorLine, const AnchorLine& targetAnchorLine)
{
    if (sourceAnchorLine.control() && targetAnchorLine.control()) {
        data.sourceAnchorLineType = sourceAnchorLine.type();
        data.targetAnchorLineType = targetAnchorLine.type();

        data.startPoint = createAnchorPoint(sourceAnchorLine.control(), sourceAnchorLine.type());

        if (targetAnchorLine.control() == sourceAnchorLine.control()->parentControl())
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
#include <QDebug>
static AnchorLine makeAnchorLine(const QStringList& anchorPair, DesignerScene* scene)
{
    qDebug() << anchorPair;
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
    scenePath.addRect(rect());
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
                updateAnchorData(data, sourceLine, targetLine);

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

                QPen greenPen(QColor("#6fae3b"), 2);
                greenPen.setCosmetic(true);
                painter->setPen(greenPen);
                painter->drawLine(data.sourceAnchorLineFirstPoint, data.sourceAnchorLineSecondPoint);

                bumpRectangle.moveTo(data.startPoint.x() - m / 2, data.startPoint.y() - m / 2);
                painter->setBrush(painter->pen().color());
                painter->setRenderHint(QPainter::Antialiasing, true);
                painter->drawChord(bumpRectangle, startAngleForAnchorLine(data.sourceAnchorLineType), 180 * AngleDegree);
                painter->setRenderHint(QPainter::Antialiasing, false);

                QPen bluePen(QColor("#a25db4"), 2);
                bluePen.setCosmetic(true);
                painter->setPen(bluePen);
                painter->drawLine(data.targetAnchorLineFirstPoint, data.targetAnchorLineSecondPoint);

                bumpRectangle.moveTo(data.endPoint.x() - m / 2, data.endPoint.y() - m / 2);
                painter->setBrush(painter->pen().color());
                painter->setRenderHint(QPainter::Antialiasing, true);
                painter->drawChord(bumpRectangle, startAngleForAnchorLine(data.targetAnchorLineType), 180 * AngleDegree);
                painter->setRenderHint(QPainter::Antialiasing, false);
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
    paintSelectionOutlines(painter);
    paintMovingSelectionOutline(painter);
    paintAnchors(painter);
    paintGuidelines(painter);
}
