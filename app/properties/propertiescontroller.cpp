#include <propertiescontroller.h>
#include <propertiespane.h>

PropertiesController::PropertiesController(PropertiesPane* propertiesPane, QObject* parent) : QObject(parent)
  , m_propertiesPane(propertiesPane)
{
    connect(m_idEdit, &QLineEdit::editingFinished,
            this, &PropertiesController::onControlIdEditingFinish);
    connect(m_indexEdit, &QSpinBox::editingFinished,
            this, &PropertiesController::onControlIndexEditingFinish);
    connect(m_searchEdit, &LineEdit::textChanged, this, &PropertiesController::filterList);
    connect(m_designerScene, &DesignerScene::currentFormChanged,
            this, &PropertiesController::onSceneSelectionChange);
    connect(m_designerScene, &DesignerScene::selectionChanged,
            this, &PropertiesController::onSceneSelectionChange);
    connect(ControlPropertyManager::instance(), &ControlPropertyManager::zChanged,
            this, &PropertiesController::onControlZChange);
    connect(ControlPropertyManager::instance(), &ControlPropertyManager::renderInfoChanged,
            this, &PropertiesController::onControlRenderInfoChange);
    connect(ControlPropertyManager::instance(), &ControlPropertyManager::geometryChanged,
            this, &PropertiesController::onControlGeometryChange);
    connect(ControlPropertyManager::instance(), &ControlPropertyManager::propertyChanged,
            this, &PropertiesController::onControlPropertyChange);
    connect(ControlPropertyManager::instance(), &ControlPropertyManager::idChanged,
            this, &PropertiesController::onControlIdChange);
    connect(ControlPropertyManager::instance(), &ControlPropertyManager::indexChanged,
            this, &PropertiesController::onControlIndexChange);
}

void PropertiesController::discharge()
{
    m_searchEdit->clear();
    clear();
}

