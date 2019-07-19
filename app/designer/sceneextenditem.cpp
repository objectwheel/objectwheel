#include <sceneextenditem.h>
#include <control.h>

SceneExtendItem::SceneExtendItem(Control* parent) : QGraphicsItem(parent)
{
    // NOTE: Don't set ItemHasNoContents, otherwise
    // sceneRect() won't count us an intersecting item
    setFlag(ItemStacksBehindParent);
    setAcceptedMouseButtons(Qt::NoButton);
    QObject::connect(parent, &Control::geometryChanged, [=] {
        updateRect();
    });
}

Control* SceneExtendItem::parentControl() const
{
    return static_cast<Control*>(parentItem());
}

QRectF SceneExtendItem::boundingRect() const
{
    return m_rect;
}

void SceneExtendItem::updateRect()
{
    prepareGeometryChange();
    const QRectF& r = parentControl()->rect();
    const QRectF& f = parentControl()->frame();
    QRectF boundingRect = r;
    if (f.width() > r.width() || f.height() > r.height())
        boundingRect = f;
    boundingRect.adjust(-20, -30, 20, 20);
    m_rect = boundingRect;
    update();
}

void SceneExtendItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(painter)
    Q_UNUSED(option)
    Q_UNUSED(widget)
}
