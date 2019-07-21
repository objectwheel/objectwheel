#ifndef GADGETLAYER_H
#define GADGETLAYER_H

#include <designeritem.h>

class GadgetLayer final : public DesignerItem
{
    Q_OBJECT
    Q_DISABLE_COPY(GadgetLayer)

public:
    explicit GadgetLayer(DesignerItem* parent = nullptr);

    void addItem(DesignerItem* item);
    void removeItem(DesignerItem* item);

private:
    void paintSelectionOutline(QPainter* painter);
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

private:
    QList<DesignerItem*> m_items;
};

#endif // GADGETLAYER_H
