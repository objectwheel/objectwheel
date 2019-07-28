#include <gadgetlayer.h>
#include <designerscene.h>
#include <headlineitem.h>
#include <resizeritem.h>
#include <QCursor>

GadgetLayer::GadgetLayer(DesignerScene* scene) : DesignerItem()
  , m_formHeadlineItem(new HeadlineItem(this))
  , m_headlineItem(new HeadlineItem(this))
{
    m_headlineItem->setPen(QPen(Qt::white));
    m_headlineItem->setBrush(DesignerScene::outlineColor());
    m_headlineItem->setCursor(Qt::OpenHandCursor);

    m_formHeadlineItem->setVisible(true);
    m_formHeadlineItem->setPen(QPen(Qt::white));
    m_formHeadlineItem->setBrush(Qt::darkGray);
    m_formHeadlineItem->setCursor(Qt::OpenHandCursor);

    connect(m_headlineItem, &HeadlineItem::doubleClicked,
            this, [=] { emit headlineDoubleClicked(false); });
    connect(m_formHeadlineItem, &HeadlineItem::doubleClicked,
            this, [=] { emit headlineDoubleClicked(true); });
    connect(scene, &DesignerScene::selectionChanged,
            this, &GadgetLayer::onSceneSelectionChange);
    connect(scene, &DesignerScene::currentFormChanged,
            this, &GadgetLayer::onSceneCurrentFormChange);
}

void GadgetLayer::clearResizers()
{
    QList<DesignerItem*> itemList(m_resizerHash.keys());
    for (DesignerItem* item : itemList)
        removeResizers(item);
}

void GadgetLayer::addResizers(DesignerItem* item)
{
    QList<ResizerItem*> resizers;
    for (int i = 0; i < 8; ++i) {
        auto resizer = new ResizerItem(ResizerItem::Placement(i), this);
        resizer->setPen(DesignerScene::pen());
        resizer->setTargetItem(item);
        resizer->setVisible(item->isSelected() && item->resizable());
        resizer->updatePosition();
        connect(item, &DesignerItem::geometryChanged,
                resizer, &ResizerItem::updatePosition);
        connect(item, &DesignerItem::resizableChanged,
                resizer, [=] { resizer->setVisible(item->isSelected() && item->resizable()); });
        resizers.append(resizer);
    }
    connect(item, &DesignerItem::geometryChanged, this, [=] {
        for (DesignerItem* childItem : item->childItems()) {
            for (ResizerItem* resizer : this->resizers(childItem))
                resizer->updatePosition();
        }
    });
    m_resizerHash.insert(item, resizers);
}

void GadgetLayer::removeResizers(DesignerItem* item)
{
    for (ResizerItem* resizer : resizers(item))
        delete resizer;
    item->disconnect(this);
    m_resizerHash.remove(item);
}

QList<ResizerItem*> GadgetLayer::resizers(DesignerItem* item) const
{
    return m_resizerHash.value(item);
}

void GadgetLayer::onSceneSelectionChange()
{
    DesignerItem* currentForm = scene()->currentForm();
    if (currentForm)
        m_formHeadlineItem->setBrush(currentForm->isSelected() ? scene()->outlineColor() : Qt::darkGray);
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
    if (m_headlineItem->targetItem())
        m_headlineItem->targetItem()->disconnect(m_headlineItem);
    if (selectedItems.size() == 1) {
        DesignerItem* selectedItem = selectedItems.first();
        connect(selectedItem, &DesignerItem::geometryChanged,
                m_headlineItem, &HeadlineItem::updateGeometry);
        // Move headline even if the form of the selected item moves
        connect(selectedItem->topLevelItem(), &DesignerItem::geometryChanged,
                m_headlineItem, &HeadlineItem::updateGeometry);
        connect(selectedItem, &DesignerItem::objectNameChanged,
                m_headlineItem, &HeadlineItem::setText);
        m_headlineItem->setTargetItem(selectedItem);
        m_headlineItem->setText(selectedItem->objectName()); // id
        m_headlineItem->updateGeometry();
        m_headlineItem->setVisible(true);
    } else {
        m_headlineItem->setVisible(false);
    }
}

void GadgetLayer::onSceneCurrentFormChange(DesignerItem* formItem)
{
    if (m_formHeadlineItem->targetItem())
        m_formHeadlineItem->targetItem()->disconnect(m_formHeadlineItem);
    if (formItem) {
        connect(formItem, &DesignerItem::geometryChanged,
                m_formHeadlineItem, &HeadlineItem::updateGeometry);
        connect(formItem, &DesignerItem::objectNameChanged,
                m_formHeadlineItem, &HeadlineItem::setText);
        connect(formItem, &DesignerItem::geometryChanged,
                m_formHeadlineItem, [=] { m_formHeadlineItem->setDimensions(formItem->size()); });
        m_formHeadlineItem->setTargetItem(formItem);
        m_formHeadlineItem->setText(formItem->objectName()); // id
        m_formHeadlineItem->setDimensions(formItem->size());
        // Needed, since updateGeometry takes place later
        // We need to set pos to 0,0 in order to correctly
        // calculate visibleItemsBoundingRect in scene
        m_formHeadlineItem->setPos(0, 0);
        m_formHeadlineItem->updateGeometry();
    }
}
