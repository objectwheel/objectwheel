#ifndef GADGETLAYER_H
#define GADGETLAYER_H

#include <designeritem.h>

class HeadlineItem;
class GadgetLayer final : public DesignerItem
{
    Q_OBJECT
    Q_DISABLE_COPY(GadgetLayer)

public:
    explicit GadgetLayer(DesignerScene* scene);

private slots:
    void onSceneSelectionChange();
    void onSceneCurrentFormChange(DesignerItem* formItem);

private:
    void paintSelectionOutline(QPainter* painter, DesignerItem* selectedItem);
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

signals:
    void headlineDoubleClicked(bool isFormHeadline);

private:
    QList<DesignerItem*> m_items;
    HeadlineItem* m_headlineItem;
    HeadlineItem* m_formHeadlineItem;
};

#endif // GADGETLAYER_H
