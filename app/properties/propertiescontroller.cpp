#include <propertiescontroller.h>
#include <propertiespane.h>
#include <propertiestree.h>
#include <propertiesdelegate.h>
#include <lineedit.h>
#include <designerscene.h>
#include <form.h>
#include <controlpropertymanager.h>
#include <saveutils.h>
#include <parserutils.h>
#include <paintutils.h>
#include <utilityfunctions.h>

#include <QTimer>
#include <QToolButton>
#include <QColorDialog>
#include <QSpinBox>
#include <QMetaEnum>
#include <QScrollBar>

// FIXME: Fix all QTimer usages

static bool isGeometryProperty(const QString& propertyName)
{
    return propertyName == QStringLiteral("x")
            || propertyName == QStringLiteral("y")
            || propertyName == QStringLiteral("width")
            || propertyName == QStringLiteral("height");
}

static bool isFontProperty(const QString& propertyName)
{
    return propertyName == QStringLiteral("font.family")
            || propertyName == QStringLiteral("font.bold")
            || propertyName == QStringLiteral("font.italic")
            || propertyName == QStringLiteral("font.underline")
            || propertyName == QStringLiteral("font.pointSize")
            || propertyName == QStringLiteral("font.pixelSize")
            || propertyName == QStringLiteral("font.weight")
            || propertyName == QStringLiteral("font.overline")
            || propertyName == QStringLiteral("font.strikeout")
            || propertyName == QStringLiteral("font.capitalization")
            || propertyName == QStringLiteral("font.kerning")
            || propertyName == QStringLiteral("font.preferShaping");
}

static QString cleanUrl(const QUrl& url, const QString& controlDir)
{
    QString displayText = url.toDisplayString();
    if (url.isLocalFile()) {
        displayText = url.toLocalFile().remove(
                    SaveUtils::toControlThisDir(controlDir) + '/');
    }
    return displayText;
}

PropertiesController::PropertiesController(PropertiesPane* propertiesPane, DesignerScene* designerScene,
                                           QObject* parent) : QObject(parent)
  , m_propertiesPane(propertiesPane)
  , m_designerScene(designerScene)
  , m_verticalScrollPosition(9999)
  , m_horizontalScrollPosition(9999)
  , m_fontItemOpen(false)
  , m_geometryItemOpen(false)
  , m_isExpandCollapseSignalsBlocked(false)
{
    PropertiesTree* tree = m_propertiesPane->propertiesTree();
    tree->setDesignerScene(designerScene);
    tree->viewport()->installEventFilter(this);

    connect(m_propertiesPane->resetButton(), &QToolButton::clicked,
            this, &PropertiesController::onResetButtonClick);
    connect(m_propertiesPane->idEdit(), &QLineEdit::editingFinished,
            this, &PropertiesController::onControlIdEditingFinish);
    connect(m_propertiesPane->indexEdit(), qOverload<int>(&QSpinBox::valueChanged),
            this, &PropertiesController::onControlIndexValueChange);
    connect(m_propertiesPane->searchEdit(), &LineEdit::editingFinished,
            this, &PropertiesController::onSearchEditEditingFinish);
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
    connect(tree, &PropertiesTree::itemExpanded, this, [=] (QTreeWidgetItem* item) {
        if (!m_isExpandCollapseSignalsBlocked) {
            if (item->text(0) == QStringLiteral("font"))
                m_fontItemOpen = true;
            else if (item->text(0) == QStringLiteral("geometry"))
                m_geometryItemOpen = true;
        }
    });
    connect(tree, &PropertiesTree::itemCollapsed, this, [=] (QTreeWidgetItem* item) {
        if (!m_isExpandCollapseSignalsBlocked) {
            if (item->text(0) == QStringLiteral("font"))
                m_fontItemOpen = false;
            else if (item->text(0) == QStringLiteral("geometry"))
                m_geometryItemOpen = false;
        }
    });
    connect(tree->verticalScrollBar(), &QScrollBar::valueChanged, [=] (int value)
    {
        if (tree->topLevelItemCount() > m_propertiesPane->permanentItemCount()) {
            m_verticalScrollPosition = tree->verticalScrollBar()->maximum()
                    - tree->verticalScrollBar()->minimum() - value;
        }
    });
    connect(tree->horizontalScrollBar(), &QScrollBar::valueChanged, [=] (int value)
    {
        if (tree->topLevelItemCount() > m_propertiesPane->permanentItemCount()) {
            m_horizontalScrollPosition = tree->horizontalScrollBar()->maximum()
                    - tree->horizontalScrollBar()->minimum() - value;
        }
    });
    connect(tree->verticalScrollBar(), &QScrollBar::rangeChanged, [=] (int min, int max)
    {
        if (tree->topLevelItemCount() > m_propertiesPane->permanentItemCount())
            m_verticalScrollPosition = max - min - tree->verticalScrollBar()->value();
    });
    connect(tree->horizontalScrollBar(), &QScrollBar::rangeChanged, [=] (int min, int max)
    {
        if (tree->topLevelItemCount() > m_propertiesPane->permanentItemCount())
            m_horizontalScrollPosition = max - min - tree->horizontalScrollBar()->value();
    });
}

void PropertiesController::discharge()
{
    clear();
    m_propertiesPane->searchEdit()->clear();
    m_verticalScrollPosition = 9999;
    m_horizontalScrollPosition = 9999;
    m_fontItemOpen = false;
    m_geometryItemOpen = false;
}

void PropertiesController::clear() const
{
    PropertiesTree* tree = m_propertiesPane->propertiesTree();

    m_propertiesPane->typeItem()->setHidden(true);
    m_propertiesPane->uidItem()->setHidden(true);
    m_propertiesPane->idItem()->setHidden(true);
    m_propertiesPane->indexItem()->setHidden(true);

    for (QTreeWidgetItem* topLevelItem : tree->topLevelItems()) {
        if (m_propertiesPane->isPermanentItem(topLevelItem))
            continue;
        for (QTreeWidgetItem* childItem : tree->allSubChildItems(topLevelItem, true, true, true))
            tree->delegate()->destroyItem(childItem);
    }
}

