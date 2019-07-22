#include <gadgetlayer.h>
#include <designerscene.h>
#include <headlineitem.h>
#include <QPainter>

GadgetLayer::GadgetLayer(DesignerScene* scene) : DesignerItem()
  , m_headlineItem(new HeadlineItem(this))
  , m_formHeadlineItem(new HeadlineItem(this))
{
    m_headlineItem->setPen(QPen(Qt::white));
    m_headlineItem->setBrush(DesignerScene::outlineColor());
    m_formHeadlineItem->setVisible(true);
    m_formHeadlineItem->setPen(QPen(Qt::white));
    m_formHeadlineItem->setBrush(Qt::darkGray);

    connect(m_headlineItem, &HeadlineItem::doubleClicked,
            this, [=] { emit headlineDoubleClicked(false); });
    connect(m_formHeadlineItem, &HeadlineItem::doubleClicked,
            this, [=] { emit headlineDoubleClicked(true); });
    connect(scene, &DesignerScene::selectionChanged,
            this, &GadgetLayer::onSceneSelectionChange);
    connect(scene, &DesignerScene::currentFormChanged,
            this, &GadgetLayer::onSceneCurrentFormChange);
}

void GadgetLayer::onSceneSelectionChange()
{
    DesignerItem* currentFormItem = scene()->currentForm();
    if (currentFormItem)
        m_formHeadlineItem->setBrush(currentFormItem->isSelected() ? scene()->outlineColor() : Qt::darkGray);
    QList<DesignerItem*> selectedItems = scene()->selectedItems();
    if (currentFormItem && currentFormItem->isSelected())
        selectedItems.removeOne(currentFormItem);
    if (m_headlineItem->targetItem())
        m_headlineItem->targetItem()->disconnect(m_headlineItem);
    if (selectedItems.size() == 1) {
        DesignerItem* selectedItem = selectedItems.first();
        connect(selectedItem, &DesignerItem::geometryChanged,
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
        m_formHeadlineItem->updateGeometry();
    }
}

void GadgetLayer::paintSelectionOutline(QPainter* painter, DesignerItem* selectedItem)
{
    painter->setPen(scene()->pen(scene()->outlineColor(), 2));
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(selectedItem->mapRectToScene(selectedItem->rect()));
}

void GadgetLayer::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
{
    for (DesignerItem* selectedItem : scene()->selectedItems())
        paintSelectionOutline(painter, selectedItem);
}
