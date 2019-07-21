#include <gadgetlayer.h>
#include <designerscene.h>
#include <QPainter>

GadgetLayer::GadgetLayer(DesignerItem* parent) : DesignerItem(parent)
{
}

void GadgetLayer::addItem(DesignerItem* item)
{
    m_items.append(item);

}

void GadgetLayer::removeItem(DesignerItem* item)
{
    m_items.removeOne(item);
}

void GadgetLayer::paintSelectionOutline(QPainter* painter)
{
    painter->setPen(scene()->pen(scene()->outlineColor(), 2));
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(rect());
}

void GadgetLayer::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{

}