// FIXME: This function has severe performance issues.
void PropertiesController::onSceneSelectionChange()
{
    clear();

    if (m_designerScene->selectedControls().size() != 1)
        return;

    Control* selectedControl = m_designerScene->selectedControls().first();
    setDisabled(selectedControl->hasErrors());

    QVector<PropertyNode> properties = selectedControl->properties();
    if (properties.isEmpty())
        return;

    m_typeItem->setText(1, properties.first().cleanClassName);
    m_uidItem->setText(1, selectedControl->uid());
    m_idEdit->setText(selectedControl->id());
    m_indexEdit->setValue(selectedControl->index());

    for (const PropertyNode& propertyNode : properties) {
        const QVector<Enum>& enumList = propertyNode.enums;
        const QMap<QString, QVariant>& propertyMap = propertyNode.properties;

        if (propertyMap.isEmpty() && enumList.isEmpty())
            continue;

        auto classItem = new QTreeWidgetItem;
        classItem->setText(0, propertyNode.cleanClassName);
        addTopLevelItem(classItem);

        for (const QString& propertyName : propertyMap.keys()) {
            const QVariant& propertyValue = propertyMap.value(propertyName);

            switch (propertyValue.type()) {
            case QVariant::Font: {
                const QFont& font = propertyValue.value<QFont>();
                createAndAddFontPropertiesBlock(classItem, font, selectedControl);
                break;
            }

            case QVariant::Color: {
                const QColor& color = propertyValue.value<QColor>();
                auto item = new QTreeWidgetItem;
                item->setText(0, propertyName);
                item->setData(0, Qt::DecorationRole,
                              ParserUtils::exists(selectedControl->dir(), propertyName));
                classItem->addChild(item);
                setItemWidget(item, 1,
                              createColorHandlerWidget(propertyName, color, selectedControl));
                break;
            }

            case QVariant::Bool: {
                const bool checked = propertyValue.value<bool>();
                auto item = new QTreeWidgetItem;
                item->setText(0, propertyName);
                item->setData(0, Qt::DecorationRole,
                              ParserUtils::exists(selectedControl->dir(), propertyName));
                classItem->addChild(item);
                setItemWidget(item, 1,
                              createBoolHandlerWidget(propertyName, checked, selectedControl));
                break;
            }

            case QVariant::String: {
                const QString& text = propertyValue.value<QString>();
                auto item = new QTreeWidgetItem;
                item->setText(0, propertyName);
                item->setData(0, Qt::DecorationRole,
                              ParserUtils::exists(selectedControl->dir(), propertyName));
                classItem->addChild(item);
                setItemWidget(item, 1,
                              createStringHandlerWidget(propertyName, text, selectedControl));
                break;
            }

            case QVariant::Url: {
                const QUrl& url = propertyValue.value<QUrl>();
                const QString& displayText = urlToDisplayText(url, selectedControl->dir());
                auto item = new QTreeWidgetItem;
                item->setText(0, propertyName);
                item->setData(0, Qt::DecorationRole,
                              ParserUtils::exists(selectedControl->dir(), propertyName));
                classItem->addChild(item);
                setItemWidget(item, 1,
                              createUrlHandlerWidget(propertyName, displayText, selectedControl));
                break;
            }

            case QVariant::Double: {
                if (isXProperty(propertyName)) {
                    createAndAddGeometryPropertiesBlock(classItem, properties, selectedControl, false);
                } else {
                    if (isGeometryProperty(propertyName))
                        break;

                    double number = propertyValue.value<double>();
                    auto item = new QTreeWidgetItem;
                    item->setText(0, propertyName);
                    item->setData(0, Qt::DecorationRole,
                                  ParserUtils::exists(selectedControl->dir(), propertyName));
                    classItem->addChild(item);
                    setItemWidget(item, 1,
                                  createNumberHandlerWidget(propertyName, number, selectedControl, false));
                }
                break;
            }

            case QVariant::Int: {
                if (isXProperty(propertyName)) {
                    createAndAddGeometryPropertiesBlock(classItem, properties, selectedControl, true);
                } else {
                    if (isGeometryProperty(propertyName))
                        break;

                    int number = propertyValue.value<int>();
                    auto item = new QTreeWidgetItem;
                    item->setText(0, propertyName);
                    item->setData(0, Qt::DecorationRole,
                                  ParserUtils::exists(selectedControl->dir(), propertyName));
                    classItem->addChild(item);
                    setItemWidget(item, 1,
                                  createNumberHandlerWidget(propertyName, number, selectedControl, true));
                }
                break;
            }

            default:
                break;
            }
        }

        for (const Enum& enumm : enumList) {
            auto item = new QTreeWidgetItem;
            item->setText(0, enumm.name);
            item->setData(0, Qt::DecorationRole, ParserUtils::exists(selectedControl->dir(), enumm.name));
            classItem->addChild(item);
            setItemWidget(item, 1, createEnumHandlerWidget(enumm, selectedControl));
        }

        expandItem(classItem);
    }

    filterList(m_searchEdit->text());
}

void PropertiesController::onControlZChange(Control* control)
{
    if (topLevelItemCount() <= 0)
        return;

    if (m_designerScene->selectedControls().size() != 1)
        return;

    Control* selectedControl = m_designerScene->selectedControls().first();
    if (selectedControl != control)
        return;

    for (QTreeWidgetItem* topLevelItem : topLevelItems(this)) {
        for (QTreeWidgetItem* childItem : allSubChildItems(topLevelItem)) {
            if (childItem->text(0) == "z") {
                QTreeWidget* treeWidget = childItem->treeWidget();
                Q_ASSERT(treeWidget);
                QSpinBox* iSpinBox
                        = qobject_cast<QSpinBox*>(treeWidget->itemWidget(childItem, 1));
                QDoubleSpinBox* dSpinBox
                        = qobject_cast<QDoubleSpinBox*>(treeWidget->itemWidget(childItem, 1));
                Q_ASSERT(iSpinBox || dSpinBox);

                childItem->setData(0, Qt::DecorationRole, ParserUtils::exists(control->dir(), "z"));
                if (dSpinBox) {
                    dSpinBox->blockSignals(true);
                    dSpinBox->setValue(control->zValue());
                    dSpinBox->blockSignals(false);
                } else {
                    iSpinBox->blockSignals(true);
                    iSpinBox->setValue(control->zValue());
                    iSpinBox->blockSignals(false);
                }
                break;
            }
        }
    }
}