void PropertiesController::onResetButtonClick() const
{
    PropertiesTree* tree = m_propertiesPane->propertiesTree();

    if (tree->topLevelItemCount() <= m_propertiesPane->permanentItemCount())
        return;

    if (Control* selectedControl = this->control()) {
        QTreeWidgetItem* item = tree->itemAt(m_propertiesPane->resetButton()->geometry().center());
        Q_ASSERT(item);
        if (item) {
            const QString& propertyName = item->text(0);
            m_propertiesPane->resetButton()->setVisible(false);
            item->setData(0, PropertiesDelegate::ModificationRole, false);

            if (propertyName == QStringLiteral("z")) {
                // Only to emit zChanged signal of the ControlPropertyManager
                ControlPropertyManager::setZ(selectedControl, 0, ControlPropertyManager::DontApplyDesigner);
                ControlPropertyManager::setBinding(selectedControl, QStringLiteral("z"), QString(),
                                                   ControlPropertyManager::UpdateRenderer |
                                                   ControlPropertyManager::SaveChanges);
                QTimer::singleShot(200, [=] { onControlZChange(selectedControl); });
            }
            // NOTE: geometryItem (parent item) is going to be handled, because
            // onControlGeometryChange is going to be called due to control
            // syncGeometry stuff. Also no worries for geometryChanged signal of
            // the ControlPropertyManager since it is also, naturally, will be  called
            else if (isGeometryProperty(propertyName)) {
                ControlPropertyManager::setBinding(selectedControl, propertyName, QString(),
                                                   ControlPropertyManager::UpdateRenderer |
                                                   ControlPropertyManager::SaveChanges);
            } else if (propertyName == QStringLiteral("geometry")) {
                ControlPropertyManager::setBinding(selectedControl, QStringLiteral("x"), QString(),
                                                   ControlPropertyManager::UpdateRenderer |
                                                   ControlPropertyManager::SaveChanges);
                ControlPropertyManager::setBinding(selectedControl, QStringLiteral("y"), QString(),
                                                   ControlPropertyManager::UpdateRenderer |
                                                   ControlPropertyManager::SaveChanges);
                ControlPropertyManager::setBinding(selectedControl, QStringLiteral("width"), QString(),
                                                   ControlPropertyManager::UpdateRenderer |
                                                   ControlPropertyManager::SaveChanges);
                ControlPropertyManager::setBinding(selectedControl, QStringLiteral("height"), QString(),
                                                   ControlPropertyManager::UpdateRenderer |
                                                   ControlPropertyManager::SaveChanges);
            } else if (isFontProperty(QStringLiteral("font.") + propertyName)) {
                ControlPropertyManager::setBinding(selectedControl, QStringLiteral("font.") + propertyName, QString(),
                                                   ControlPropertyManager::UpdateRenderer |
                                                   ControlPropertyManager::SaveChanges);
                if (auto classItem = item->parent()) {
                    bool clearModificationFlag = true;
                    for (int i = 0; i < classItem->childCount(); ++i) {
                        auto child = classItem->child(i);
                        if (child != item && child->data(0, PropertiesDelegate::ModificationRole).toBool())
                            clearModificationFlag = false;
                    }
                    if (clearModificationFlag)
                        classItem->setData(0, PropertiesDelegate::ModificationRole, false);
                }
                QTimer::singleShot(200, this, [=] {
                    if (auto w = tree->itemWidget(item, 1)) {
                        auto font = selectedControl->itemProperty(QStringLiteral("font")).value<QFont>();
                        const QMetaEnum& e = QMetaEnum::fromType<QFont::Weight>();
                        const QMetaEnum& e2 = QMetaEnum::fromType<QFont::Capitalization>();
                        w->blockSignals(true);
                        if (propertyName == QStringLiteral("family"))
                            w->setProperty("currentText", font.family());
                        else if (propertyName == QStringLiteral("bold"))
                            w->setProperty("checked", font.bold());
                        else if (propertyName == QStringLiteral("italic"))
                            w->setProperty("checked", font.italic());
                        else if (propertyName == QStringLiteral("underline"))
                            w->setProperty("checked", font.underline());
                        else if (propertyName == QStringLiteral("pointSize"))
                            w->setProperty("value", font.pointSize());
                        else if (propertyName == QStringLiteral("pixelSize"))
                            w->setProperty("value", font.pixelSize());
                        else if (propertyName == QStringLiteral("weight"))
                            w->setProperty("currentText", e.valueToKey(font.weight()));
                        else if (propertyName == QStringLiteral("overline"))
                            w->setProperty("checked", font.overline());
                        else if (propertyName == QStringLiteral("strikeout"))
                            w->setProperty("checked", font.strikeOut());
                        else if (propertyName == QStringLiteral("capitalization"))
                            w->setProperty("currentText", e2.valueToKey(font.capitalization()));
                        else if (propertyName == QStringLiteral("kerning"))
                            w->setProperty("checked", font.kerning());
                        else if (propertyName == QStringLiteral("preferShaping"))
                            w->setProperty("checked", !(font.styleStrategy() & QFont::PreferNoShaping));
                        w->blockSignals(false);
                    }
                });
            } else if (propertyName == QStringLiteral("font")) {
                ControlPropertyManager::setBinding(selectedControl, QStringLiteral("font.family"), QString(),
                                                   ControlPropertyManager::UpdateRenderer |
                                                   ControlPropertyManager::SaveChanges);
                ControlPropertyManager::setBinding(selectedControl, QStringLiteral("font.bold"), QString(),
                                                   ControlPropertyManager::UpdateRenderer |
                                                   ControlPropertyManager::SaveChanges);
                ControlPropertyManager::setBinding(selectedControl, QStringLiteral("font.italic"), QString(),
                                                   ControlPropertyManager::UpdateRenderer |
                                                   ControlPropertyManager::SaveChanges);
                ControlPropertyManager::setBinding(selectedControl, QStringLiteral("font.underline"), QString(),
                                                   ControlPropertyManager::UpdateRenderer |
                                                   ControlPropertyManager::SaveChanges);
                ControlPropertyManager::setBinding(selectedControl, QStringLiteral("font.pointSize"), QString(),
                                                   ControlPropertyManager::UpdateRenderer |
                                                   ControlPropertyManager::SaveChanges);
                ControlPropertyManager::setBinding(selectedControl, QStringLiteral("font.pixelSize"), QString(),
                                                   ControlPropertyManager::UpdateRenderer |
                                                   ControlPropertyManager::SaveChanges);
                ControlPropertyManager::setBinding(selectedControl, QStringLiteral("font.weight"), QString(),
                                                   ControlPropertyManager::UpdateRenderer |
                                                   ControlPropertyManager::SaveChanges);
                ControlPropertyManager::setBinding(selectedControl, QStringLiteral("font.overline"), QString(),
                                                   ControlPropertyManager::UpdateRenderer |
                                                   ControlPropertyManager::SaveChanges);
                ControlPropertyManager::setBinding(selectedControl, QStringLiteral("font.strikeout"), QString(),
                                                   ControlPropertyManager::UpdateRenderer |
                                                   ControlPropertyManager::SaveChanges);
                ControlPropertyManager::setBinding(selectedControl, QStringLiteral("font.capitalization"), QString(),
                                                   ControlPropertyManager::UpdateRenderer |
                                                   ControlPropertyManager::SaveChanges);
                ControlPropertyManager::setBinding(selectedControl, QStringLiteral("font.kerning"), QString(),
                                                   ControlPropertyManager::UpdateRenderer |
                                                   ControlPropertyManager::SaveChanges);
                ControlPropertyManager::setBinding(selectedControl, QStringLiteral("font.preferShaping"), QString(),
                                                   ControlPropertyManager::UpdateRenderer |
                                                   ControlPropertyManager::SaveChanges);
                for (int i = 0; i < item->childCount(); ++i)
                    item->child(i)->setData(0, PropertiesDelegate::ModificationRole, false);
                QTimer::singleShot(600, this, [=] {
                    for (int i = 0; i < item->childCount(); ++i) {
                        auto child = item->child(i);
                        if (auto w = tree->itemWidget(child, 1)) {
                            auto propertyName = child->text(0);
                            auto font = selectedControl->itemProperty(QStringLiteral("font")).value<QFont>();
                            const QMetaEnum& e = QMetaEnum::fromType<QFont::Weight>();
                            const QMetaEnum& e2 = QMetaEnum::fromType<QFont::Capitalization>();
                            w->blockSignals(true);
                            if (propertyName == QStringLiteral("family"))
                                w->setProperty("currentText", font.family());
                            else if (propertyName == QStringLiteral("bold"))
                                w->setProperty("checked", font.bold());
                            else if (propertyName == QStringLiteral("italic"))
                                w->setProperty("checked", font.italic());
                            else if (propertyName == QStringLiteral("underline"))
                                w->setProperty("checked", font.underline());
                            else if (propertyName == QStringLiteral("pointSize"))
                                w->setProperty("value", font.pointSize());
                            else if (propertyName == QStringLiteral("pixelSize"))
                                w->setProperty("value", font.pixelSize());
                            else if (propertyName == QStringLiteral("weight"))
                                w->setProperty("currentText", e.valueToKey(font.weight()));
                            else if (propertyName == QStringLiteral("overline"))
                                w->setProperty("checked", font.overline());
                            else if (propertyName == QStringLiteral("strikeout"))
                                w->setProperty("checked", font.strikeOut());
                            else if (propertyName == QStringLiteral("capitalization"))
                                w->setProperty("currentText", e2.valueToKey(font.capitalization()));
                            else if (propertyName == QStringLiteral("kerning"))
                                w->setProperty("checked", font.kerning());
                            else if (propertyName == QStringLiteral("preferShaping"))
                                w->setProperty("checked", !(font.styleStrategy() & QFont::PreferNoShaping));
                            w->blockSignals(false);
                        }
                    }
                });
            } else {
                ControlPropertyManager::setBinding(selectedControl, propertyName, QString(),
                                                   ControlPropertyManager::UpdateRenderer |
                                                   ControlPropertyManager::SaveChanges);
                QTimer::singleShot(200, this, [=] {
                    if (auto w = tree->itemWidget(item, 1)) {
                        auto val = selectedControl->itemProperty(propertyName);
                        auto type = item->data(1, PropertiesDelegate::TypeRole).value<PropertiesDelegate::Type>();
                        if (type == PropertiesDelegate::Enum) {
                            auto e = val.value<Enum>();
                            tree->delegate()->setInitialValue(w, type, e.value);
                        } else {
                            tree->delegate()->setInitialValue(w, type, val);
                        }
                    }
                });
            }
        }
    }
}

void PropertiesController::onSearchEditEditingFinish() const
{
    PropertiesTree* tree = m_propertiesPane->propertiesTree();
    const QList<QTreeWidgetItem*>& topLevelItems = tree->topLevelItems();
    const QString& searchTerm = m_propertiesPane->searchEdit()->text();
    for (QTreeWidgetItem* topLevelItem : topLevelItems) {
        auto tlv = false;
        for (int j = 0; j < topLevelItem->childCount(); j++) {
            auto tci = topLevelItem->child(j);
            auto tcv = false;
            auto vv = tci->text(0).contains(searchTerm, Qt::CaseInsensitive);

            for (int z = 0; z < tci->childCount(); z++) {
                auto tdi = tci->child(z);
                auto v = (searchTerm.isEmpty() || vv)
                        ? true
                        : tdi->text(0).contains(searchTerm, Qt::CaseInsensitive);

                tdi->setHidden(!v);
                if (v)
                    tcv = v;
            }

            auto v = searchTerm.isEmpty() ? true : (tci->childCount() > 0 ? tcv : vv);
            tci->setHidden(!v);
            if (v)
                tlv = v;
        }

        auto v = searchTerm.isEmpty() ? true : tlv;
        topLevelItem->setHidden(!v);
    }
}

