#include <paintlayer.h>
#include <designerscene.h>
#include <resizeritem.h>
#include <gadgetlayer.h>
#include <QPainter>

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
    Control m_control;
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

void updateAnchorData(AnchorData& data, const AnchorLine& sourceAnchorLine, const AnchorLine& targetAnchorLine)
{
    if (sourceAnchorLine.control() && targetAnchorLine.control()) {
        m_sourceAnchorLineType = sourceAnchorLine.type();
        m_targetAnchorLineType = targetAnchorLine.type();

        m_startPoint = createAnchorPoint(sourceAnchorLine.control(), sourceAnchorLine.type());

        if (targetAnchorLine.control() == sourceAnchorLine.control()->parentControl())
            m_endPoint = createParentAnchorPoint(targetAnchorLine.control(), targetAnchorLine.type(), sourceAnchorLine.control());
        else
            m_endPoint = createAnchorPoint(targetAnchorLine.control(), targetAnchorLine.type());

        m_firstControlPoint = createControlPoint(m_startPoint, sourceAnchorLine.type(), m_endPoint);
        m_secondControlPoint = createControlPoint(m_endPoint, targetAnchorLine.type(), m_startPoint);

        updateAnchorLinePoints(&m_sourceAnchorLineFirstPoint, &m_sourceAnchorLineSecondPoint, sourceAnchorLine);
        updateAnchorLinePoints(&m_targetAnchorLineFirstPoint, &m_targetAnchorLineSecondPoint, targetAnchorLine);

        updateBoundingRect();
        update();
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
    const qreal m = 8 / z;
    const QRectF bumpRectangle(0, 0, m, m);
    for (DesignerItem* selectedItem : scene()->selectedItems()) {
        AnchorData data;
        updateAnchorData(data, bum, bum);

        DesignerScene::drawDashLine(painter, {data.startPoint, data.firstControlPoint});
        DesignerScene::drawDashLine(painter, {data.firstControlPoint, data.secondControlPoint});
        DesignerScene::drawDashLine(painter, {data.secondControlPoint, data.endPoint});

        QPen greenPen(Qt::green, 2);
        greenPen.setCosmetic(true);
        painter->setPen(greenPen);
        painter->drawLine(data.sourceAnchorLineFirstPoint, data.sourceAnchorLineSecondPoint);

        bumpRectangle.moveTo(data.startPoint.x() - m / 2, data.startPoint.y() - m / 2);
        painter->setBrush(painter->pen().color());
        painter->setRenderHint(QPainter::Antialiasing, true);
        painter->drawChord(bumpRectangle, startAngleForAnchorLine(data.sourceAnchorLineType), 180 * AngleDegree);
        painter->setRenderHint(QPainter::Antialiasing, false);

        QPen bluePen(Qt::blue, 2);
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
    paintGuidelines(painter);
}
