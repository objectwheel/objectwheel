#ifndef GADGETLAYER_H
#define GADGETLAYER_H

#include <designeritem.h>

class ResizerItem;
class HeadlineItem;

class GadgetLayer final : public DesignerItem
{
    Q_OBJECT
    Q_DISABLE_COPY(GadgetLayer)

    friend class DesignerScene; // TODO: Remove me later with the assert

public:
    explicit GadgetLayer(DesignerItem* parent = nullptr);
    void addResizers(DesignerItem* item);
    void removeResizers(DesignerItem* item);
    QList<ResizerItem*> resizers(DesignerItem* item) const;

public slots:
    void handleSceneSelectionChange();
    void handleSceneCurrentFormChange(DesignerItem* formItem);

private slots:
    void updateResizerPositions(DesignerItem* item);

signals:
    void headlineDoubleClicked(bool isFormHeadline);

private:
    HeadlineItem* m_formHeadlineItem;
    HeadlineItem* m_headlineItem;
    QHash<DesignerItem*, QList<ResizerItem*>> m_resizerHash;
};

#endif // GADGETLAYER_H
