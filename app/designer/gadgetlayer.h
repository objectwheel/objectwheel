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
    QList<ResizerItem*> resizers() const;

private slots:
    void onSceneSelectionChange();
    void onSceneCurrentFormChange(DesignerItem* formItem);

signals:
    void headlineDoubleClicked(bool isFormHeadline);

private:
    QList<ResizerItem*> m_resizers;
    HeadlineItem* m_formHeadlineItem;
    HeadlineItem* m_headlineItem;
};

#endif // GADGETLAYER_H