void PropertiesController::onControlRenderInfoChange(Control* control, bool codeChanged)
{
    if (m_designerScene->selectedControls().size() != 1)
        return;

    Control* selectedControl = m_designerScene->selectedControls().first();
    if (selectedControl != control)
        return;

    if (topLevelItemCount() <= 0)
        return onSceneSelectionChange();

    if (codeChanged)
        return onSceneSelectionChange();
    else
        return onControlGeometryChange(control);
}

void PropertiesController::onControlGeometryChange(const Control* control)
{
    if (topLevelItemCount() <= 0)
        return;

    if (m_designerScene->selectedControls().size() != 1)
        return;

    Control* selectedControl = m_designerScene->selectedControls().first();
    if (selectedControl != control)
        return;

    const QRectF& geometry = control->geometry();

    bool xUnknown = false, yUnknown = false;
    if (control->type() == Form::Type) {
        xUnknown = !ParserUtils::exists(control->dir(), "x");
        yUnknown = !ParserUtils::exists(control->dir(), "y");
    }

    const QString& geometryText = QString::fromUtf8("[(%1, %2), %3 x %4]")
            .arg(xUnknown ? "?" : QString::number(int(geometry.x())))
            .arg(yUnknown ? "?" : QString::number(int(geometry.y())))
            .arg(int(geometry.width()))
            .arg(int(geometry.height()));

    const bool xChanged = ParserUtils::exists(control->dir(), "x");
    const bool yChanged = ParserUtils::exists(control->dir(), "y");
    const bool wChanged = ParserUtils::exists(control->dir(), "width");
    const bool hChanged = ParserUtils::exists(control->dir(), "height");
    const bool geometryChanged = xChanged || yChanged || wChanged || hChanged;

    for (QTreeWidgetItem* topLevelItem : topLevelItems(this)) {
        for (QTreeWidgetItem* childItem : allSubChildItems(topLevelItem)) {
            if (childItem->text(0) == "geometry") {
                childItem->setText(1, geometryText);
                childItem->setData(0, Qt::DecorationRole, geometryChanged);
            }
            if (!isGeometryProperty(childItem->text(0)))
                continue;

            QTreeWidget* treeWidget = childItem->treeWidget();
            Q_ASSERT(treeWidget);
            QSpinBox* iSpinBox
                    = qobject_cast<QSpinBox*>(treeWidget->itemWidget(childItem, 1));
            QDoubleSpinBox* dSpinBox
                    = qobject_cast<QDoubleSpinBox*>(treeWidget->itemWidget(childItem, 1));
            Q_ASSERT(iSpinBox || dSpinBox);

            if (dSpinBox)
                dSpinBox->blockSignals(true);
            else
                iSpinBox->blockSignals(true);

            if (childItem->text(0) == "x") {
                childItem->setData(0, Qt::DecorationRole, xChanged);
                if (dSpinBox) {
                    dSpinBox->setValue(geometry.x());
                    fixPosForForm(control, "x", dSpinBox);
                } else {
                    iSpinBox->setValue(geometry.x());
                    fixPosForForm(control, "x", iSpinBox);
                }
            } else if (childItem->text(0) == "y") {
                childItem->setData(0, Qt::DecorationRole, yChanged);
                if (dSpinBox) {
                    dSpinBox->setValue(geometry.y());
                    fixPosForForm(control, "y", dSpinBox);
                } else {
                    iSpinBox->setValue(geometry.y());
                    fixPosForForm(control, "y", iSpinBox);
                }
            } else if (childItem->text(0) == "width") {
                childItem->setData(0, Qt::DecorationRole, wChanged);
                if (dSpinBox)
                    dSpinBox->setValue(control->width());
                else
                    iSpinBox->setValue(control->width());
            } else if (childItem->text(0) == "height") {
                childItem->setData(0, Qt::DecorationRole, hChanged);
                if (dSpinBox)
                    dSpinBox->setValue(control->height());
                else
                    iSpinBox->setValue(control->height());
            }

            if (dSpinBox)
                dSpinBox->blockSignals(false);
            else
                iSpinBox->blockSignals(false);
        }
    }
}

