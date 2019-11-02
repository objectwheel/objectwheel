#include <gadgetlayer.h>
#include <designerscene.h>
#include <headlineitem.h>
#include <resizeritem.h>
#include <designersettings.h>
#include <scenesettings.h>
#include <generalsettings.h>
#include <interfacesettings.h>
#include <form.h>
#include <QCursor>

GadgetLayer::GadgetLayer(DesignerItem* parent) : DesignerItem(parent)
  , m_formHeadlineItem(new HeadlineItem(this))
  , m_headlineItem(new HeadlineItem(this))
{
    m_formHeadlineItem->setPen(QPen(Qt::white));
    m_formHeadlineItem->setBrush(Qt::darkGray);
    m_formHeadlineItem->setCursor(Qt::OpenHandCursor);
    m_formHeadlineItem->setMousePressCursorShape(Qt::ClosedHandCursor);

    m_headlineItem->setPen(QPen(Qt::white));
    m_headlineItem->setBrush(GeneralSettings::interfaceSettings()->highlightColor);
    m_headlineItem->setCursor(Qt::OpenHandCursor);
    m_headlineItem->setMousePressCursorShape(Qt::ClosedHandCursor);

    connect(m_formHeadlineItem, &HeadlineItem::doubleClicked,
            this, [=] (Qt::MouseButtons buttons) {
        if (DesignerItem* targetItem = m_formHeadlineItem->targetItem())
            emit targetItem->doubleClicked(buttons);
    });
    connect(m_headlineItem, &HeadlineItem::doubleClicked,
            this, [=] (Qt::MouseButtons buttons) {
        if (DesignerItem* targetItem = m_headlineItem->targetItem())
            emit targetItem->doubleClicked(buttons);
    });
    connect(GeneralSettings::instance(), &GeneralSettings::interfaceSettingsChanged,
            this, [=] {
        Q_ASSERT(scene());
        m_headlineItem->setBrush(GeneralSettings::interfaceSettings()->highlightColor);
        if (const DesignerItem* currentForm = scene()->currentForm()) {
            m_formHeadlineItem->setBrush(currentForm->isSelected()
                                         ? GeneralSettings::interfaceSettings()->highlightColor
                                         : Qt::darkGray);
        }
        const QList<Control*>& allControls = Control::controls();
        for (Control* control : allControls) {
            const QList<ResizerItem*>& resizers_ = resizers(control);
            for (ResizerItem* resizer : resizers_)
                resizer->setPen(DesignerScene::pen());
        }
    });
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
        connect(item, &DesignerItem::scaleChanged,
                resizer, &ResizerItem::updatePosition);
        connect(item, &DesignerItem::rotationChanged,
                resizer, &ResizerItem::updatePosition);
        connect(item, &DesignerItem::transformOriginPointChanged,
                resizer, &ResizerItem::updatePosition);
        connect(item, &DesignerItem::resizableChanged,
                resizer, [=] { resizer->setVisible(item->isSelected() && item->resizable()); });
        resizers.append(resizer);
    }
    connect(item, &DesignerItem::geometryChanged,
            this, [=] { updateResizerPositions(item); });
    connect(item, &DesignerItem::scaleChanged,
            this, [=] { updateResizerPositions(item); });
    connect(item, &DesignerItem::rotationChanged,
            this, [=] { updateResizerPositions(item); });
    connect(item, &DesignerItem::transformOriginPointChanged,
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

QList<ResizerItem*> GadgetLayer::resizers(const DesignerItem* item) const
{
    return m_resizerHash.value(const_cast<DesignerItem*>(item));
}

void GadgetLayer::handleSceneSelectionChange()
{
    Q_ASSERT(scene());
    DesignerItem* currentForm = scene()->currentForm();
    if (currentForm) {
        m_formHeadlineItem->setBrush(currentForm->isSelected()
                                     ? GeneralSettings::interfaceSettings()->highlightColor
                                     : Qt::darkGray);
    }
    for (DesignerItem* item : m_resizerHash.keys()) {
        for (ResizerItem* resizer : resizers(item)) {
            if (item->isVisible()) {
                bool show = item->scene() && item->isSelected() && item->resizable();
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
        connect(selectedItem->topLevelItem(), &DesignerItem::scaleChanged,
                m_headlineItem, [=] { m_headlineItem->updateGeometry(); });
        connect(selectedItem->topLevelItem(), &DesignerItem::rotationChanged,
                m_headlineItem, [=] { m_headlineItem->updateGeometry(); });
        connect(selectedItem->topLevelItem(), &DesignerItem::transformOriginPointChanged,
                m_headlineItem, [=] { m_headlineItem->updateGeometry(); });
        connect(selectedItem, &DesignerItem::geometryChanged,
                m_headlineItem, [=] { m_headlineItem->updateGeometry(); });
        connect(selectedItem, &DesignerItem::scaleChanged,
                m_headlineItem, [=] { m_headlineItem->updateGeometry(); });
        connect(selectedItem, &DesignerItem::rotationChanged,
                m_headlineItem, [=] { m_headlineItem->updateGeometry(); });
        connect(selectedItem, &DesignerItem::transformOriginPointChanged,
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
        connect(formItem, &DesignerItem::scaleChanged,
                m_formHeadlineItem, [=] { m_formHeadlineItem->updateGeometry(); });
        connect(formItem, &DesignerItem::rotationChanged,
                m_formHeadlineItem, [=] { m_formHeadlineItem->updateGeometry(); });
        connect(formItem, &DesignerItem::transformOriginPointChanged,
                m_formHeadlineItem, [=] { m_formHeadlineItem->updateGeometry(); });
        connect(formItem, &DesignerItem::geometryChanged,
                m_formHeadlineItem, [=] { m_formHeadlineItem->setDimensions(formItem->size()); });
        connect(formItem, &DesignerItem::scaleChanged,
                m_formHeadlineItem, [=] { m_formHeadlineItem->setDimensions(formItem->size()); });
        connect(formItem, &DesignerItem::rotationChanged,
                m_formHeadlineItem, [=] { m_formHeadlineItem->setDimensions(formItem->size()); });
        connect(formItem, &DesignerItem::transformOriginPointChanged,
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
