#include <gadgetlayer.h>
#include <designerscene.h>
#include <headlineitem.h>
#include <resizeritem.h>
#include <QCursor>

GadgetLayer::GadgetLayer(DesignerItem* parent) : DesignerItem(parent)
  , m_formHeadlineItem(new HeadlineItem(this))
  , m_headlineItem(new HeadlineItem(this))
{
    m_formHeadlineItem->setPen(QPen(Qt::white));
    m_formHeadlineItem->setBrush(Qt::darkGray);
    m_formHeadlineItem->setCursor(Qt::OpenHandCursor);

    m_headlineItem->setPen(QPen(Qt::white));
    m_headlineItem->setBrush(DesignerScene::outlineColor());
    m_headlineItem->setCursor(Qt::OpenHandCursor);

    connect(m_headlineItem, &HeadlineItem::doubleClicked,
            this, [=] { emit headlineDoubleClicked(false); });
    connect(m_formHeadlineItem, &HeadlineItem::doubleClicked,
            this, [=] { emit headlineDoubleClicked(true); });
}

void GadgetLayer::addResizers(DesignerItem* item)
{
    if (m_resizerHash.contains(item))
        return;

    QList<ResizerItem*> resizers;
    for (int i = 0; i < 8; ++i) {
        auto resizer = new ResizerItem(ResizerItem::Placement(i), this);
        resizer->setPen(DesignerScene::pen());
        resizer->setTargetItem(item); // Disconnects previous connections
        resizer->setVisible(item->isSelected() && item->resizable());
        resizer->updatePosition();
        connect(item, &DesignerItem::geometryChanged,
                resizer, &ResizerItem::updatePosition);
        connect(item, &DesignerItem::resizableChanged,
                resizer, [=] { resizer->setVisible(item->isSelected() && item->resizable()); });
        resizers.append(resizer);
    }
    connect(item, &DesignerItem::geometryChanged,
            this, [=] { updateResizerPositions(item); });

    m_resizerHash.insert(item, resizers);
}

void GadgetLayer::removeResizers(DesignerItem* item)
{
    item->disconnect(this);
    for (ResizerItem* resizer : resizers(item))
        delete resizer;
    m_resizerHash.remove(item);
}

QList<ResizerItem*> GadgetLayer::resizers(DesignerItem* item) const
{
    return m_resizerHash.value(item);
}

void GadgetLayer::handleSceneSelectionChange()
{
    Q_ASSERT(scene());
    DesignerItem* currentForm = scene()->currentForm();
    if (currentForm)
        m_formHeadlineItem->setBrush(currentForm->isSelected() ? DesignerScene::outlineColor() : Qt::darkGray);
    for (DesignerItem* item : m_resizerHash.keys()) {
        for (ResizerItem* resizer : resizers(item)) {
            if (item->isVisible()) {
                bool show = item->isSelected() && item->resizable();
                resizer->setVisible(show);
                if (show)
                    resizer->updatePosition();
            }
        }
    }
    QList<DesignerItem*> selectedItems = scene()->selectedItems();
    if (currentForm && currentForm->isSelected())
        selectedItems.removeOne(currentForm);
    if (m_headlineItem->targetItem()) {
        m_headlineItem->targetItem()->disconnect(m_headlineItem);
        if (m_headlineItem->targetItem()->topLevelItem())
            m_headlineItem->targetItem()->topLevelItem()->disconnect(m_headlineItem);
    }
    if (selectedItems.size() == 1) {
        DesignerItem* selectedItem = selectedItems.first();
        // Move headline even if the form of the selected item moves
        connect(selectedItem->topLevelItem(), &DesignerItem::geometryChanged,
                m_headlineItem, [=] { m_headlineItem->updateGeometry(); });
        connect(selectedItem, &DesignerItem::geometryChanged,
                m_headlineItem, [=] { m_headlineItem->updateGeometry(); });
        connect(selectedItem, &DesignerItem::objectNameChanged,
                m_headlineItem, &HeadlineItem::setText);
        m_headlineItem->setTargetItem(selectedItem);
        m_headlineItem->setText(selectedItem->objectName()); // id
        m_headlineItem->updateGeometry(false);
        m_headlineItem->setVisible(true);
    } else {
        m_headlineItem->setVisible(false);
    }
}

void GadgetLayer::handleSceneCurrentFormChange(DesignerItem* formItem)
{
    if (m_formHeadlineItem->targetItem())
        m_formHeadlineItem->targetItem()->disconnect(m_formHeadlineItem);
    if (formItem) {
        connect(formItem, &DesignerItem::objectNameChanged,
                m_formHeadlineItem, &HeadlineItem::setText);
        connect(formItem, &DesignerItem::geometryChanged,
                m_formHeadlineItem, [=] { m_formHeadlineItem->updateGeometry(); });
        connect(formItem, &DesignerItem::geometryChanged,
                m_formHeadlineItem, [=] { m_formHeadlineItem->setDimensions(formItem->size()); });
        m_formHeadlineItem->setTargetItem(formItem);
        m_formHeadlineItem->setText(formItem->objectName()); // id
        m_formHeadlineItem->setDimensions(formItem->size());
        m_formHeadlineItem->updateGeometry(false);
    }
    m_formHeadlineItem->setVisible(formItem);
}

void GadgetLayer::updateResizerPositions(DesignerItem* item)
{
    for (DesignerItem* childItem : item->childItems()) {
        for (ResizerItem* resizer : resizers(childItem))
            resizer->updatePosition();
    }
}