void PropertiesController::onControlIndexChange(Control* control)
{
    if (topLevelItemCount() <= 0)
        return;

    if (m_designerScene->selectedControls().size() != 1)
        return;

    QList<Control*> affectedControls({control});
    affectedControls.append(control->siblings());

    Control* selectedControl = m_designerScene->selectedControls().first();
    if (!affectedControls.contains(selectedControl))
        return;

    Control* issuedControl = nullptr;
    for (QTreeWidgetItem* topLevelItem : topLevelItems(this)) {
        for (QTreeWidgetItem* childItem : allSubChildItems(topLevelItem)) {
            if (childItem->text(0) == "uid") {
                const QString& uid = childItem->text(1);
                for (Control* ctrl : affectedControls) {
                    if (ctrl->uid() == uid)
                        issuedControl = ctrl;
                }
            } else if (childItem->text(0) == "index") {
                Q_ASSERT(issuedControl);
                QTreeWidget* treeWidget = childItem->treeWidget();
                Q_ASSERT(treeWidget);
                QSpinBox* spinBox
                        = qobject_cast<QSpinBox*>(treeWidget->itemWidget(childItem, 1));
                Q_ASSERT(spinBox);
                spinBox->setValue(control->index());
                break;
            }
        }
    }
}

void PropertiesController::onControlIdChange(Control* control, const QString& /*previousId*/)
{
    if (topLevelItemCount() <= 0)
        return;

    if (m_designerScene->selectedControls().size() != 1)
        return;

    Control* selectedControl = m_designerScene->selectedControls().first();
    if (selectedControl != control)
        return;

    for (QTreeWidgetItem* topLevelItem : topLevelItems(this)) {
        for (QTreeWidgetItem* childItem : allSubChildItems(topLevelItem)) {
            if (childItem->text(0) == "id") {
                QTreeWidget* treeWidget = childItem->treeWidget();
                Q_ASSERT(treeWidget);
                QLineEdit* lineEdit
                        = qobject_cast<QLineEdit*>(treeWidget->itemWidget(childItem, 1));
                Q_ASSERT(lineEdit);
                lineEdit->setText(control->id());
                break;
            }
        }
    }
}

void PropertiesController::onControlPropertyChange()
{
    Q_UNUSED(0)
    /*
        FIXME: Empty for now, because the only user of the setProperty function of
               ControlPropertyManager is this class. Hence no need to handle property
               changes which made by us already.
    */
}

void PropertiesController::onControlIdEditingFinish()
{
    if (control->id() != m_idEdit->text()) {
        if (m_idEdit->text().isEmpty()) {
            m_idEdit->setText(control->id());
        } else {
            ControlPropertyManager::setId(control, m_idEdit->text(),
                                          ControlPropertyManager::SaveChanges
                                          | ControlPropertyManager::UpdateRenderer);
        }
    }
}

void PropertiesController::onControlIndexEditingFinish()
{
    // NOTE: No need for previous value equality check, since this signal is only emitted
    // when the value is changed
    ControlPropertyManager::Options options =
            ControlPropertyManager::SaveChanges | ControlPropertyManager::UpdateRenderer;

    ControlPropertyManager::setIndex(control, spinBox->value(), options);
}
