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


#include <QToolButton>
#include <QColorDialog>
#include <QSpinBox>
#include <QMetaEnum>

static bool isGeometryProperty(const QString& propertyName)
{
    return propertyName == QStringLiteral("x")
            || propertyName == QStringLiteral("y")
            || propertyName == QStringLiteral("width")
            || propertyName == QStringLiteral("height");
}

template<typename SpinBox>
static void fixPosForForm(const Control* control, const QString& propertyName, SpinBox spinBox)
{
    if (control->type() == Form::Type) {
        if ((propertyName == "x" || propertyName == "y")
                && !ParserUtils::exists(control->dir(), propertyName)) {
            spinBox->setValue(0);
        }
    }
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
{
    m_propertiesPane->propertiesTree()->setDesignerScene(designerScene);
    connect(m_propertiesPane->idEdit(), &QLineEdit::editingFinished,
            this, &PropertiesController::onControlIdEditingFinish);
    connect(m_propertiesPane->indexEdit(), &QSpinBox::editingFinished,
            this, &PropertiesController::onControlIndexEditingFinish);
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
}

void PropertiesController::discharge() const
{
    clear();
    m_propertiesPane->searchEdit()->clear();
}

void PropertiesController::clear() const
{
    m_propertiesPane->typeItem()->setHidden(true);
    m_propertiesPane->uidItem()->setHidden(true);
    m_propertiesPane->idItem()->setHidden(true);
    m_propertiesPane->indexItem()->setHidden(true);

    for (QTreeWidgetItem* topLevelItem : m_propertiesPane->propertiesTree()->topLevelItems()) {
        if (m_propertiesPane->isPermanentItem(topLevelItem))
            continue;
        for (QTreeWidgetItem* childItem : m_propertiesPane->propertiesTree()->allSubChildItems(topLevelItem, true, true, true))
            m_propertiesPane->propertiesTree()->delegate()->destroyItem(childItem);
    }
}

void PropertiesController::onSearchEditEditingFinish() const
{
    const QList<QTreeWidgetItem*>& topLevelItems = m_propertiesPane->propertiesTree()->topLevelItems();
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
    if (m_propertiesPane->propertiesTree()->topLevelItemCount() <= 0)
        return;

    if (Control* selectedControl = this->control()) {
        if (selectedControl != control)
            return;
        for (QTreeWidgetItem* topLevelItem : m_propertiesPane->propertiesTree()->topLevelItems()) {
            for (QTreeWidgetItem* childItem : m_propertiesPane->propertiesTree()->allSubChildItems(topLevelItem)) {
                if (childItem->text(0) == "z") {
                    QTreeWidget* treeWidget = childItem->treeWidget();
                    Q_ASSERT(treeWidget);
                    QSpinBox* iSpinBox
                            = qobject_cast<QSpinBox*>(treeWidget->itemWidget(childItem, 1));
                    QDoubleSpinBox* dSpinBox
                            = qobject_cast<QDoubleSpinBox*>(treeWidget->itemWidget(childItem, 1));
                    Q_ASSERT(iSpinBox || dSpinBox);

                    childItem->setData(0, PropertiesDelegate::ModificationRole, ParserUtils::exists(control->dir(), "z"));
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
    if (Control* selectedControl = this->control()) {
        if (selectedControl != control)
            return;
        if (m_propertiesPane->propertiesTree()->topLevelItemCount() <= 0)
            return onSceneSelectionChange();
        if (codeChanged)
            return onSceneSelectionChange();
        else
            return onControlGeometryChange(control);
    }
}

void PropertiesController::onControlGeometryChange(const Control* control) const
{
    if (m_propertiesPane->propertiesTree()->topLevelItemCount() <= 0)
        return;

    if (Control* selectedControl = this->control()) {
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

        for (QTreeWidgetItem* topLevelItem : m_propertiesPane->propertiesTree()->topLevelItems()) {
            for (QTreeWidgetItem* childItem : m_propertiesPane->propertiesTree()->allSubChildItems(topLevelItem)) {
                if (childItem->text(0) == "geometry") {
                    childItem->setText(1, geometryText);
                    childItem->setData(0, PropertiesDelegate::ModificationRole, geometryChanged);
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
                    childItem->setData(0, PropertiesDelegate::ModificationRole, xChanged);
                    if (dSpinBox) {
                        dSpinBox->setValue(geometry.x());
                        fixPosForForm(control, "x", dSpinBox);
                    } else {
                        iSpinBox->setValue(geometry.x());
                        fixPosForForm(control, "x", iSpinBox);
                    }
                } else if (childItem->text(0) == "y") {
                    childItem->setData(0, PropertiesDelegate::ModificationRole, yChanged);
                    if (dSpinBox) {
                        dSpinBox->setValue(geometry.y());
                        fixPosForForm(control, "y", dSpinBox);
                    } else {
                        iSpinBox->setValue(geometry.y());
                        fixPosForForm(control, "y", iSpinBox);
                    }
                } else if (childItem->text(0) == "width") {
                    childItem->setData(0, PropertiesDelegate::ModificationRole, wChanged);
                    if (dSpinBox)
                        dSpinBox->setValue(control->width());
                    else
                        iSpinBox->setValue(control->width());
                } else if (childItem->text(0) == "height") {
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
    if (m_propertiesPane->propertiesTree()->topLevelItemCount() <= 0)
        return;

    if (Control* selectedControl = this->control()) {

        QList<Control*> affectedControls({control});
        affectedControls.append(control->siblings());
        if (!affectedControls.contains(selectedControl))
            return;

        Control* issuedControl = nullptr;
        for (QTreeWidgetItem* topLevelItem : m_propertiesPane->propertiesTree()->topLevelItems()) {
            for (QTreeWidgetItem* childItem : m_propertiesPane->propertiesTree()->allSubChildItems(topLevelItem)) {
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
}

void PropertiesController::onControlIdChange(Control* control, const QString& /*previousId*/) const
{
    if (m_propertiesPane->propertiesTree()->topLevelItemCount() <= 0)
        return;

    if (Control* selectedControl = this->control()) {
        if (selectedControl != control)
            return;

        for (QTreeWidgetItem* topLevelItem : m_propertiesPane->propertiesTree()->topLevelItems()) {
            for (QTreeWidgetItem* childItem : m_propertiesPane->propertiesTree()->allSubChildItems(topLevelItem)) {
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

void PropertiesController::onControlIndexEditingFinish() const
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

    if (Control* selectedControl = control()) {
        m_propertiesPane->setDisabled(selectedControl->hasErrors());
        const QVector<PropertyNode>& properties = selectedControl->properties();

        if (properties.isEmpty())
            return;

        m_propertiesPane->typeItem()->setText(1, properties.first().cleanClassName);
        m_propertiesPane->uidItem()->setText(1, selectedControl->uid());
        m_propertiesPane->idEdit()->setText(selectedControl->id());
        m_propertiesPane->indexEdit()->setValue(selectedControl->index());
        m_propertiesPane->typeItem()->setHidden(false);
        m_propertiesPane->uidItem()->setHidden(false);
        m_propertiesPane->idItem()->setHidden(false);
        m_propertiesPane->indexItem()->setHidden(false);

        bool isGeometryHandled = false;
        QList<QTreeWidgetItem*> classItems;
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

            classItem = m_propertiesPane->propertiesTree()->delegate()->createItem();
            classItem->setText(0, propertyNode.cleanClassName);

            const QList<QString>& propertyKeys = propertyMap.keys();
            for (const QString& propertyName : propertyKeys) {
                const QVariant& propertyValue = propertyMap.value(propertyName);
                switch (propertyValue.type()) {
                case QVariant::Font: {
                    const QFont& font = propertyValue.value<QFont>();
                    const bool isPx = font.pixelSize() > 0 ? true : false;
                    const QString& family = QFontInfo(font).family();
                    const QString& fontText = QString::fromUtf8("[%1, %2%3]")
                            .arg(family)
                            .arg(isPx ? font.pixelSize() : font.pointSize())
                            .arg(isPx ? "px" : "pt");

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

                    fontItem = m_propertiesPane->propertiesTree()->delegate()->createItem();
                    fontItem->setText(0, QStringLiteral("font"));
                    fontItem->setText(1, fontText);
                    fontItem->setData(0, PropertiesDelegate::ModificationRole, fontChanged);
                    children.append(fontItem);

                    auto callback = PropertiesDelegate::makeCallback(&PropertiesController::onFontFamilyPropertyEdit, this, fontItem);
                    auto fItem = m_propertiesPane->propertiesTree()->delegate()->createItem();
                    fItem->setText(0, QStringLiteral("family"));
                    fItem->setData(0, PropertiesDelegate::ModificationRole, fChanged);
                    fItem->setData(1, PropertiesDelegate::InitialValueRole, family);
                    fItem->setData(1, PropertiesDelegate::TypeRole, PropertiesDelegate::FontFamily);
                    fItem->setData(1, PropertiesDelegate::CallbackRole, callback.toVariant());
                    fontChildren.append(fItem);

                    const QMetaEnum& e = QMetaEnum::fromType<QFont::Weight>();
                    callback = PropertiesDelegate::makeCallback(&PropertiesController::onFontWeightPropertyEdit, this, e);
                    auto wItem = m_propertiesPane->propertiesTree()->delegate()->createItem();
                    wItem->setText(0, QStringLiteral("weight"));
                    wItem->setData(0, PropertiesDelegate::ModificationRole, wChanged);
                    wItem->setData(1, PropertiesDelegate::InitialValueRole, e.valueToKey(font.weight()));
                    wItem->setData(1, PropertiesDelegate::TypeRole, PropertiesDelegate::FontWeight);
                    wItem->setData(1, PropertiesDelegate::CallbackRole, callback.toVariant());
                    fontChildren.append(wItem);

                    const QMetaEnum& e2 = QMetaEnum::fromType<QFont::Capitalization>();
                    callback = PropertiesDelegate::makeCallback(&PropertiesController::onFontCapitalizationPropertyEdit, this, e2);
                    auto cItem = m_propertiesPane->propertiesTree()->delegate()->createItem();
                    cItem->setText(0, QStringLiteral("capitalization"));
                    cItem->setData(0, PropertiesDelegate::ModificationRole, cChanged);
                    cItem->setData(1, PropertiesDelegate::InitialValueRole, e2.valueToKey(font.capitalization()));
                    cItem->setData(1, PropertiesDelegate::TypeRole, PropertiesDelegate::FontCapitalization);
                    cItem->setData(1, PropertiesDelegate::CallbackRole, callback.toVariant());
                    fontChildren.append(cItem);

                    callback = PropertiesDelegate::makeCallback(&PropertiesController::onFontSizePropertyEdit,
                                                                this, fontItem, QStringLiteral("font.pointSize"));
                    auto ptItem = m_propertiesPane->propertiesTree()->delegate()->createItem();
                    ptItem->setText(0, QStringLiteral("pointSize"));
                    ptItem->setData(0, PropertiesDelegate::ModificationRole, ptChanged);
                    ptItem->setData(1, PropertiesDelegate::InitialValueRole, font.pointSize() < 0 ? 0 : font.pointSize());
                    ptItem->setData(1, PropertiesDelegate::TypeRole, PropertiesDelegate::FontSize);
                    ptItem->setData(1, PropertiesDelegate::CallbackRole, callback.toVariant());
                    fontChildren.append(ptItem);

                    callback = PropertiesDelegate::makeCallback(&PropertiesController::onFontSizePropertyEdit,
                                                                this, fontItem, QStringLiteral("font.pixelSize"));
                    auto pxItem = m_propertiesPane->propertiesTree()->delegate()->createItem();
                    pxItem->setText(0, QStringLiteral("pixelSize"));
                    pxItem->setData(0, PropertiesDelegate::ModificationRole, piChanged);
                    pxItem->setData(1, PropertiesDelegate::InitialValueRole, font.pixelSize() < 0 ? 0 : font.pixelSize());
                    pxItem->setData(1, PropertiesDelegate::TypeRole, PropertiesDelegate::FontSize);
                    pxItem->setData(1, PropertiesDelegate::CallbackRole, callback.toVariant());
                    fontChildren.append(pxItem);

                    callback = PropertiesDelegate::makeCallback(&PropertiesController::onBoolPropertyEdit, this, QStringLiteral("font.bold"));
                    auto bItem = m_propertiesPane->propertiesTree()->delegate()->createItem();
                    bItem->setText(0, QStringLiteral("bold"));
                    bItem->setData(0, PropertiesDelegate::ModificationRole, bChanged);
                    bItem->setData(1, PropertiesDelegate::InitialValueRole, font.bold());
                    bItem->setData(1, PropertiesDelegate::TypeRole, PropertiesDelegate::Bool);
                    bItem->setData(1, PropertiesDelegate::CallbackRole, callback.toVariant());
                    fontChildren.append(bItem);

                    callback = PropertiesDelegate::makeCallback(&PropertiesController::onBoolPropertyEdit, this, QStringLiteral("font.italic"));
                    auto iItem = m_propertiesPane->propertiesTree()->delegate()->createItem();
                    iItem->setText(0, QStringLiteral("italic"));
                    iItem->setData(0, PropertiesDelegate::ModificationRole, iChanged);
                    iItem->setData(1, PropertiesDelegate::InitialValueRole, font.italic());
                    iItem->setData(1, PropertiesDelegate::TypeRole, PropertiesDelegate::Bool);
                    iItem->setData(1, PropertiesDelegate::CallbackRole, callback.toVariant());
                    fontChildren.append(iItem);

                    callback = PropertiesDelegate::makeCallback(&PropertiesController::onBoolPropertyEdit, this, QStringLiteral("font.underline"));
                    auto uItem = m_propertiesPane->propertiesTree()->delegate()->createItem();
                    uItem->setText(0, QStringLiteral("underline"));
                    uItem->setData(0, PropertiesDelegate::ModificationRole, uChanged);
                    uItem->setData(1, PropertiesDelegate::InitialValueRole, font.underline());
                    uItem->setData(1, PropertiesDelegate::TypeRole, PropertiesDelegate::Bool);
                    uItem->setData(1, PropertiesDelegate::CallbackRole, callback.toVariant());
                    fontChildren.append(uItem);

                    callback = PropertiesDelegate::makeCallback(&PropertiesController::onBoolPropertyEdit, this, QStringLiteral("font.overline"));
                    auto oItem = m_propertiesPane->propertiesTree()->delegate()->createItem();
                    oItem->setText(0, QStringLiteral("overline"));
                    oItem->setData(0, PropertiesDelegate::ModificationRole, oChanged);
                    oItem->setData(1, PropertiesDelegate::InitialValueRole, font.overline());
                    oItem->setData(1, PropertiesDelegate::TypeRole, PropertiesDelegate::Bool);
                    oItem->setData(1, PropertiesDelegate::CallbackRole, callback.toVariant());
                    fontChildren.append(oItem);

                    callback = PropertiesDelegate::makeCallback(&PropertiesController::onBoolPropertyEdit, this, QStringLiteral("font.strikeout"));
                    auto sItem = m_propertiesPane->propertiesTree()->delegate()->createItem();
                    sItem->setText(0, QStringLiteral("strikeout"));
                    sItem->setData(0, PropertiesDelegate::ModificationRole, sChanged);
                    sItem->setData(1, PropertiesDelegate::InitialValueRole, font.strikeOut());
                    sItem->setData(1, PropertiesDelegate::TypeRole, PropertiesDelegate::Bool);
                    sItem->setData(1, PropertiesDelegate::CallbackRole, callback.toVariant());
                    fontChildren.append(sItem);

                    callback = PropertiesDelegate::makeCallback(&PropertiesController::onBoolPropertyEdit, this, QStringLiteral("font.kerning"));
                    auto kItem = m_propertiesPane->propertiesTree()->delegate()->createItem();
                    kItem->setText(0, QStringLiteral("kerning"));
                    kItem->setData(0, PropertiesDelegate::ModificationRole, kChanged);
                    kItem->setData(1, PropertiesDelegate::InitialValueRole, font.kerning());
                    kItem->setData(1, PropertiesDelegate::TypeRole, PropertiesDelegate::Bool);
                    kItem->setData(1, PropertiesDelegate::CallbackRole, callback.toVariant());
                    fontChildren.append(kItem);

                    callback = PropertiesDelegate::makeCallback(&PropertiesController::onBoolPropertyEdit, this, QStringLiteral("font.preferShaping"));
                    auto prItem = m_propertiesPane->propertiesTree()->delegate()->createItem();
                    prItem->setText(0, QStringLiteral("preferShaping"));
                    prItem->setData(0, PropertiesDelegate::ModificationRole, prChanged);
                    prItem->setData(1, PropertiesDelegate::InitialValueRole, !(font.styleStrategy() & QFont::PreferNoShaping));
                    prItem->setData(1, PropertiesDelegate::TypeRole, PropertiesDelegate::Bool);
                    prItem->setData(1, PropertiesDelegate::CallbackRole, callback.toVariant());
                    fontChildren.append(prItem);
                    break;
                }

                case QVariant::Color: {
                    auto callback = PropertiesDelegate::makeCallback(&PropertiesController::onColorPropertyEdit,
                                                                     this, propertyName);
                    const QColor& color = propertyValue.value<QColor>();
                    auto item = m_propertiesPane->propertiesTree()->delegate()->createItem();
                    item->setText(0, propertyName);
                    item->setData(0, PropertiesDelegate::ModificationRole, ParserUtils::exists(selectedControl->dir(), propertyName));
                    item->setData(1, PropertiesDelegate::InitialValueRole, color);
                    item->setData(1, PropertiesDelegate::TypeRole, PropertiesDelegate::Color);
                    item->setData(1, PropertiesDelegate::CallbackRole, callback.toVariant());
                    children.append(item);
                    break;
                }

                case QVariant::Bool: {
                    auto callback = PropertiesDelegate::makeCallback(&PropertiesController::onBoolPropertyEdit,
                                                                     this, propertyName);
                    const bool checked = propertyName == "visible"
                            ? selectedControl->visible()
                            : propertyValue.value<bool>();
                    auto item = m_propertiesPane->propertiesTree()->delegate()->createItem();
                    item->setText(0, propertyName);
                    item->setData(0, PropertiesDelegate::ModificationRole, ParserUtils::exists(selectedControl->dir(), propertyName));
                    item->setData(1, PropertiesDelegate::InitialValueRole, checked);
                    item->setData(1, PropertiesDelegate::TypeRole, PropertiesDelegate::Bool);
                    item->setData(1, PropertiesDelegate::CallbackRole, callback.toVariant());
                    children.append(item);
                    break;
                }

                case QVariant::String: {
                    auto callback = PropertiesDelegate::makeCallback(&PropertiesController::onStringPropertyEdit,
                                                                     this, propertyName);
                    const QString& text = propertyValue.value<QString>();
                    auto item = m_propertiesPane->propertiesTree()->delegate()->createItem();
                    item->setText(0, propertyName);
                    item->setData(0, PropertiesDelegate::ModificationRole, ParserUtils::exists(selectedControl->dir(), propertyName));
                    item->setData(1, PropertiesDelegate::InitialValueRole, text);
                    item->setData(1, PropertiesDelegate::TypeRole, PropertiesDelegate::String);
                    item->setData(1, PropertiesDelegate::CallbackRole, callback.toVariant());
                    children.append(item);
                    break;
                }

                case QVariant::Url: {
                    auto callback = PropertiesDelegate::makeCallback(&PropertiesController::onUrlPropertyEdit,
                                                                     this, propertyName);
                    const QString& displayText = cleanUrl(propertyValue.value<QUrl>(), selectedControl->dir());
                    auto item = m_propertiesPane->propertiesTree()->delegate()->createItem();
                    item->setText(0, propertyName);
                    item->setData(0, PropertiesDelegate::ModificationRole, ParserUtils::exists(selectedControl->dir(), propertyName));
                    item->setData(1, PropertiesDelegate::InitialValueRole, displayText);
                    item->setData(1, PropertiesDelegate::TypeRole, PropertiesDelegate::String);
                    item->setData(1, PropertiesDelegate::CallbackRole, callback.toVariant());
                    children.append(item);
                    break;
                }

                case QVariant::Double: {
                    if (isGeometryProperty(propertyName) && !isGeometryHandled) {
                        isGeometryHandled = true;
                        const QRectF& geometry = UtilityFunctions::getGeometryFromProperties(properties);
                        bool xUnknown = false, yUnknown = false;
                        if (selectedControl->type() == Form::Type) {
                            xUnknown = !ParserUtils::exists(selectedControl->dir(), "x");
                            yUnknown = !ParserUtils::exists(selectedControl->dir(), "y");
                        }

                        const QString& geometryText = QString::fromUtf8("[(%1, %2), %3 x %4]")
                                .arg(xUnknown ? "?" : QString::number(int(geometry.x())))
                                .arg(yUnknown ? "?" : QString::number(int(geometry.y())))
                                .arg(int(geometry.width()))
                                .arg(int(geometry.height()));

                        const bool xChanged = ParserUtils::exists(selectedControl->dir(), "x");
                        const bool yChanged = ParserUtils::exists(selectedControl->dir(), "y");
                        const bool wChanged = ParserUtils::exists(selectedControl->dir(), "width");
                        const bool hChanged = ParserUtils::exists(selectedControl->dir(), "height");
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

                        geometryItem = m_propertiesPane->propertiesTree()->delegate()->createItem();
                        geometryItem->setText(0, QStringLiteral("geometry"));
                        geometryItem->setText(1, geometryText);
                        geometryItem->setData(0, PropertiesDelegate::ModificationRole, geometryChanged);
                        children.append(geometryItem);

                        auto callback = PropertiesDelegate::makeCallback(&PropertiesController::onRealPropertyEdit,
                                                                         this,  QStringLiteral("x"));
                        auto xItem = m_propertiesPane->propertiesTree()->delegate()->createItem();
                        xItem->setText(0, QStringLiteral("x"));
                        xItem->setData(0, PropertiesDelegate::ModificationRole, xChanged);
                        xItem->setData(1, PropertiesDelegate::InitialValueRole, x);
                        xItem->setData(1, PropertiesDelegate::TypeRole, PropertiesDelegate::Real);
                        xItem->setData(1, PropertiesDelegate::CallbackRole, callback.toVariant());
                        geometryChildren.append(xItem);

                        callback = PropertiesDelegate::makeCallback(&PropertiesController::onRealPropertyEdit,
                                                                    this,  QStringLiteral("y"));
                        auto yItem = m_propertiesPane->propertiesTree()->delegate()->createItem();
                        yItem->setText(0, QStringLiteral("y"));
                        yItem->setData(0, PropertiesDelegate::ModificationRole, yChanged);
                        yItem->setData(1, PropertiesDelegate::InitialValueRole, y);
                        yItem->setData(1, PropertiesDelegate::TypeRole, PropertiesDelegate::Real);
                        yItem->setData(1, PropertiesDelegate::CallbackRole, callback.toVariant());
                        geometryChildren.append(yItem);

                        callback = PropertiesDelegate::makeCallback(&PropertiesController::onRealPropertyEdit,
                                                                    this,  QStringLiteral("width"));
                        auto wItem = m_propertiesPane->propertiesTree()->delegate()->createItem();
                        wItem->setText(0, QStringLiteral("width"));
                        wItem->setData(0, PropertiesDelegate::ModificationRole, wChanged);
                        wItem->setData(1, PropertiesDelegate::InitialValueRole, geometry.width());
                        wItem->setData(1, PropertiesDelegate::TypeRole, PropertiesDelegate::Real);
                        wItem->setData(1, PropertiesDelegate::CallbackRole, callback.toVariant());
                        geometryChildren.append(wItem);

                        callback = PropertiesDelegate::makeCallback(&PropertiesController::onRealPropertyEdit,
                                                                    this,  QStringLiteral("height"));
                        auto hItem = m_propertiesPane->propertiesTree()->delegate()->createItem();
                        hItem->setText(0, QStringLiteral("height"));
                        hItem->setData(0, PropertiesDelegate::ModificationRole, hChanged);
                        hItem->setData(1, PropertiesDelegate::InitialValueRole, geometry.height());
                        hItem->setData(1, PropertiesDelegate::TypeRole, PropertiesDelegate::Real);
                        hItem->setData(1, PropertiesDelegate::CallbackRole, callback.toVariant());
                        geometryChildren.append(hItem);
                    } else {
                        auto callback = PropertiesDelegate::makeCallback(&PropertiesController::onRealPropertyEdit,
                                                                         this, propertyName);
                        auto item = m_propertiesPane->propertiesTree()->delegate()->createItem();
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
                    if (isGeometryProperty(propertyName) && !isGeometryHandled) {
                        isGeometryHandled = true;
                        const QRectF& geometry = UtilityFunctions::getGeometryFromProperties(properties);
                        bool xUnknown = false, yUnknown = false;
                        if (selectedControl->type() == Form::Type) {
                            xUnknown = !ParserUtils::exists(selectedControl->dir(), "x");
                            yUnknown = !ParserUtils::exists(selectedControl->dir(), "y");
                        }

                        const QString& geometryText = QString::fromUtf8("[(%1, %2), %3 x %4]")
                                .arg(xUnknown ? "?" : QString::number(int(geometry.x())))
                                .arg(yUnknown ? "?" : QString::number(int(geometry.y())))
                                .arg(int(geometry.width()))
                                .arg(int(geometry.height()));

                        const bool xChanged = ParserUtils::exists(selectedControl->dir(), "x");
                        const bool yChanged = ParserUtils::exists(selectedControl->dir(), "y");
                        const bool wChanged = ParserUtils::exists(selectedControl->dir(), "width");
                        const bool hChanged = ParserUtils::exists(selectedControl->dir(), "height");
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

                        geometryItem = m_propertiesPane->propertiesTree()->delegate()->createItem();
                        geometryItem->setText(0, QStringLiteral("geometry"));
                        geometryItem->setText(1, geometryText);
                        geometryItem->setData(0, PropertiesDelegate::ModificationRole, geometryChanged);
                        children.append(geometryItem);

                        auto callback = PropertiesDelegate::makeCallback(&PropertiesController::onIntPropertyEdit,
                                                                         this,  QStringLiteral("x"));
                        auto xItem = m_propertiesPane->propertiesTree()->delegate()->createItem();
                        xItem->setText(0, QStringLiteral("x"));
                        xItem->setData(0, PropertiesDelegate::ModificationRole, xChanged);
                        xItem->setData(1, PropertiesDelegate::InitialValueRole, x);
                        xItem->setData(1, PropertiesDelegate::TypeRole, PropertiesDelegate::Int);
                        xItem->setData(1, PropertiesDelegate::CallbackRole, callback.toVariant());
                        geometryChildren.append(xItem);

                        callback = PropertiesDelegate::makeCallback(&PropertiesController::onIntPropertyEdit,
                                                                    this,  QStringLiteral("y"));
                        auto yItem = m_propertiesPane->propertiesTree()->delegate()->createItem();
                        yItem->setText(0, QStringLiteral("y"));
                        yItem->setData(0, PropertiesDelegate::ModificationRole, yChanged);
                        yItem->setData(1, PropertiesDelegate::InitialValueRole, y);
                        yItem->setData(1, PropertiesDelegate::TypeRole, PropertiesDelegate::Int);
                        yItem->setData(1, PropertiesDelegate::CallbackRole, callback.toVariant());
                        geometryChildren.append(yItem);

                        callback = PropertiesDelegate::makeCallback(&PropertiesController::onIntPropertyEdit,
                                                                    this,  QStringLiteral("width"));
                        auto wItem = m_propertiesPane->propertiesTree()->delegate()->createItem();
                        wItem->setText(0, QStringLiteral("width"));
                        wItem->setData(0, PropertiesDelegate::ModificationRole, wChanged);
                        wItem->setData(1, PropertiesDelegate::InitialValueRole, geometry.width());
                        wItem->setData(1, PropertiesDelegate::TypeRole, PropertiesDelegate::Int);
                        wItem->setData(1, PropertiesDelegate::CallbackRole, callback.toVariant());
                        geometryChildren.append(wItem);

                        callback = PropertiesDelegate::makeCallback(&PropertiesController::onIntPropertyEdit,
                                                                    this,  QStringLiteral("height"));
                        auto hItem = m_propertiesPane->propertiesTree()->delegate()->createItem();
                        hItem->setText(0, QStringLiteral("height"));
                        hItem->setData(0, PropertiesDelegate::ModificationRole, hChanged);
                        hItem->setData(1, PropertiesDelegate::InitialValueRole, geometry.height());
                        hItem->setData(1, PropertiesDelegate::TypeRole, PropertiesDelegate::Int);
                        hItem->setData(1, PropertiesDelegate::CallbackRole, callback.toVariant());
                        geometryChildren.append(hItem);
                    } else {
                        auto callback = PropertiesDelegate::makeCallback(&PropertiesController::onIntPropertyEdit,
                                                                         this, propertyName);
                        auto item = m_propertiesPane->propertiesTree()->delegate()->createItem();
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
                if (selectedControl->window() && propertyName == "visibility") {
                    value = "AutomaticVisibility";
                    const QString& visibility = ParserUtils::property(selectedControl->dir(), propertyName);
                    if (!visibility.isEmpty()) {
                        const QList<QString>& enumKeys = _enum.keys.keys();
                        for (const QString& key : enumKeys) {
                            if (visibility.contains(key))
                                value = key;
                        }
                    }
                }
                auto callback = PropertiesDelegate::makeCallback(&PropertiesController::onEnumPropertyEdit,
                                                                 this, propertyName, _enum);
                auto item = m_propertiesPane->propertiesTree()->delegate()->createItem();
                item->setText(0, propertyName);
                item->setData(0, PropertiesDelegate::ModificationRole, ParserUtils::exists(selectedControl->dir(), propertyName));
                item->setData(1, PropertiesDelegate::ValuesRole, QVariant(_enum.keys.keys()));
                item->setData(1, PropertiesDelegate::InitialValueRole, value);
                item->setData(1, PropertiesDelegate::TypeRole, PropertiesDelegate::Enum);
                item->setData(1, PropertiesDelegate::CallbackRole, callback.toVariant());
                children.append(item);
            }

            if (fontItem)
                fontItem->addChildren(fontChildren);
            if (geometryItem)
                geometryItem->addChildren(geometryChildren);
            if (!children.isEmpty())
                classItem->addChildren(children);
            classItems.append(classItem);
        }

        m_propertiesPane->propertiesTree()->addTopLevelItems(classItems);
        m_propertiesPane->propertiesTree()->expandAll();

        for (QTreeWidgetItem* topLevelItem : m_propertiesPane->propertiesTree()->topLevelItems()) {
            if (m_propertiesPane->isPermanentItem(topLevelItem))
                continue;
            for (QTreeWidgetItem* childItem : m_propertiesPane->propertiesTree()->allSubChildItems(topLevelItem, false, true, true)) {
                if (childItem->childCount() == 0) {
                    m_propertiesPane->propertiesTree()->openPersistentEditor(childItem, 1);
                    QWidget *focusWidget = m_propertiesPane->propertiesTree()->itemWidget(childItem, 1);
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
    }
}

void PropertiesController::onIntPropertyEdit(const QString& propertyName, const QVariant& value) const
{
    if (m_propertiesPane->propertiesTree()->topLevelItemCount() <= 0)
        return;

    if (Control* selectedControl = this->control()) {
        const QSpinBox* spinBox = static_cast<QSpinBox*>(value.value<QSpinBox*>());
        const int value = spinBox->value();
        const QString& parserValue = QString::number(spinBox->value());

        // NOTE: No need for previous value equality check, since this signal is only emitted
        // when the value is changed

        ControlPropertyManager::Options options =
                ControlPropertyManager::SaveChanges | ControlPropertyManager::UpdateRenderer;

        if (selectedControl->type() == Form::Type && (propertyName == "x" || propertyName == "y"))
            options |= ControlPropertyManager::DontApplyDesigner;

        if (propertyName == "x") {
            ControlPropertyManager::setX(selectedControl, value, options);
        } else if (propertyName == "y") {
            ControlPropertyManager::setY(selectedControl, value, options);
        } else if (propertyName == "z") {
            ControlPropertyManager::setZ(selectedControl, value, options);
        } else if (propertyName == "width") {
            ControlPropertyManager::setWidth(selectedControl, value, options);
        } else if (propertyName == "height") {
            ControlPropertyManager::setHeight(selectedControl, value, options);
        } else {
            ControlPropertyManager::setProperty(selectedControl, propertyName, parserValue, int(value), options);
        }
    }
}

void PropertiesController::onRealPropertyEdit(const QString& propertyName, const QVariant& value) const
{
    if (m_propertiesPane->propertiesTree()->topLevelItemCount() <= 0)
        return;

    if (Control* selectedControl = this->control()) {
        const QDoubleSpinBox* spinBox = static_cast<QDoubleSpinBox*>(value.value<QDoubleSpinBox*>());
        const qreal value = spinBox->value();
        const QString& parserValue = QString::number(spinBox->value());

        // NOTE: No need for previous value equality check, since this signal is only emitted
        // when the value is changed

        ControlPropertyManager::Options options =
                ControlPropertyManager::SaveChanges | ControlPropertyManager::UpdateRenderer;

        if (selectedControl->type() == Form::Type && (propertyName == "x" || propertyName == "y"))
            options |= ControlPropertyManager::DontApplyDesigner;

        if (propertyName == "x") {
            ControlPropertyManager::setX(selectedControl, value, options);
        } else if (propertyName == "y") {
            ControlPropertyManager::setY(selectedControl, value, options);
        } else if (propertyName == "z") {
            ControlPropertyManager::setZ(selectedControl, value, options);
        } else if (propertyName == "width") {
            ControlPropertyManager::setWidth(selectedControl, value, options);
        } else if (propertyName == "height") {
            ControlPropertyManager::setHeight(selectedControl, value, options);
        } else {
            ControlPropertyManager::setProperty(selectedControl, propertyName, parserValue, value, options);
        }
    }
}

void PropertiesController::onFontSizePropertyEdit(QTreeWidgetItem* fontClassItem, const QString& propertyName, const QVariant& value) const
{
    if (m_propertiesPane->propertiesTree()->topLevelItemCount() <= 0)
        return;

    if (Control* selectedControl = this->control()) {
        bool isPx = propertyName.contains("pixelSize") ? true : false;
        QFont font = UtilityFunctions::getProperty("font", selectedControl->properties()).value<QFont>();
        QSpinBox* spinBox = value.value<QSpinBox*>();

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

        QString fontText = fontClassItem->text(1);
        if (isPx)
            fontText.replace(QRegExp(",.*"), ", " + QString::number(font.pixelSize()) + "px]");
        else
            fontText.replace(QRegExp(",.*"), ", " + QString::number(font.pointSize()) + "pt]");
        fontClassItem->setText(1, fontText);
        for (int i = 0; i < fontClassItem->childCount(); ++i) {
            QTreeWidgetItem* chilItem = fontClassItem->child(i);
            if (chilItem->text(0) == (isPx ? "pointSize" : "pixelSize")) {
                auto spinBox = qobject_cast<QSpinBox*>(m_propertiesPane->propertiesTree()->itemWidget(chilItem, 1));
                Q_ASSERT(spinBox);
                spinBox->blockSignals(true);
                spinBox->setValue(0);
                spinBox->blockSignals(false);
                break;
            }
        }

        // FIXME: Remove related property instead of setting its value to 0
        ControlPropertyManager::setProperty(selectedControl, QString::fromUtf8("font.") +
                                            (isPx ? "pointSize" : "pixelSize"),
                                            QString::number(0), 0,
                                            ControlPropertyManager::SaveChanges);
        ControlPropertyManager::setProperty(selectedControl, propertyName,
                                            QString::number(spinBox->value()), spinBox->value(),
                                            ControlPropertyManager::SaveChanges);
        ControlPropertyManager::setProperty(selectedControl, "font", QString(), font,
                                            ControlPropertyManager::UpdateRenderer);
    }
}

void PropertiesController::onFontFamilyPropertyEdit(QTreeWidgetItem* fontClassItem, const QVariant& value) const
{
    if (m_propertiesPane->propertiesTree()->topLevelItemCount() <= 0)
        return;

    if (Control* selectedControl = this->control()) {
        const QFont& font = UtilityFunctions::getProperty("font", selectedControl->properties()).value<QFont>();
        const QString& previousFamily = QFontInfo(font).family();
        const QString& currentText = value.toString();

        if (currentText == previousFamily)
            return;

        QString fontText = fontClassItem->text(1);
        fontText.replace(previousFamily, currentText);
        fontClassItem->setText(1, fontText);

        ControlPropertyManager::setProperty(selectedControl, "font.family",
                                            UtilityFunctions::stringify(currentText), currentText,
                                            ControlPropertyManager::SaveChanges |
                                            ControlPropertyManager::UpdateRenderer);
    }
}

void PropertiesController::onFontWeightPropertyEdit(const QMetaEnum& _enum, const QVariant& value) const
{
    if (m_propertiesPane->propertiesTree()->topLevelItemCount() <= 0)
        return;

    if (Control* selectedControl = this->control()) {
        const QString& currentText = value.toString();
        const QFont& font = UtilityFunctions::getProperty("font", selectedControl->properties()).value<QFont>();
        const int weightValue = _enum.keyToValue(currentText.toUtf8().constData());

        if (weightValue == font.weight())
            return;

        ControlPropertyManager::setProperty(selectedControl, "font.weight",
                                            "Font." + currentText, weightValue,
                                            ControlPropertyManager::SaveChanges |
                                            ControlPropertyManager::UpdateRenderer);
    }
}

void PropertiesController::onFontCapitalizationPropertyEdit(const QMetaEnum& _enum, const QVariant& value) const
{
    if (m_propertiesPane->propertiesTree()->topLevelItemCount() <= 0)
        return;

    if (Control* selectedControl = this->control()) {
        const QString& currentText = value.toString();
        const QFont& font = UtilityFunctions::getProperty("font", selectedControl->properties()).value<QFont>();
        const int capitalizationValue = _enum.keyToValue(currentText.toUtf8().constData());

        if (capitalizationValue == font.capitalization())
            return;

        ControlPropertyManager::setProperty(selectedControl, "font.capitalization",
                                            "Font." + currentText, capitalizationValue,
                                            ControlPropertyManager::SaveChanges |
                                            ControlPropertyManager::UpdateRenderer);
    }
}

void PropertiesController::onEnumPropertyEdit(const QString& propertyName, const Enum& _enum, const QVariant& value) const
{
    if (m_propertiesPane->propertiesTree()->topLevelItemCount() <= 0)
        return;

    if (Control* selectedControl = this->control()) {
        const QString& currentText = value.toString();
        const QString& previousText = UtilityFunctions::getEnum(propertyName, selectedControl->properties()).value;

        if (previousText == currentText)
            return;

        QFile file(SaveUtils::toControlMainQmlFile(selectedControl->dir()));
        if (!file.open(QFile::ReadOnly)) {
            qWarning("createEnumHandlerWidget: Cannot open control main qml file");
            return;
        }

        QString fixedScope = _enum.scope;
        if (selectedControl->window() && fixedScope == "Window") {
            if (!file.readAll().contains("import QtQuick.Window"))
                fixedScope = "ApplicationWindow";
        }
        file.close();

        ControlPropertyManager::setProperty(selectedControl,
                                            propertyName, fixedScope + "." + currentText,
                                            _enum.keys.value(currentText),
                                            ControlPropertyManager::SaveChanges |
                                            ControlPropertyManager::UpdateRenderer);
    }
}

void PropertiesController::onUrlPropertyEdit(const QString& propertyName, const QVariant& value) const
{
    if (m_propertiesPane->propertiesTree()->topLevelItemCount() <= 0)
        return;

    if (Control* selectedControl = this->control()) {
        // TODO: Clear whitespaces in the url
        const QUrl& url = QUrl::fromUserInput(value.toString(), SaveUtils::toControlThisDir(selectedControl->dir()),
                                              QUrl::AssumeLocalFile);
        const QString& displayText = cleanUrl(url, selectedControl->dir());
        const QUrl& previousUrl = UtilityFunctions::getProperty(propertyName, selectedControl->properties()).value<QUrl>();

        if (url == previousUrl)
            return;

        ControlPropertyManager::setProperty(selectedControl, propertyName,
                                            UtilityFunctions::stringify(displayText), url,
                                            ControlPropertyManager::SaveChanges |
                                            ControlPropertyManager::UpdateRenderer);
    }
}

void PropertiesController::onStringPropertyEdit(const QString& propertyName, const QVariant& value) const
{
    if (m_propertiesPane->propertiesTree()->topLevelItemCount() <= 0)
        return;

    if (Control* selectedControl = this->control()) {
        const QString& text = value.toString();
        const QString& previousText
                = UtilityFunctions::getProperty(propertyName, selectedControl->properties()).value<QString>();

        if (previousText == text)
            return;

        ControlPropertyManager::setProperty(selectedControl, propertyName,
                                            UtilityFunctions::stringify(text), text,
                                            ControlPropertyManager::SaveChanges |
                                            ControlPropertyManager::UpdateRenderer);
    }
}

void PropertiesController::onBoolPropertyEdit(const QString& propertyName, const QVariant& value) const
{
    if (m_propertiesPane->propertiesTree()->topLevelItemCount() <= 0)
        return;

    if (Control* selectedControl = this->control()) {
        // NOTE: No need for previous value equality check, since this
        // signal is only emitted when the value is changed/toggled
        bool checked = value.toBool();
        ControlPropertyManager::setProperty(selectedControl, propertyName,
                                            checked ? "true" : "false", checked,
                                            ControlPropertyManager::SaveChanges |
                                            ControlPropertyManager::UpdateRenderer);
    }
}

void PropertiesController::onColorPropertyEdit(const QString& propertyName, const QVariant& value) const
{
    if (m_propertiesPane->propertiesTree()->topLevelItemCount() <= 0)
        return;

    if (Control* selectedControl = this->control()) {
        QToolButton* toolButton = value.value<QToolButton*>();
        const QColor& previousColor = UtilityFunctions::getProperty(propertyName, selectedControl->properties()).value<QColor>();

        QColorDialog cDialog;
        cDialog.setWindowTitle(QObject::tr("Select Color"));
        cDialog.setOptions(QColorDialog::ShowAlphaChannel | QColorDialog::DontUseNativeDialog);
        cDialog.setCurrentColor(previousColor);
        cDialog.exec();

        const QColor& color = cDialog.currentColor();
        if (color == previousColor || !color.isValid())
            return;

        toolButton->setText(color.name(QColor::HexArgb));
        toolButton->setIcon(QIcon(PaintUtils::renderPropertyColorPixmap({12, 12}, color, {Qt::black}, toolButton->devicePixelRatioF())));
        ControlPropertyManager::setProperty(selectedControl, propertyName,
                                            UtilityFunctions::stringify(color.name(QColor::HexArgb)), color,
                                            ControlPropertyManager::SaveChanges |
                                            ControlPropertyManager::UpdateRenderer);
    }
}

Control* PropertiesController::control() const
{
    const QList<Control*>& selectedControls = m_designerScene->selectedControls();
    if (selectedControls.size() != 1)
        return nullptr;
    return selectedControls.first();
}
