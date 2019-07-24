#ifndef GADGETLAYER_H
#define GADGETLAYER_H

#include <designeritem.h>

class ResizerItem;
class HeadlineItem;

class GadgetLayer final : public DesignerItem
{
    Q_OBJECT
    Q_DISABLE_COPY(GadgetLayer)

public:
    explicit GadgetLayer(DesignerScene* scene);
    void clearResizers();
    void addResizers(DesignerItem* item);
    void removeResizers(DesignerItem* item);
    QList<ResizerItem*> resizers(DesignerItem* item) const;

private slots:
    void onSceneSelectionChange();
    void onSceneCurrentFormChange(DesignerItem* formItem);

signals:
    void headlineDoubleClicked(bool isFormHeadline);

private:
    HeadlineItem* m_formHeadlineItem;
    HeadlineItem* m_headlineItem;
    QHash<DesignerItem*, QList<ResizerItem*>> m_resizerHash;
};

#endif // GADGETLAYER_H
