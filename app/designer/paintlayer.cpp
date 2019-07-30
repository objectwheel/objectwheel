#include <paintlayer.h>
#include <designerscene.h>
#include <resizeritem.h>
#include <gadgetlayer.h>
#include <QPainter>

PaintLayer::PaintLayer(DesignerItem* parent) : DesignerItem(parent)
  , m_updateGeometryScheduled(false)
{
}

void PaintLayer::updateGeometry()
{
    if (m_updateGeometryScheduled)
        return;
    m_updateGeometryScheduled = true;
    QMetaObject::invokeMethod(this, [=] {
        setGeometry(QPointF(), scene()->sceneRect().size());
        m_updateGeometryScheduled = false;
    }, Qt::QueuedConnection);
}

void PaintLayer::paintGuidelines(QPainter* painter)
{
    const QVector<QLineF>& lines = scene()->guidelines();
    if (!lines.isEmpty()) {
        painter->setPen(scene()->pen());
        painter->drawLines(lines);
        for (const QLineF& line : lines) {
            painter->setBrush(scene()->outlineColor());
            painter->drawRoundedRect(QRectF(line.p1() - QPointF(1.5, 1.5), QSizeF(3.0, 3.0)), 1.5, 1.5);
            painter->drawRoundedRect(QRectF(line.p2() - QPointF(1.5, 1.5), QSizeF(3.0, 3.0)), 1.5, 1.5);
        }
    }
}

void PaintLayer::paintSelectionOutlines(QPainter* painter)
{
    const qreal m = 0.5 / scene()->zoomLevel();
    QPainterPath path;
    QPainterPath path2;
    path2.setFillRule(Qt::WindingFill);
    for (DesignerItem* selectedItem : scene()->selectedItems()) {
        const QRectF& rect = selectedItem->mapRectToScene(selectedItem->rect());
        QPainterPath path3;
        path3.addRect(rect.adjusted(-m, -m, m, m));
        path3.addRect(rect.adjusted(m, m, -m, -m));
        path |= path3;
        for (ResizerItem* resizer : scene()->gadgetLayer()->resizers(selectedItem)) {
            if (resizer->isVisible())
                path2.addRect(QRectF(resizer->pos() + resizer->rect().topLeft() / scene()->zoomLevel(),
                                     resizer->size() / scene()->zoomLevel()));
        }
    }
    painter->setPen(Qt::NoPen);
    painter->setBrush(scene()->outlineColor());
    painter->drawPath(path.subtracted(path2));
}

void PaintLayer::paintSelectionSurroundingOutline(QPainter* painter)
{
    // Only one item can be resized at a time, so this piece of code
    // wouldn't be triggered for resize operations, also a form could
    // only be included when the form is resized, forms don't move. So
    // we don't have to remove form instances from the list either.
    // In short, this piece of code is only triggered for dragged childs.
    const QList<DesignerItem*>& items = scene()->draggedResizedSelectedItems();
    if (items.size() > 1) // Multiple items moving
        scene()->paintOutline(painter, scene()->outerRect(scene()->itemsBoundingRect(items)));
}

void PaintLayer::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
{
    paintSelectionOutlines(painter);
    paintSelectionSurroundingOutline(painter);
    paintGuidelines(painter);
}