void PropertiesController::onControlZChange(Control* control) const
{
    PropertiesTree* tree = m_propertiesPane->propertiesTree();

    if (tree->topLevelItemCount() <= m_propertiesPane->permanentItemCount())
        return;

    if (Control* selectedControl = this->control()) {
        if (selectedControl != control)
            return;
        for (QTreeWidgetItem* topLevelItem : tree->topLevelItems()) {
            for (QTreeWidgetItem* childItem : tree->allSubChildItems(topLevelItem)) {
                if (childItem->text(0) == QStringLiteral("z")) {
                    QTreeWidget* treeWidget = childItem->treeWidget();
                    Q_ASSERT(treeWidget);
                    QSpinBox* iSpinBox
                            = qobject_cast<QSpinBox*>(treeWidget->itemWidget(childItem, 1));
                    QDoubleSpinBox* dSpinBox
                            = qobject_cast<QDoubleSpinBox*>(treeWidget->itemWidget(childItem, 1));
                    Q_ASSERT(iSpinBox || dSpinBox);

                    childItem->setData(0, PropertiesDelegate::ModificationRole,
                                       ParserUtils::exists(control->dir(), QStringLiteral("z")));
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
}

void PropertiesController::onControlRenderInfoChange(Control* control, bool codeChanged) const
{
    PropertiesTree* tree = m_propertiesPane->propertiesTree();
    if (Control* selectedControl = this->control()) {
        if (selectedControl != control)
            return;
        if (tree->topLevelItemCount() <= m_propertiesPane->permanentItemCount())
            return onSceneSelectionChange();
        if (codeChanged)
            return onSceneSelectionChange();
        else
            return onControlGeometryChange(control);
    }
}

void PropertiesController::onControlGeometryChange(const Control* control) const
{
    PropertiesTree* tree = m_propertiesPane->propertiesTree();

    if (tree->topLevelItemCount() <= m_propertiesPane->permanentItemCount())
        return;

    if (Control* selectedControl = this->control()) {
        if (selectedControl != control)
            return;

        const QRectF& geometry = control->geometry();

        bool xUnknown = false, yUnknown = false;
        if (control->type() == Form::Type) {
            xUnknown = !ParserUtils::exists(control->dir(), QStringLiteral("x"));
            yUnknown = !ParserUtils::exists(control->dir(), QStringLiteral("y"));
        }

        const QString& geometryText = QStringLiteral("[(%1, %2), %3 x %4]")
                .arg(xUnknown ? QStringLiteral("?") : QString::number(int(geometry.x())))
                .arg(yUnknown ? QStringLiteral("?") : QString::number(int(geometry.y())))
                .arg(int(geometry.width()))
                .arg(int(geometry.height()));

        const bool xChanged = ParserUtils::exists(control->dir(), QStringLiteral("x"));
        const bool yChanged = ParserUtils::exists(control->dir(), QStringLiteral("y"));
        const bool wChanged = ParserUtils::exists(control->dir(), QStringLiteral("width"));
        const bool hChanged = ParserUtils::exists(control->dir(), QStringLiteral("height"));
        const bool geometryChanged = xChanged || yChanged || wChanged || hChanged;

        for (QTreeWidgetItem* topLevelItem : tree->topLevelItems()) {
            for (QTreeWidgetItem* childItem : tree->allSubChildItems(topLevelItem)) {
                if (childItem->text(0) == QStringLiteral("geometry")) {
                    childItem->setText(1, geometryText);
                    childItem->setData(0, PropertiesDelegate::ModificationRole, geometryChanged);
                }
                if (!isGeometryProperty(childItem->text(0)))
                    continue;

                QSpinBox* iSpinBox
                        = qobject_cast<QSpinBox*>(tree->itemWidget(childItem, 1));
                QDoubleSpinBox* dSpinBox
                        = qobject_cast<QDoubleSpinBox*>(tree->itemWidget(childItem, 1));
                Q_ASSERT(iSpinBox || dSpinBox);

                if (dSpinBox)
                    dSpinBox->blockSignals(true);
                else
                    iSpinBox->blockSignals(true);

                if (childItem->text(0) == QStringLiteral("x")) {
                    childItem->setData(0, PropertiesDelegate::ModificationRole, xChanged);
                    qreal value = geometry.x();
                    if (control->type() == Form::Type && !ParserUtils::exists(control->dir(), QStringLiteral("x")))
                        value = 0;
                    if (dSpinBox)
                        dSpinBox->setValue(value);
                    else
                        iSpinBox->setValue(value);
                } else if (childItem->text(0) == QStringLiteral("y")) {
                    childItem->setData(0, PropertiesDelegate::ModificationRole, yChanged);
                    qreal value = geometry.y();
                    if (control->type() == Form::Type && !ParserUtils::exists(control->dir(), QStringLiteral("y")))
                        value = 0;
                    if (dSpinBox)
                        dSpinBox->setValue(value);
                    else
                        iSpinBox->setValue(value);
                } else if (childItem->text(0) == QStringLiteral("width")) {
                    childItem->setData(0, PropertiesDelegate::ModificationRole, wChanged);
                    if (dSpinBox)
                        dSpinBox->setValue(control->width());
                    else
                        iSpinBox->setValue(control->width());
                } else if (childItem->text(0) == QStringLiteral("height")) {
                    childItem->setData(0, PropertiesDelegate::ModificationRole, hChanged);
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
}

void PropertiesController::onControlIndexChange(Control* control) const
{
    PropertiesTree* tree = m_propertiesPane->propertiesTree();

    if (tree->topLevelItemCount() <= m_propertiesPane->permanentItemCount())
        return;

    if (Control* selectedControl = this->control()) {

        QList<Control*> affectedControls({control});
        affectedControls.append(control->siblings());
        if (!affectedControls.contains(selectedControl))
            return;

        Control* issuedControl = nullptr;
        for (QTreeWidgetItem* topLevelItem : tree->topLevelItems()) {
            for (QTreeWidgetItem* childItem : tree->allSubChildItems(topLevelItem)) {
                if (childItem->text(0) == QStringLiteral("uid")) {
                    const QString& uid = childItem->text(1);
                    for (Control* ctrl : affectedControls) {
                        if (ctrl->uid() == uid)
                            issuedControl = ctrl;
                    }
                } else if (childItem->text(0) == QStringLiteral("index")) {
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
}

void PropertiesController::onControlIdChange(Control* control, const QString& /*previousId*/) const
{
    PropertiesTree* tree = m_propertiesPane->propertiesTree();

    if (tree->topLevelItemCount() <= m_propertiesPane->permanentItemCount())
        return;

    if (Control* selectedControl = this->control()) {
        if (selectedControl != control)
            return;

        for (QTreeWidgetItem* topLevelItem : tree->topLevelItems()) {
            for (QTreeWidgetItem* childItem : tree->allSubChildItems(topLevelItem)) {
                if (childItem->text(0) == QStringLiteral("id")) {
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
}

void PropertiesController::onControlPropertyChange() const
{
    Q_UNUSED(0)
    /*
        FIXME: Empty for now, because the only user of the setProperty function of
               ControlPropertyManager is this class. Hence no need to handle property
               changes which made by us already.
    */
}

void PropertiesController::onControlIdEditingFinish() const
{
    if (Control* selectedControl = control()) {
        if (selectedControl->id() != m_propertiesPane->idEdit()->text()) {
            if (m_propertiesPane->idEdit()->text().isEmpty()) {
                m_propertiesPane->idEdit()->setText(selectedControl->id());
            } else {
                ControlPropertyManager::setId(selectedControl, m_propertiesPane->idEdit()->text(),
                                              ControlPropertyManager::SaveChanges |
                                              ControlPropertyManager::UpdateRenderer);
            }
        }
    }
}

void PropertiesController::onControlIndexValueChange() const
{
    // NOTE: No need for previous value equality check, since this signal is only emitted
    // when the value is changed
    ControlPropertyManager::setIndex(control(), m_propertiesPane->indexEdit()->value(),
                                     ControlPropertyManager::SaveChanges |
                                     ControlPropertyManager::UpdateRenderer);
}

void PropertiesController::onSceneSelectionChange() const
{
    clear();

    PropertiesTree* tree = m_propertiesPane->propertiesTree();
    const int verticalScrollPosition = m_verticalScrollPosition;
    const int horizontalScrollPosition = m_horizontalScrollPosition;

    if (Control* selectedControl = control()) {
        m_propertiesPane->setDisabled(selectedControl->hasErrors());
        const QVector<PropertyNode>& properties = selectedControl->properties();

        if (properties.isEmpty())
            return;

        m_propertiesPane->typeItem()->setText(1, properties.first().cleanClassName);
        m_propertiesPane->uidItem()->setText(1, selectedControl->uid());
        m_propertiesPane->idEdit()->setText(selectedControl->id());
        m_propertiesPane->indexEdit()->blockSignals(true);
        m_propertiesPane->indexEdit()->setValue(selectedControl->index());
        m_propertiesPane->indexEdit()->blockSignals(false);

        m_propertiesPane->typeItem()->setHidden(false);
        m_propertiesPane->uidItem()->setHidden(false);
        m_propertiesPane->idItem()->setHidden(false);
        m_propertiesPane->indexItem()->setHidden(false);

        bool isGeometryHandled = false;
        QList<QTreeWidgetItem*> classItems;
        QTreeWidgetItem* fontItemCopy = nullptr;
        QTreeWidgetItem* geometryItemCopy = nullptr;
        for (const PropertyNode& propertyNode : properties) {
            const QVector<Enum>& enumList = propertyNode.enums;
            const QMap<QString, QVariant>& propertyMap = propertyNode.properties;

            if (propertyMap.isEmpty() && enumList.isEmpty())
                continue;

            QList<QTreeWidgetItem*> children;
            QList<QTreeWidgetItem*> fontChildren;
            QList<QTreeWidgetItem*> geometryChildren;

            QTreeWidgetItem* classItem = nullptr;
            QTreeWidgetItem* fontItem = nullptr;
            QTreeWidgetItem* geometryItem = nullptr;

            classItem = tree->delegate()->createItem();
            classItem->setText(0, propertyNode.cleanClassName);

            const QList<QString>& propertyKeys = propertyMap.keys();
            for (const QString& propertyName : propertyKeys) {
                const QVariant& propertyValue = propertyMap.value(propertyName);
                switch (propertyValue.type()) {
                case QVariant::Font: {
                    const QFont& font = propertyValue.value<QFont>();
                    const bool isPx = font.pixelSize() > 0 ? true : false;
                    const QString& family = QFontInfo(font).family();
                    const QString& fontText = QStringLiteral("[%1, %2%3]")
                            .arg(family)
                            .arg(isPx ? font.pixelSize() : font.pointSize())
                            .arg(isPx ? QStringLiteral("px") : QStringLiteral("pt"));

                    const bool fChanged    = ParserUtils::exists(selectedControl->dir(), QStringLiteral("font.family"));
                    const bool bChanged    = ParserUtils::exists(selectedControl->dir(), QStringLiteral("font.bold"));
                    const bool iChanged    = ParserUtils::exists(selectedControl->dir(), QStringLiteral("font.italic"));
                    const bool uChanged    = ParserUtils::exists(selectedControl->dir(), QStringLiteral("font.underline"));
                    const bool ptChanged   = ParserUtils::exists(selectedControl->dir(), QStringLiteral("font.pointSize"));
                    const bool piChanged   = ParserUtils::exists(selectedControl->dir(), QStringLiteral("font.pixelSize"));
                    const bool wChanged    = ParserUtils::exists(selectedControl->dir(), QStringLiteral("font.weight"));
                    const bool oChanged    = ParserUtils::exists(selectedControl->dir(), QStringLiteral("font.overline"));
                    const bool sChanged    = ParserUtils::exists(selectedControl->dir(), QStringLiteral("font.strikeout"));
                    const bool cChanged    = ParserUtils::exists(selectedControl->dir(), QStringLiteral("font.capitalization"));
                    const bool kChanged    = ParserUtils::exists(selectedControl->dir(), QStringLiteral("font.kerning"));
                    const bool prChanged   = ParserUtils::exists(selectedControl->dir(), QStringLiteral("font.preferShaping"));
                    const bool fontChanged = fChanged || bChanged || iChanged || uChanged || ptChanged || piChanged
                            || wChanged || oChanged || sChanged || cChanged || kChanged || prChanged;

                    fontItem = tree->delegate()->createItem();
                    fontItem->setText(0, QStringLiteral("font"));
                    fontItem->setText(1, fontText);
                    fontItem->setData(0, PropertiesDelegate::ModificationRole, fontChanged);
                    children.append(fontItem);

                    auto fItem = tree->delegate()->createItem();
                    auto callback = PropertiesDelegate::makeCallback(&PropertiesController::onFontFamilyPropertyEdit,
                                                                     this, fItem, fontItem);
                    fItem->setText(0, QStringLiteral("family"));
                    fItem->setData(0, PropertiesDelegate::ModificationRole, fChanged);
                    fItem->setData(1, PropertiesDelegate::InitialValueRole, family);
                    fItem->setData(1, PropertiesDelegate::TypeRole, PropertiesDelegate::FontFamily);
                    fItem->setData(1, PropertiesDelegate::CallbackRole, callback.toVariant());
                    fontChildren.append(fItem);

                    const QMetaEnum& e = QMetaEnum::fromType<QFont::Weight>();
                    auto wItem = tree->delegate()->createItem();
                    callback = PropertiesDelegate::makeCallback(&PropertiesController::onFontWeightPropertyEdit,
                                                                this, wItem, fontItem);
                    wItem->setText(0, QStringLiteral("weight"));
                    wItem->setData(0, PropertiesDelegate::ModificationRole, wChanged);
                    wItem->setData(1, PropertiesDelegate::InitialValueRole, e.valueToKey(font.weight()));
                    wItem->setData(1, PropertiesDelegate::TypeRole, PropertiesDelegate::FontWeight);
                    wItem->setData(1, PropertiesDelegate::CallbackRole, callback.toVariant());
                    fontChildren.append(wItem);

                    const QMetaEnum& e2 = QMetaEnum::fromType<QFont::Capitalization>();
                    auto cItem = tree->delegate()->createItem();
                    callback = PropertiesDelegate::makeCallback(&PropertiesController::onFontCapitalizationPropertyEdit,
                                                                this, cItem, fontItem);
                    cItem->setText(0, QStringLiteral("capitalization"));
                    cItem->setData(0, PropertiesDelegate::ModificationRole, cChanged);
                    cItem->setData(1, PropertiesDelegate::InitialValueRole, e2.valueToKey(font.capitalization()));
                    cItem->setData(1, PropertiesDelegate::TypeRole, PropertiesDelegate::FontCapitalization);
                    cItem->setData(1, PropertiesDelegate::CallbackRole, callback.toVariant());
                    fontChildren.append(cItem);

                    auto ptItem = tree->delegate()->createItem();
                    auto pxItem = tree->delegate()->createItem();
                    callback = PropertiesDelegate::makeCallback(&PropertiesController::onFontSizePropertyEdit,
                                                                this, ptItem, pxItem, fontItem, QStringLiteral("font.pointSize"));
                    ptItem->setText(0, QStringLiteral("pointSize"));
                    ptItem->setData(0, PropertiesDelegate::ModificationRole, ptChanged);
                    ptItem->setData(1, PropertiesDelegate::InitialValueRole, font.pointSize() < 0 ? 0 : font.pointSize());
                    ptItem->setData(1, PropertiesDelegate::TypeRole, PropertiesDelegate::FontSize);
                    ptItem->setData(1, PropertiesDelegate::CallbackRole, callback.toVariant());
                    fontChildren.append(ptItem);

                    callback = PropertiesDelegate::makeCallback(&PropertiesController::onFontSizePropertyEdit,
                                                                this, pxItem, ptItem, fontItem, QStringLiteral("font.pixelSize"));
                    pxItem->setText(0, QStringLiteral("pixelSize"));
                    pxItem->setData(0, PropertiesDelegate::ModificationRole, piChanged);
                    pxItem->setData(1, PropertiesDelegate::InitialValueRole, font.pixelSize() < 0 ? 0 : font.pixelSize());
                    pxItem->setData(1, PropertiesDelegate::TypeRole, PropertiesDelegate::FontSize);
                    pxItem->setData(1, PropertiesDelegate::CallbackRole, callback.toVariant());
                    fontChildren.append(pxItem);

                    auto bItem = tree->delegate()->createItem();
                    callback = PropertiesDelegate::makeCallback(&PropertiesController::onBoolPropertyEdit,
                                                                this, bItem, fontItem, QStringLiteral("font.bold"));
                    bItem->setText(0, QStringLiteral("bold"));
                    bItem->setData(0, PropertiesDelegate::ModificationRole, bChanged);
                    bItem->setData(1, PropertiesDelegate::InitialValueRole, font.bold());
                    bItem->setData(1, PropertiesDelegate::TypeRole, PropertiesDelegate::Bool);
                    bItem->setData(1, PropertiesDelegate::CallbackRole, callback.toVariant());
                    fontChildren.append(bItem);

                    auto iItem = tree->delegate()->createItem();
                    callback = PropertiesDelegate::makeCallback(&PropertiesController::onBoolPropertyEdit,
                                                                this, iItem, fontItem, QStringLiteral("font.italic"));
                    iItem->setText(0, QStringLiteral("italic"));
                    iItem->setData(0, PropertiesDelegate::ModificationRole, iChanged);
                    iItem->setData(1, PropertiesDelegate::InitialValueRole, font.italic());
                    iItem->setData(1, PropertiesDelegate::TypeRole, PropertiesDelegate::Bool);
                    iItem->setData(1, PropertiesDelegate::CallbackRole, callback.toVariant());
                    fontChildren.append(iItem);

                    auto uItem = tree->delegate()->createItem();
                    callback = PropertiesDelegate::makeCallback(&PropertiesController::onBoolPropertyEdit,
                                                                this, uItem, fontItem, QStringLiteral("font.underline"));
                    uItem->setText(0, QStringLiteral("underline"));
                    uItem->setData(0, PropertiesDelegate::ModificationRole, uChanged);
                    uItem->setData(1, PropertiesDelegate::InitialValueRole, font.underline());
                    uItem->setData(1, PropertiesDelegate::TypeRole, PropertiesDelegate::Bool);
                    uItem->setData(1, PropertiesDelegate::CallbackRole, callback.toVariant());
                    fontChildren.append(uItem);

                    auto oItem = tree->delegate()->createItem();
                    callback = PropertiesDelegate::makeCallback(&PropertiesController::onBoolPropertyEdit,
                                                                this, oItem, fontItem, QStringLiteral("font.overline"));
                    oItem->setText(0, QStringLiteral("overline"));
                    oItem->setData(0, PropertiesDelegate::ModificationRole, oChanged);
                    oItem->setData(1, PropertiesDelegate::InitialValueRole, font.overline());
                    oItem->setData(1, PropertiesDelegate::TypeRole, PropertiesDelegate::Bool);
                    oItem->setData(1, PropertiesDelegate::CallbackRole, callback.toVariant());
                    fontChildren.append(oItem);

                    auto sItem = tree->delegate()->createItem();
                    callback = PropertiesDelegate::makeCallback(&PropertiesController::onBoolPropertyEdit,
                                                                this, sItem, fontItem, QStringLiteral("font.strikeout"));
                    sItem->setText(0, QStringLiteral("strikeout"));
                    sItem->setData(0, PropertiesDelegate::ModificationRole, sChanged);
                    sItem->setData(1, PropertiesDelegate::InitialValueRole, font.strikeOut());
                    sItem->setData(1, PropertiesDelegate::TypeRole, PropertiesDelegate::Bool);
                    sItem->setData(1, PropertiesDelegate::CallbackRole, callback.toVariant());
                    fontChildren.append(sItem);

                    auto kItem = tree->delegate()->createItem();
                    callback = PropertiesDelegate::makeCallback(&PropertiesController::onBoolPropertyEdit,
                                                                this, kItem, fontItem, QStringLiteral("font.kerning"));
                    kItem->setText(0, QStringLiteral("kerning"));
                    kItem->setData(0, PropertiesDelegate::ModificationRole, kChanged);
                    kItem->setData(1, PropertiesDelegate::InitialValueRole, font.kerning());
                    kItem->setData(1, PropertiesDelegate::TypeRole, PropertiesDelegate::Bool);
                    kItem->setData(1, PropertiesDelegate::CallbackRole, callback.toVariant());
                    fontChildren.append(kItem);

                    auto prItem = tree->delegate()->createItem();
                    callback = PropertiesDelegate::makeCallback(&PropertiesController::onBoolPropertyEdit,
                                                                this, prItem, fontItem, QStringLiteral("font.preferShaping"));
                    prItem->setText(0, QStringLiteral("preferShaping"));
                    prItem->setData(0, PropertiesDelegate::ModificationRole, prChanged);
                    prItem->setData(1, PropertiesDelegate::InitialValueRole, !(font.styleStrategy() & QFont::PreferNoShaping));
                    prItem->setData(1, PropertiesDelegate::TypeRole, PropertiesDelegate::Bool);
                    prItem->setData(1, PropertiesDelegate::CallbackRole, callback.toVariant());
                    fontChildren.append(prItem);
                    break;
                }

                case QVariant::Color: {
                    auto item = tree->delegate()->createItem();
                    auto callback = PropertiesDelegate::makeCallback(&PropertiesController::onColorPropertyEdit,
                                                                     this, item, nullptr, propertyName);
                    const QColor& color = propertyValue.value<QColor>();
                    item->setText(0, propertyName);
                    item->setData(0, PropertiesDelegate::ModificationRole, ParserUtils::exists(selectedControl->dir(), propertyName));
                    item->setData(1, PropertiesDelegate::InitialValueRole, color);
                    item->setData(1, PropertiesDelegate::TypeRole, PropertiesDelegate::Color);
                    item->setData(1, PropertiesDelegate::CallbackRole, callback.toVariant());
                    children.append(item);
                    break;
                }

                case QVariant::Bool: {
                    auto item = tree->delegate()->createItem();
                    auto callback = PropertiesDelegate::makeCallback(&PropertiesController::onBoolPropertyEdit,
                                                                     this, item, nullptr, propertyName);
                    const bool checked = propertyName == QStringLiteral("visible")
                            ? selectedControl->visible()
                            : propertyValue.value<bool>();
                    item->setText(0, propertyName);
                    item->setData(0, PropertiesDelegate::ModificationRole, ParserUtils::exists(selectedControl->dir(), propertyName));
                    item->setData(1, PropertiesDelegate::InitialValueRole, checked);
                    item->setData(1, PropertiesDelegate::TypeRole, PropertiesDelegate::Bool);
                    item->setData(1, PropertiesDelegate::CallbackRole, callback.toVariant());
                    children.append(item);
                    break;
                }

                case QVariant::String: {
                    auto item = tree->delegate()->createItem();
                    auto callback = PropertiesDelegate::makeCallback(&PropertiesController::onStringPropertyEdit,
                                                                     this, item, nullptr, propertyName);
                    const QString& text = propertyValue.value<QString>();
                    item->setText(0, propertyName);
                    item->setData(0, PropertiesDelegate::ModificationRole, ParserUtils::exists(selectedControl->dir(), propertyName));
                    item->setData(1, PropertiesDelegate::InitialValueRole, text);
                    item->setData(1, PropertiesDelegate::TypeRole, PropertiesDelegate::String);
                    item->setData(1, PropertiesDelegate::CallbackRole, callback.toVariant());
                    children.append(item);
                    break;
                }

                case QVariant::Url: {
                    auto item = tree->delegate()->createItem();
                    auto callback = PropertiesDelegate::makeCallback(&PropertiesController::onUrlPropertyEdit,
                                                                     this, item, nullptr, propertyName);
                    const QString& displayText = cleanUrl(propertyValue.value<QUrl>(), selectedControl->dir());
                    item->setText(0, propertyName);
                    item->setData(0, PropertiesDelegate::ModificationRole, ParserUtils::exists(selectedControl->dir(), propertyName));
                    item->setData(1, PropertiesDelegate::InitialValueRole, displayText);
                    item->setData(1, PropertiesDelegate::TypeRole, PropertiesDelegate::String);
                    item->setData(1, PropertiesDelegate::CallbackRole, callback.toVariant());
                    children.append(item);
                    break;
                }

                case QVariant::Double: {
                    if (isGeometryProperty(propertyName)) {
                        if (isGeometryHandled)
                            break;
                        isGeometryHandled = true;
                        const QRectF& geometry = UtilityFunctions::itemGeometry(properties);
                        bool xUnknown = false, yUnknown = false;
                        if (selectedControl->type() == Form::Type) {
                            xUnknown = !ParserUtils::exists(selectedControl->dir(), QStringLiteral("x"));
                            yUnknown = !ParserUtils::exists(selectedControl->dir(), QStringLiteral("y"));
                        }

                        const QString& geometryText = QStringLiteral("[(%1, %2), %3 x %4]")
                                .arg(xUnknown ? QStringLiteral("?") : QString::number(int(geometry.x())))
                                .arg(yUnknown ? QStringLiteral("?") : QString::number(int(geometry.y())))
                                .arg(int(geometry.width()))
                                .arg(int(geometry.height()));

                        const bool xChanged = ParserUtils::exists(selectedControl->dir(), QStringLiteral("x"));
                        const bool yChanged = ParserUtils::exists(selectedControl->dir(), QStringLiteral("y"));
                        const bool wChanged = ParserUtils::exists(selectedControl->dir(), QStringLiteral("width"));
                        const bool hChanged = ParserUtils::exists(selectedControl->dir(), QStringLiteral("height"));
                        const bool geometryChanged = xChanged || yChanged || wChanged || hChanged;

                        qreal x = geometry.x();
                        if (selectedControl->type() == Form::Type
                                && !ParserUtils::exists(selectedControl->dir(), propertyName)) {
                            x = 0;
                        }
                        qreal y = geometry.y();
                        if (selectedControl->type() == Form::Type
                                && !ParserUtils::exists(selectedControl->dir(), propertyName)) {
                            y = 0;
                        }

                        geometryItem = tree->delegate()->createItem();
                        geometryItem->setText(0, QStringLiteral("geometry"));
                        geometryItem->setText(1, geometryText);
                        geometryItem->setData(0, PropertiesDelegate::ModificationRole, geometryChanged);
                        children.append(geometryItem);

                        auto xItem = tree->delegate()->createItem();
                        auto callback = PropertiesDelegate::makeCallback(&PropertiesController::onRealPropertyEdit,
                                                                         this, xItem, geometryItem, QStringLiteral("x"));
                        xItem->setText(0, QStringLiteral("x"));
                        xItem->setData(0, PropertiesDelegate::ModificationRole, xChanged);
                        xItem->setData(1, PropertiesDelegate::InitialValueRole, x);
                        xItem->setData(1, PropertiesDelegate::TypeRole, PropertiesDelegate::Real);
                        xItem->setData(1, PropertiesDelegate::CallbackRole, callback.toVariant());
                        geometryChildren.append(xItem);

                        auto yItem = tree->delegate()->createItem();
                        callback = PropertiesDelegate::makeCallback(&PropertiesController::onRealPropertyEdit,
                                                                    this, yItem, geometryItem, QStringLiteral("y"));
                        yItem->setText(0, QStringLiteral("y"));
                        yItem->setData(0, PropertiesDelegate::ModificationRole, yChanged);
                        yItem->setData(1, PropertiesDelegate::InitialValueRole, y);
                        yItem->setData(1, PropertiesDelegate::TypeRole, PropertiesDelegate::Real);
                        yItem->setData(1, PropertiesDelegate::CallbackRole, callback.toVariant());
                        geometryChildren.append(yItem);

                        auto wItem = tree->delegate()->createItem();
                        callback = PropertiesDelegate::makeCallback(&PropertiesController::onRealPropertyEdit,
                                                                    this, wItem, geometryItem, QStringLiteral("width"));
                        wItem->setText(0, QStringLiteral("width"));
                        wItem->setData(0, PropertiesDelegate::ModificationRole, wChanged);
                        wItem->setData(1, PropertiesDelegate::InitialValueRole, geometry.width());
                        wItem->setData(1, PropertiesDelegate::TypeRole, PropertiesDelegate::Real);
                        wItem->setData(1, PropertiesDelegate::CallbackRole, callback.toVariant());
                        geometryChildren.append(wItem);

                        auto hItem = tree->delegate()->createItem();
                        callback = PropertiesDelegate::makeCallback(&PropertiesController::onRealPropertyEdit,
                                                                    this, hItem, geometryItem, QStringLiteral("height"));
                        hItem->setText(0, QStringLiteral("height"));
                        hItem->setData(0, PropertiesDelegate::ModificationRole, hChanged);
                        hItem->setData(1, PropertiesDelegate::InitialValueRole, geometry.height());
                        hItem->setData(1, PropertiesDelegate::TypeRole, PropertiesDelegate::Real);
                        hItem->setData(1, PropertiesDelegate::CallbackRole, callback.toVariant());
                        geometryChildren.append(hItem);
                    } else {
                        auto item = tree->delegate()->createItem();
                        auto callback = PropertiesDelegate::makeCallback(&PropertiesController::onRealPropertyEdit,
                                                                         this, item, nullptr, propertyName);
                        item->setText(0, propertyName);
                        item->setData(0, PropertiesDelegate::ModificationRole, ParserUtils::exists(selectedControl->dir(), propertyName));
                        item->setData(1, PropertiesDelegate::InitialValueRole, propertyValue.value<qreal>());
                        item->setData(1, PropertiesDelegate::TypeRole, PropertiesDelegate::Real);
                        item->setData(1, PropertiesDelegate::CallbackRole, callback.toVariant());
                        children.append(item);
                    }
                    break;
                }

                case QVariant::Int: {
                    if (isGeometryProperty(propertyName)) {
                        if (isGeometryHandled)
                            break;
                        isGeometryHandled = true;
                        const QRectF& geometry = UtilityFunctions::itemGeometry(properties);
                        bool xUnknown = false, yUnknown = false;
                        if (selectedControl->type() == Form::Type) {
                            xUnknown = !ParserUtils::exists(selectedControl->dir(), QStringLiteral("x"));
                            yUnknown = !ParserUtils::exists(selectedControl->dir(), QStringLiteral("y"));
                        }

                        const QString& geometryText = QStringLiteral("[(%1, %2), %3 x %4]")
                                .arg(xUnknown ? QStringLiteral("?") : QString::number(int(geometry.x())))
                                .arg(yUnknown ? QStringLiteral("?") : QString::number(int(geometry.y())))
                                .arg(int(geometry.width()))
                                .arg(int(geometry.height()));

                        const bool xChanged = ParserUtils::exists(selectedControl->dir(), QStringLiteral("x"));
                        const bool yChanged = ParserUtils::exists(selectedControl->dir(), QStringLiteral("y"));
                        const bool wChanged = ParserUtils::exists(selectedControl->dir(), QStringLiteral("width"));
                        const bool hChanged = ParserUtils::exists(selectedControl->dir(), QStringLiteral("height"));
                        const bool geometryChanged = xChanged || yChanged || wChanged || hChanged;

                        qreal x = geometry.x();
                        if (selectedControl->type() == Form::Type
                                && !ParserUtils::exists(selectedControl->dir(), propertyName)) {
                            x = 0;
                        }
                        qreal y = geometry.y();
                        if (selectedControl->type() == Form::Type
                                && !ParserUtils::exists(selectedControl->dir(), propertyName)) {
                            y = 0;
                        }

                        geometryItem = tree->delegate()->createItem();
                        geometryItem->setText(0, QStringLiteral("geometry"));
                        geometryItem->setText(1, geometryText);
                        geometryItem->setData(0, PropertiesDelegate::ModificationRole, geometryChanged);
                        children.append(geometryItem);

                        auto xItem = tree->delegate()->createItem();
                        auto callback = PropertiesDelegate::makeCallback(&PropertiesController::onIntPropertyEdit,
                                                                         this, xItem, geometryItem, QStringLiteral("x"));
                        xItem->setText(0, QStringLiteral("x"));
                        xItem->setData(0, PropertiesDelegate::ModificationRole, xChanged);
                        xItem->setData(1, PropertiesDelegate::InitialValueRole, x);
                        xItem->setData(1, PropertiesDelegate::TypeRole, PropertiesDelegate::Int);
                        xItem->setData(1, PropertiesDelegate::CallbackRole, callback.toVariant());
                        geometryChildren.append(xItem);

                        auto yItem = tree->delegate()->createItem();
                        callback = PropertiesDelegate::makeCallback(&PropertiesController::onIntPropertyEdit,
                                                                    this, yItem, geometryItem, QStringLiteral("y"));
                        yItem->setText(0, QStringLiteral("y"));
                        yItem->setData(0, PropertiesDelegate::ModificationRole, yChanged);
                        yItem->setData(1, PropertiesDelegate::InitialValueRole, y);
                        yItem->setData(1, PropertiesDelegate::TypeRole, PropertiesDelegate::Int);
                        yItem->setData(1, PropertiesDelegate::CallbackRole, callback.toVariant());
                        geometryChildren.append(yItem);

                        auto wItem = tree->delegate()->createItem();
                        callback = PropertiesDelegate::makeCallback(&PropertiesController::onIntPropertyEdit,
                                                                    this, wItem, geometryItem, QStringLiteral("width"));
                        wItem->setText(0, QStringLiteral("width"));
                        wItem->setData(0, PropertiesDelegate::ModificationRole, wChanged);
                        wItem->setData(1, PropertiesDelegate::InitialValueRole, geometry.width());
                        wItem->setData(1, PropertiesDelegate::TypeRole, PropertiesDelegate::Int);
                        wItem->setData(1, PropertiesDelegate::CallbackRole, callback.toVariant());
                        geometryChildren.append(wItem);

                        auto hItem = tree->delegate()->createItem();
                        callback = PropertiesDelegate::makeCallback(&PropertiesController::onIntPropertyEdit,
                                                                    this, hItem, geometryItem, QStringLiteral("height"));
                        hItem->setText(0, QStringLiteral("height"));
                        hItem->setData(0, PropertiesDelegate::ModificationRole, hChanged);
                        hItem->setData(1, PropertiesDelegate::InitialValueRole, geometry.height());
                        hItem->setData(1, PropertiesDelegate::TypeRole, PropertiesDelegate::Int);
                        hItem->setData(1, PropertiesDelegate::CallbackRole, callback.toVariant());
                        geometryChildren.append(hItem);
                    } else {
                        auto item = tree->delegate()->createItem();
                        auto callback = PropertiesDelegate::makeCallback(&PropertiesController::onIntPropertyEdit,
                                                                         this, item, nullptr, propertyName);
                        item->setText(0, propertyName);
                        item->setData(0, PropertiesDelegate::ModificationRole, ParserUtils::exists(selectedControl->dir(), propertyName));
                        item->setData(1, PropertiesDelegate::InitialValueRole, propertyValue.value<int>());
                        item->setData(1, PropertiesDelegate::TypeRole, PropertiesDelegate::Int);
                        item->setData(1, PropertiesDelegate::CallbackRole, callback.toVariant());
                        children.append(item);
                    }
                    break;
                }

                default:
                    break;
                }
            }

            for (const Enum& _enum : qAsConst(enumList)) {
                const QString& propertyName = _enum.name;
                QString value = _enum.value;
                if (selectedControl->window() && propertyName == QStringLiteral("visibility")) {
                    value = QStringLiteral("AutomaticVisibility");
                    const QString& visibility = ParserUtils::property(selectedControl->dir(), propertyName);
                    if (!visibility.isEmpty()) {
                        const QList<QString>& enumKeys = _enum.keys.keys();
                        for (const QString& key : enumKeys) {
                            if (visibility.contains(key))
                                value = key;
                        }
                    }
                }
                auto item = tree->delegate()->createItem();
                auto callback = PropertiesDelegate::makeCallback(&PropertiesController::onEnumPropertyEdit,
                                                                 this, item, nullptr, propertyName);
                item->setText(0, propertyName);
                item->setData(0, PropertiesDelegate::ModificationRole, ParserUtils::exists(selectedControl->dir(), propertyName));
                item->setData(1, PropertiesDelegate::ValuesRole, QVariant(_enum.keys.keys()));
                item->setData(1, PropertiesDelegate::InitialValueRole, value);
                item->setData(1, PropertiesDelegate::TypeRole, PropertiesDelegate::Enum);
                item->setData(1, PropertiesDelegate::CallbackRole, callback.toVariant());
                children.append(item);
            }

            if (fontItem) {
                fontItem->addChildren(fontChildren);
                fontItemCopy = fontItem;
            }
            if (geometryItem) {
                geometryItem->addChildren(geometryChildren);
                geometryItemCopy = geometryItem;
            }
            if (!children.isEmpty())
                classItem->addChildren(children);
            classItems.append(classItem);
        }

        tree->addTopLevelItems(classItems);

        m_isExpandCollapseSignalsBlocked = true;
        tree->expandAll();
        if (fontItemCopy && !m_fontItemOpen)
            tree->collapseItem(fontItemCopy);
        if (geometryItemCopy && !m_geometryItemOpen)
            tree->collapseItem(geometryItemCopy);
        m_isExpandCollapseSignalsBlocked = false;

        for (QTreeWidgetItem* topLevelItem : tree->topLevelItems()) {
            if (m_propertiesPane->isPermanentItem(topLevelItem))
                continue;
            for (QTreeWidgetItem* childItem : tree->allSubChildItems(topLevelItem, false, true, true)) {
                if (childItem->childCount() == 0) {
                    tree->openPersistentEditor(childItem, 1);
                    QWidget *focusWidget = tree->itemWidget(childItem, 1);
                    while (QWidget *fp = focusWidget->focusProxy())
                        focusWidget = fp;
                    if (QLineEdit *le = qobject_cast<QLineEdit*>(focusWidget))
                        le->deselect();
                    if (QAbstractSpinBox *sb = qobject_cast<QAbstractSpinBox*>(focusWidget)) {
                        if (QLineEdit* le = sb->findChild<QLineEdit*>())
                            le->deselect();
                    }
                }
            }
        }

        onSearchEditEditingFinish();
    } else {
        m_propertiesPane->resetButton()->setVisible(false);
    }

    if (tree->topLevelItemCount() > m_propertiesPane->permanentItemCount()) {
        tree->verticalScrollBar()->setSliderPosition(
                    tree->verticalScrollBar()->maximum() -
                    tree->verticalScrollBar()->minimum() -
                    verticalScrollPosition);
        tree->horizontalScrollBar()->setSliderPosition(
                    tree->horizontalScrollBar()->maximum() -
                    tree->horizontalScrollBar()->minimum() -
                    horizontalScrollPosition);
    }
}

void PropertiesController::onIntPropertyEdit(QTreeWidgetItem* item, QTreeWidgetItem* classItem,
                                             const QString& propertyName, const QVariant& value) const
{
    PropertiesTree* tree = m_propertiesPane->propertiesTree();

    if (tree->topLevelItemCount() <= m_propertiesPane->permanentItemCount())
        return;

    if (Control* selectedControl = this->control()) {
        const QSpinBox* spinBox = static_cast<QSpinBox*>(value.value<QSpinBox*>());
        const int value = spinBox->value();
        const QString& parserValue = QString::number(spinBox->value());

        item->setData(0, PropertiesDelegate::ModificationRole, true);
        if (classItem)
            classItem->setData(0, PropertiesDelegate::ModificationRole, true);

        // NOTE: No need for previous value equality check, since this signal is only emitted
        // when the value is changed

        ControlPropertyManager::Options options =
                ControlPropertyManager::SaveChanges | ControlPropertyManager::UpdateRenderer;

        if (selectedControl->type() == Form::Type
                && (propertyName == QStringLiteral("x") || propertyName == QStringLiteral("y"))) {
            options |= ControlPropertyManager::DontApplyDesigner;
        }

        if (propertyName == QStringLiteral("x")) {
            ControlPropertyManager::setX(selectedControl, value, options);
        } else if (propertyName == QStringLiteral("y")) {
            ControlPropertyManager::setY(selectedControl, value, options);
        } else if (propertyName == QStringLiteral("z")) {
            ControlPropertyManager::setZ(selectedControl, value, options);
        } else if (propertyName == QStringLiteral("width")) {
            ControlPropertyManager::setWidth(selectedControl, value, options);
        } else if (propertyName == QStringLiteral("height")) {
            ControlPropertyManager::setHeight(selectedControl, value, options);
        } else {
            ControlPropertyManager::setProperty(selectedControl, propertyName, parserValue, int(value), options);
        }
    }
}

void PropertiesController::onRealPropertyEdit(QTreeWidgetItem* item, QTreeWidgetItem* classItem,
                                              const QString& propertyName, const QVariant& value) const
{
    PropertiesTree* tree = m_propertiesPane->propertiesTree();

    if (tree->topLevelItemCount() <= m_propertiesPane->permanentItemCount())
        return;

    if (Control* selectedControl = this->control()) {
        const QDoubleSpinBox* spinBox = static_cast<QDoubleSpinBox*>(value.value<QDoubleSpinBox*>());
        const qreal value = spinBox->value();
        const QString& parserValue = QString::number(spinBox->value());

        item->setData(0, PropertiesDelegate::ModificationRole, true);
        if (classItem)
            classItem->setData(0, PropertiesDelegate::ModificationRole, true);

        // NOTE: No need for previous value equality check, since this signal is only emitted
        // when the value is changed

        ControlPropertyManager::Options options =
                ControlPropertyManager::SaveChanges | ControlPropertyManager::UpdateRenderer;

        if (selectedControl->type() == Form::Type
                && (propertyName == QStringLiteral("x") || propertyName == QStringLiteral("y"))) {
            options |= ControlPropertyManager::DontApplyDesigner;
        }

        if (propertyName == QStringLiteral("x")) {
            ControlPropertyManager::setX(selectedControl, value, options);
        } else if (propertyName == QStringLiteral("y")) {
            ControlPropertyManager::setY(selectedControl, value, options);
        } else if (propertyName == QStringLiteral("z")) {
            ControlPropertyManager::setZ(selectedControl, value, options);
        } else if (propertyName == QStringLiteral("width")) {
            ControlPropertyManager::setWidth(selectedControl, value, options);
        } else if (propertyName == QStringLiteral("height")) {
            ControlPropertyManager::setHeight(selectedControl, value, options);
        } else {
            ControlPropertyManager::setProperty(selectedControl, propertyName, parserValue, value, options);
        }
    }
}

void PropertiesController::onFontSizePropertyEdit(QTreeWidgetItem* item, QTreeWidgetItem* opponentItem,
                                                  QTreeWidgetItem* classItem, const QString& propertyName,
                                                  const QVariant& value) const
{
    PropertiesTree* tree = m_propertiesPane->propertiesTree();

    if (tree->topLevelItemCount() <= m_propertiesPane->permanentItemCount())
        return;

    if (Control* selectedControl = this->control()) {
        bool isPx = propertyName.contains(QStringLiteral("pixelSize")) ? true : false;
        QFont font = selectedControl->itemProperty(QStringLiteral("font")).value<QFont>();
        QSpinBox* spinBox = value.value<QSpinBox*>();

        opponentItem->setData(0, PropertiesDelegate::ModificationRole, false);
        item->setData(0, PropertiesDelegate::ModificationRole, true);
        if (classItem)
            classItem->setData(0, PropertiesDelegate::ModificationRole, true);

        // NOTE: No need for previous value equality check, since this signal is only emitted
        // when the value is changed

        if (spinBox->value() == 0) {
            spinBox->blockSignals(true);
            spinBox->setValue(isPx ? font.pixelSize() : font.pointSize());
            spinBox->blockSignals(false);
            return;
        }

        if (isPx)
            font.setPixelSize(spinBox->value());
        else
            font.setPointSize(spinBox->value());

        QString fontText = classItem->text(1);
        if (isPx) {
            fontText.replace(QRegExp(QStringLiteral(",.*")), QStringLiteral(", ") +
                             QString::number(font.pixelSize()) + QStringLiteral("px]"));
        } else {
            fontText.replace(QRegExp(QStringLiteral(",.*")), QStringLiteral(", ") +
                             QString::number(font.pointSize()) + QStringLiteral("pt]"));
        }
        classItem->setText(1, fontText);
        for (int i = 0; i < classItem->childCount(); ++i) {
            QTreeWidgetItem* chilItem = classItem->child(i);
            if (chilItem->text(0) == (isPx ? QStringLiteral("pointSize") : QStringLiteral("pixelSize"))) {
                auto spinBox = qobject_cast<QSpinBox*>(tree->itemWidget(chilItem, 1));
                Q_ASSERT(spinBox);
                spinBox->blockSignals(true);
                spinBox->setValue(0);
                spinBox->blockSignals(false);
                break;
            }
        }

        ControlPropertyManager::setBinding(selectedControl, QStringLiteral("font.") +
                                           (isPx ? QStringLiteral("pointSize") : QStringLiteral("pixelSize")),
                                           QString(), ControlPropertyManager::SaveChanges);
        ControlPropertyManager::setProperty(selectedControl, propertyName,
                                            QString::number(spinBox->value()), spinBox->value(),
                                            ControlPropertyManager::SaveChanges);
        ControlPropertyManager::setProperty(selectedControl, QStringLiteral("font"), QString(), font,
                                            ControlPropertyManager::UpdateRenderer);
    }
}

void PropertiesController::onFontFamilyPropertyEdit(QTreeWidgetItem* item, QTreeWidgetItem* classItem,
                                                    const QVariant& value) const
{
    PropertiesTree* tree = m_propertiesPane->propertiesTree();

    if (tree->topLevelItemCount() <= m_propertiesPane->permanentItemCount())
        return;

    if (Control* selectedControl = this->control()) {
        const QFont& font = selectedControl->itemProperty(QStringLiteral("font")).value<QFont>();
        const QString& previousFamily = QFontInfo(font).family();
        const QString& currentText = value.toString();

        if (currentText == previousFamily)
            return;

        item->setData(0, PropertiesDelegate::ModificationRole, true);
        if (classItem)
            classItem->setData(0, PropertiesDelegate::ModificationRole, true);

        QString fontText = classItem->text(1);
        fontText.replace(previousFamily, currentText);
        classItem->setText(1, fontText);

        ControlPropertyManager::setProperty(selectedControl, QStringLiteral("font.family"),
                                            UtilityFunctions::stringify(currentText), currentText,
                                            ControlPropertyManager::SaveChanges |
                                            ControlPropertyManager::UpdateRenderer);
    }
}

void PropertiesController::onFontWeightPropertyEdit(QTreeWidgetItem* item, QTreeWidgetItem* classItem,
                                                    const QVariant& value) const
{
    PropertiesTree* tree = m_propertiesPane->propertiesTree();

    if (tree->topLevelItemCount() <= m_propertiesPane->permanentItemCount())
        return;

    if (Control* selectedControl = this->control()) {
        const QString& currentText = value.toString();
        const QFont& font = selectedControl->itemProperty(QStringLiteral("font")).value<QFont>();
        const QMetaEnum& _enum = QMetaEnum::fromType<QFont::Weight>();
        const int weightValue = _enum.keyToValue(currentText.toUtf8().constData());

        if (weightValue == font.weight())
            return;

        item->setData(0, PropertiesDelegate::ModificationRole, true);
        if (classItem)
            classItem->setData(0, PropertiesDelegate::ModificationRole, true);

        ControlPropertyManager::setProperty(selectedControl, QStringLiteral("font.weight"),
                                            QStringLiteral("Font.") + currentText, weightValue,
                                            ControlPropertyManager::SaveChanges |
                                            ControlPropertyManager::UpdateRenderer);
    }
}

void PropertiesController::onFontCapitalizationPropertyEdit(QTreeWidgetItem* item,
                                                            QTreeWidgetItem* classItem,
                                                            const QVariant& value) const
{
    PropertiesTree* tree = m_propertiesPane->propertiesTree();

    if (tree->topLevelItemCount() <= m_propertiesPane->permanentItemCount())
        return;

    if (Control* selectedControl = this->control()) {
        const QString& currentText = value.toString();
        const QFont& font = selectedControl->itemProperty(QStringLiteral("font")).value<QFont>();
        const QMetaEnum& _enum = QMetaEnum::fromType<QFont::Capitalization>();
        const int capitalizationValue = _enum.keyToValue(currentText.toUtf8().constData());

        if (capitalizationValue == font.capitalization())
            return;

        item->setData(0, PropertiesDelegate::ModificationRole, true);
        if (classItem)
            classItem->setData(0, PropertiesDelegate::ModificationRole, true);

        ControlPropertyManager::setProperty(selectedControl, QStringLiteral("font.capitalization"),
                                            QStringLiteral("Font.") + currentText, capitalizationValue,
                                            ControlPropertyManager::SaveChanges |
                                            ControlPropertyManager::UpdateRenderer);
    }
}

void PropertiesController::onEnumPropertyEdit(QTreeWidgetItem* item, QTreeWidgetItem* classItem,
                                              const QString& propertyName, const QVariant& value) const
{
    PropertiesTree* tree = m_propertiesPane->propertiesTree();

    if (tree->topLevelItemCount() <= m_propertiesPane->permanentItemCount())
        return;

    if (Control* selectedControl = this->control()) {
        const QString& currentText = value.toString();
        const Enum& _enum = selectedControl->itemProperty(propertyName).value<Enum>();
        const QString& previousText = _enum.value;

        item->setData(0, PropertiesDelegate::ModificationRole, true);
        if (classItem)
            classItem->setData(0, PropertiesDelegate::ModificationRole, true);

        if (previousText == currentText)
            return;

        QFile file(SaveUtils::toControlMainQmlFile(selectedControl->dir()));
        if (!file.open(QFile::ReadOnly)) {
            qWarning("createEnumHandlerWidget: Cannot open control main qml file");
            return;
        }

        QString fixedScope = _enum.scope;
        if (selectedControl->window() && fixedScope == QStringLiteral("Window")) {
            if (!file.readAll().contains(QByteArrayLiteral("import QtQuick.Window")))
                fixedScope = QStringLiteral("ApplicationWindow");
        }
        file.close();

        ControlPropertyManager::setProperty(selectedControl,
                                            propertyName, fixedScope + '.' + currentText,
                                            _enum.keys.value(currentText),
                                            ControlPropertyManager::SaveChanges |
                                            ControlPropertyManager::UpdateRenderer);
    }
}

void PropertiesController::onUrlPropertyEdit(QTreeWidgetItem* item, QTreeWidgetItem* classItem,
                                             const QString& propertyName, const QVariant& value) const
{
    PropertiesTree* tree = m_propertiesPane->propertiesTree();

    if (tree->topLevelItemCount() <= m_propertiesPane->permanentItemCount())
        return;

    if (Control* selectedControl = this->control()) {
        // TODO: Clear whitespaces in the url
        const QUrl& url = QUrl::fromUserInput(value.toString(), SaveUtils::toControlThisDir(selectedControl->dir()),
                                              QUrl::AssumeLocalFile);
        const QString& displayText = cleanUrl(url, selectedControl->dir());
        const QUrl& previousUrl = selectedControl->itemProperty(propertyName).toUrl();

        if (url == previousUrl)
            return;

        item->setData(0, PropertiesDelegate::ModificationRole, true);
        if (classItem)
            classItem->setData(0, PropertiesDelegate::ModificationRole, true);

        ControlPropertyManager::setProperty(selectedControl, propertyName,
                                            UtilityFunctions::stringify(displayText), url,
                                            ControlPropertyManager::SaveChanges |
                                            ControlPropertyManager::UpdateRenderer);
    }
}

void PropertiesController::onStringPropertyEdit(QTreeWidgetItem* item, QTreeWidgetItem* classItem,
                                                const QString& propertyName, const QVariant& value) const
{
    PropertiesTree* tree = m_propertiesPane->propertiesTree();

    if (tree->topLevelItemCount() <= m_propertiesPane->permanentItemCount())
        return;

    if (Control* selectedControl = this->control()) {
        const QString& text = value.toString();
        const QString& previousText = selectedControl->itemProperty(propertyName).toString();

        if (previousText == text)
            return;

        item->setData(0, PropertiesDelegate::ModificationRole, true);
        if (classItem)
            classItem->setData(0, PropertiesDelegate::ModificationRole, true);

        ControlPropertyManager::setProperty(selectedControl, propertyName,
                                            UtilityFunctions::stringify(text), text,
                                            ControlPropertyManager::SaveChanges |
                                            ControlPropertyManager::UpdateRenderer);
    }
}

void PropertiesController::onBoolPropertyEdit(QTreeWidgetItem* item, QTreeWidgetItem* classItem,
                                              const QString& propertyName, const QVariant& value) const
{
    PropertiesTree* tree = m_propertiesPane->propertiesTree();

    if (tree->topLevelItemCount() <= m_propertiesPane->permanentItemCount())
        return;

    if (Control* selectedControl = this->control()) {
        item->setData(0, PropertiesDelegate::ModificationRole, true);
        if (classItem)
            classItem->setData(0, PropertiesDelegate::ModificationRole, true);
        // NOTE: No need for previous value equality check, since this
        // signal is only emitted when the value is changed/toggled
        bool checked = value.toBool();
        ControlPropertyManager::setProperty(selectedControl, propertyName,
                                            checked ? QStringLiteral("true") : QStringLiteral("false"), checked,
                                            ControlPropertyManager::SaveChanges |
                                            ControlPropertyManager::UpdateRenderer);
    }
}

void PropertiesController::onColorPropertyEdit(QTreeWidgetItem* item, QTreeWidgetItem* classItem,
                                               const QString& propertyName, const QVariant& value) const
{
    PropertiesTree* tree = m_propertiesPane->propertiesTree();

    if (tree->topLevelItemCount() <= m_propertiesPane->permanentItemCount())
        return;

    if (Control* selectedControl = this->control()) {
        QToolButton* toolButton = value.value<QToolButton*>();
        const QColor& previousColor = selectedControl->itemProperty(propertyName).value<QColor>();

        QColorDialog cDialog;
        cDialog.setWindowTitle(tr("Select Color"));
        cDialog.setOptions(QColorDialog::ShowAlphaChannel | QColorDialog::DontUseNativeDialog);
        cDialog.setCurrentColor(previousColor);
        cDialog.exec();

        const QColor& color = cDialog.currentColor();
        if (color == previousColor || !color.isValid())
            return;

        item->setData(0, PropertiesDelegate::ModificationRole, true);
        if (classItem)
            classItem->setData(0, PropertiesDelegate::ModificationRole, true);

        toolButton->setText(color.name(QColor::HexArgb));
        toolButton->setIcon(QIcon(PaintUtils::renderPropertyColorPixmap({12, 12}, color, {Qt::black}, toolButton->devicePixelRatioF())));
        ControlPropertyManager::setProperty(selectedControl, propertyName,
                                            UtilityFunctions::stringify(color.name(QColor::HexArgb)), color,
                                            ControlPropertyManager::SaveChanges |
                                            ControlPropertyManager::UpdateRenderer);
    }
}

bool PropertiesController::eventFilter(QObject* watched, QEvent* event)
{
    PropertiesTree* tree = m_propertiesPane->propertiesTree();

    if (watched == tree->viewport() && event->type() == QEvent::MouseMove) {
        bool buttonVisible = false;
        const QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
        const QTreeWidgetItem* item = tree->itemAt(mouseEvent->pos());
        if (item) {
            const QRect& itemRect = tree->visualItemRect(item);
            QRect resetButtonRect = m_propertiesPane->resetButton()->geometry();
            resetButtonRect.moveCenter({8, itemRect.center().y()});
            m_propertiesPane->resetButton()->setGeometry(resetButtonRect);
            buttonVisible = item->data(0, PropertiesDelegate::ModificationRole).toBool();
        }
        m_propertiesPane->resetButton()->setVisible(buttonVisible);
        return false;
    }
    return QObject::eventFilter(watched, event);
}

Control* PropertiesController::control() const
{
    const QList<Control*>& selectedControls = m_designerScene->selectedControls();
    if (selectedControls.size() != 1)
        return nullptr;
    return selectedControls.first();
}
