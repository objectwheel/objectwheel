#include <gadgetitem.h>
#include <designerscene.h>
#include <designerview.h>
#include <QGraphicsSceneMouseEvent>

GadgetItem::GadgetItem(DesignerItem* parent) : DesignerItem(parent)
{
    QPen pen(Qt::black, 1, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin);
    pen.setCosmetic(true);
    setPen(pen);
    setBrush(Qt::white);
    setVisible(false);
    setFlag(ItemIgnoresTransformations);
}

QPen GadgetItem::pen() const
{
    return m_pen;
}

void GadgetItem::setPen(const QPen& pen)
{
    if (m_pen != pen) {
        m_pen = pen;
        update();
    }
}

QBrush GadgetItem::brush() const
{
    return m_brush;
}

void GadgetItem::setBrush(const QBrush& brush)
{
    if (m_brush != brush) {
        m_brush = brush;
        update();
    }
}

QPointF GadgetItem::dragDistanceVector() const
{
    return m_dragDistanceVector;
}

void GadgetItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    DesignerItem::mousePressEvent(event);
    event->accept();
}

void GadgetItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    DesignerItem::mouseMoveEvent(event);
    // Damn event->button() always returns
    // Qt::NoButton for mouse move events
    m_dragDistanceVector = event->pos() - event->buttonDownPos(scene()->view()->mousePressButton());
}

void GadgetItem::mouseUngrabEvent(QEvent* event)
{
    DesignerItem::mouseUngrabEvent(event);
    m_dragDistanceVector = QPointF();
}
