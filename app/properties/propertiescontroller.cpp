#include <propertiescontroller.h>
#include <propertiespane.h>
#include <propertiestree.h>
#include <propertiesdelegate.h>

#include <lineedit.h>

#include <saveutils.h>
#include <controlpropertymanager.h>
#include <designerscene.h>
#include <parserutils.h>
#include <transparentstyle.h>
#include <paintutils.h>
#include <utilityfunctions.h>
#include <form.h>

#include <QStyledItemDelegate>
#include <QPainter>
#include <QHeaderView>
#include <QScrollBar>
#include <QCheckBox>
#include <QToolButton>
#include <QColorDialog>
#include <QSpinBox>
#include <QFontDatabase>
#include <QMetaEnum>
#include <QComboBox>
#include <QHBoxLayout>
#include <QApplication>

static bool isXProperty(const QString& propertyName)
{
    return propertyName == "x";
}

static bool isGeometryProperty(const QString& propertyName)
{
    return propertyName == "x"
            || propertyName == "y"
            || propertyName == "width"
            || propertyName == "height";
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

static QWidget* createNumberHandlerWidget(const QString& propertyName, double number,
                                          Control* control, bool integer)
{
    QAbstractSpinBox* abstractSpinBox;
    if (integer)
        abstractSpinBox = new QSpinBox;
    else
        abstractSpinBox = new QDoubleSpinBox;

    TransparentStyle::attach(abstractSpinBox);
    abstractSpinBox->setCursor(Qt::PointingHandCursor);
    abstractSpinBox->setFocusPolicy(Qt::StrongFocus);
    abstractSpinBox->setSizePolicy(QSizePolicy::Ignored, abstractSpinBox->sizePolicy().verticalPolicy());
    abstractSpinBox->setMinimumWidth(1);
    UtilityFunctions::disableWheelEvent(abstractSpinBox);

    const auto& updateFunction = [=]
    {
        double value;
        QString parserValue;

        if (integer) {
            QSpinBox* spinBox = static_cast<QSpinBox*>(abstractSpinBox);
            value = spinBox->value();
            parserValue = QString::number(spinBox->value());
        } else {
            QDoubleSpinBox* spinBox = static_cast<QDoubleSpinBox*>(abstractSpinBox);
            value = spinBox->value();
            parserValue = QString::number(spinBox->value());
        }

        // NOTE: No need for previous value equality check, since this signal is only emitted
        // when the value is changed

        ControlPropertyManager::Options options =
                ControlPropertyManager::SaveChanges | ControlPropertyManager::UpdateRenderer;

        if (control->type() == Form::Type && (propertyName == "x" || propertyName == "y"))
            options |= ControlPropertyManager::DontApplyDesigner;

        if (propertyName == "x") {
            ControlPropertyManager::setX(control, value, options);
        } else if (propertyName == "y") {
            ControlPropertyManager::setY(control, value, options);
        } else if (propertyName == "z") {
            ControlPropertyManager::setZ(control, value, options);
        } else if (propertyName == "width") {
            ControlPropertyManager::setWidth(control, value, options);
        } else if (propertyName == "height") {
            ControlPropertyManager::setHeight(control, value, options);
        } else {
            ControlPropertyManager::setProperty(control, propertyName, parserValue,
                                                integer ? int(value) : value, options);
        }
    };

    if (integer) {
        QSpinBox* spinBox = static_cast<QSpinBox*>(abstractSpinBox);
        spinBox->setMaximum(std::numeric_limits<int>::max());
        spinBox->setMinimum(std::numeric_limits<int>::lowest());
        spinBox->setValue(number);
        fixPosForForm(control, propertyName, spinBox);
        QObject::connect(spinBox, qOverload<int>(&QSpinBox::valueChanged), updateFunction);
    } else {
        QDoubleSpinBox* spinBox = static_cast<QDoubleSpinBox*>(abstractSpinBox);
        spinBox->setMaximum(std::numeric_limits<double>::max());
        spinBox->setMinimum(std::numeric_limits<double>::lowest());
        spinBox->setValue(number);
        fixPosForForm(control, propertyName, spinBox);
        QObject::connect(spinBox, qOverload<double>(&QDoubleSpinBox::valueChanged), updateFunction);
    }

    return abstractSpinBox;
}

PropertiesController::PropertiesController(PropertiesPane* propertiesPane, DesignerScene* designerScene,
                                           QObject* parent) : QObject(parent)
  , m_propertiesPane(propertiesPane)
  , m_designerScene(designerScene)
{
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

void PropertiesController::discharge()
{
    clear();
    m_propertiesPane->searchEdit()->clear();
}

void PropertiesController::clear()
{
    m_propertiesPane->typeItem()->setHidden(true);
    m_propertiesPane->uidItem()->setHidden(true);
    m_propertiesPane->idItem()->setHidden(true);
    m_propertiesPane->indexItem()->setHidden(true);

    for (QTreeWidgetItem* topLevelItem : m_propertiesPane->propertiesTree()->topLevelItems()) {
        if (m_propertiesPane->isPermanentItem(topLevelItem))
            continue;
        for (QTreeWidgetItem* childItem : m_propertiesPane->propertiesTree()->allSubChildItems(topLevelItem))
            m_propertiesPane->propertiesTree()->delegate()->destroyItem(childItem);
        m_propertiesPane->propertiesTree()->delegate()->destroyItem(topLevelItem);
    }
}

void PropertiesController::onSearchEditEditingFinish()
{
    //    const QList<QTreeWidgetItem*>& topLevelItems = m_propertiesPane->propertiesTree()->topLevelItems();
    //    const QString& searchTerm = m_propertiesPane->searchEdit()->text();
    //    for (QTreeWidgetItem* topLevelItem : topLevelItems) {
    //        auto tlv = false;
    //        for (int j = 0; j < topLevelItem->childCount(); j++) {
    //            auto tci = topLevelItem->child(j);
    //            auto tcv = false;
    //            auto vv = tci->text(0).contains(searchTerm, Qt::CaseInsensitive);

    //            for (int z = 0; z < tci->childCount(); z++) {
    //                auto tdi = tci->child(z);
    //                auto v = (searchTerm.isEmpty() || vv)
    //                        ? true
    //                        : tdi->text(0).contains(searchTerm, Qt::CaseInsensitive);

    //                tdi->setHidden(!v);
    //                if (v)
    //                    tcv = v;
    //            }

    //            auto v = searchTerm.isEmpty() ? true : (tci->childCount() > 0 ? tcv : vv);
    //            tci->setHidden(!v);
    //            if (v)
    //                tlv = v;
    //        }

    //        auto v = searchTerm.isEmpty() ? true : tlv;
    //        topLevelItem->setHidden(!v);
    //    }
}

// FIXME: This function has severe performance issues.
void PropertiesController::onSceneSelectionChange()
{
    clear();

    if (Control* selectedControl = control()) {
        m_propertiesPane->setDisabled(selectedControl->hasErrors());
        QVector<PropertyNode> properties = selectedControl->properties();
        if (properties.isEmpty())
            return;

        m_propertiesPane->propertiesTree()->viewport()->setUpdatesEnabled(false);
        m_propertiesPane->propertiesTree()->setUpdatesEnabled(false);
        m_propertiesPane->setUpdatesEnabled(false);

        m_propertiesPane->typeItem()->setText(1, properties.first().cleanClassName);
        m_propertiesPane->uidItem()->setText(1, selectedControl->uid());
        m_propertiesPane->idEdit()->setText(selectedControl->id());
        m_propertiesPane->indexEdit()->setValue(selectedControl->index());
        m_propertiesPane->typeItem()->setHidden(false);
        m_propertiesPane->uidItem()->setHidden(false);
        m_propertiesPane->idItem()->setHidden(false);
        m_propertiesPane->indexItem()->setHidden(false);

        for (const PropertyNode& propertyNode : properties) {
            const QVector<Enum>& enumList = propertyNode.enums;
            const QMap<QString, QVariant>& propertyMap = propertyNode.properties;

            if (propertyMap.isEmpty() && enumList.isEmpty())
                continue;

            auto classItem = new QTreeWidgetItem;
            classItem->setText(0, propertyNode.cleanClassName);
            m_propertiesPane->propertiesTree()->addTopLevelItem(classItem);

            QList<QTreeWidgetItem*> children;
            for (const QString& propertyName : propertyMap.keys()) {
                const QVariant& propertyValue = propertyMap.value(propertyName);

                switch (propertyValue.type()) {
                case QVariant::Font: {
                    QList<QTreeWidgetItem*> fontChildren;
                    const QFont& font = propertyValue.value<QFont>();
                    const QString& family = QFontInfo(font).family();
                    const bool isPx = font.pixelSize() > 0 ? true : false;
                    const QString& fontText = QString::fromUtf8("[%1, %2%3]")
                            .arg(family)
                            .arg(isPx ? font.pixelSize() : font.pointSize())
                            .arg(isPx ? "px" : "pt");

                    const bool fChanged    = ParserUtils::exists(selectedControl->dir(), "font.family");
                    const bool bChanged    = ParserUtils::exists(selectedControl->dir(), "font.bold");
                    const bool iChanged    = ParserUtils::exists(selectedControl->dir(), "font.italic");
                    const bool uChanged    = ParserUtils::exists(selectedControl->dir(), "font.underline");
                    const bool ptChanged   = ParserUtils::exists(selectedControl->dir(), "font.pointSize");
                    const bool piChanged   = ParserUtils::exists(selectedControl->dir(), "font.pixelSize");
                    const bool wChanged    = ParserUtils::exists(selectedControl->dir(), "font.weight");
                    const bool oChanged    = ParserUtils::exists(selectedControl->dir(), "font.overline");
                    const bool sChanged    = ParserUtils::exists(selectedControl->dir(), "font.strikeout");
                    const bool cChanged    = ParserUtils::exists(selectedControl->dir(), "font.capitalization");
                    const bool kChanged    = ParserUtils::exists(selectedControl->dir(), "font.kerning");
                    const bool prChanged   = ParserUtils::exists(selectedControl->dir(), "font.preferShaping");
                    const bool fontChanged = fChanged || bChanged || iChanged || uChanged || ptChanged || piChanged
                            || wChanged || oChanged || sChanged || cChanged || kChanged || prChanged;

                    auto fontItem = new QTreeWidgetItem;
                    fontItem->setText(0, "font");
                    fontItem->setText(1, fontText);
                    fontItem->setData(0, PropertiesDelegate::ModificationRole, fontChanged);
                    classItem->addChild(fontItem);

                    auto callback = PropertiesDelegate::makeCallback(&PropertiesController::onFontFamilyPropertyEdit, this, fontItem);
                    auto fItem = m_propertiesPane->propertiesTree()->delegate()->createItem();
                    fItem->setText(1, QString());
                    fItem->setText(0, QStringLiteral("family"));
                    fItem->setData(0, PropertiesDelegate::ModificationRole, fChanged);
                    fItem->setData(1, PropertiesDelegate::InitialValueRole, family);
                    fItem->setData(1, PropertiesDelegate::TypeRole, PropertiesDelegate::FontFamily);
                    fItem->setData(1, PropertiesDelegate::CallbackRole, callback.toVariant());
                    fontChildren.append(fItem);

                    const QMetaEnum& e = QMetaEnum::fromType<QFont::Weight>();
                    callback = PropertiesDelegate::makeCallback(&PropertiesController::onFontWeightPropertyEdit, this, e);
                    auto wItem = m_propertiesPane->propertiesTree()->delegate()->createItem();
                    wItem->setText(1, QString());
                    wItem->setText(0, QStringLiteral("weight"));
                    wItem->setData(0, PropertiesDelegate::ModificationRole, wChanged);
                    wItem->setData(1, PropertiesDelegate::InitialValueRole, e.valueToKey(font.weight()));
                    wItem->setData(1, PropertiesDelegate::TypeRole, PropertiesDelegate::FontWeight);
                    wItem->setData(1, PropertiesDelegate::CallbackRole, callback.toVariant());
                    fontChildren.append(wItem);

                    const QMetaEnum& e2 = QMetaEnum::fromType<QFont::Capitalization>();
                    callback = PropertiesDelegate::makeCallback(&PropertiesController::onFontCapitalizationPropertyEdit, this, e2);
                    auto cItem = m_propertiesPane->propertiesTree()->delegate()->createItem();
                    cItem->setText(1, QString());
                    cItem->setText(0, QStringLiteral("capitalization"));
                    cItem->setData(0, PropertiesDelegate::ModificationRole, cChanged);
                    cItem->setData(1, PropertiesDelegate::InitialValueRole, e2.valueToKey(font.capitalization()));
                    cItem->setData(1, PropertiesDelegate::TypeRole, PropertiesDelegate::FontCapitalization);
                    cItem->setData(1, PropertiesDelegate::CallbackRole, callback.toVariant());
                    fontChildren.append(cItem);

                    callback = PropertiesDelegate::makeCallback(&PropertiesController::onFontSizePropertyEdit,
                                                                     this, fontItem, QStringLiteral("font.pointSize"));
                    auto ptItem = m_propertiesPane->propertiesTree()->delegate()->createItem();
                    ptItem->setText(1, QString());
                    ptItem->setText(0, QStringLiteral("pointSize"));
                    ptItem->setData(0, PropertiesDelegate::ModificationRole, ptChanged);
                    ptItem->setData(1, PropertiesDelegate::InitialValueRole, font.pointSize() < 0 ? 0 : font.pointSize());
                    ptItem->setData(1, PropertiesDelegate::TypeRole, PropertiesDelegate::FontSize);
                    ptItem->setData(1, PropertiesDelegate::CallbackRole, callback.toVariant());
                    fontChildren.append(ptItem);

                    callback = PropertiesDelegate::makeCallback(&PropertiesController::onFontSizePropertyEdit,
                                                                this, fontItem, QStringLiteral("font.pixelSize"));
                    auto pxItem = m_propertiesPane->propertiesTree()->delegate()->createItem();
                    pxItem->setText(1, QString());
                    pxItem->setText(0, QStringLiteral("pixelSize"));
                    pxItem->setData(0, PropertiesDelegate::ModificationRole, piChanged);
                    pxItem->setData(1, PropertiesDelegate::InitialValueRole, font.pixelSize() < 0 ? 0 : font.pixelSize());
                    pxItem->setData(1, PropertiesDelegate::TypeRole, PropertiesDelegate::FontSize);
                    pxItem->setData(1, PropertiesDelegate::CallbackRole, callback.toVariant());
                    fontChildren.append(pxItem);

                    callback = PropertiesDelegate::makeCallback(&PropertiesController::onBoolPropertyEdit, this, QStringLiteral("font.bold"));
                    auto bItem = m_propertiesPane->propertiesTree()->delegate()->createItem();
                    bItem->setText(1, QString());
                    bItem->setText(0, QStringLiteral("bold"));
                    bItem->setData(0, PropertiesDelegate::ModificationRole, bChanged);
                    bItem->setData(1, PropertiesDelegate::InitialValueRole, font.bold());
                    bItem->setData(1, PropertiesDelegate::TypeRole, PropertiesDelegate::Bool);
                    bItem->setData(1, PropertiesDelegate::CallbackRole, callback.toVariant());
                    fontChildren.append(bItem);

                    callback = PropertiesDelegate::makeCallback(&PropertiesController::onBoolPropertyEdit, this, QStringLiteral("font.italic"));
                    auto iItem = m_propertiesPane->propertiesTree()->delegate()->createItem();
                    iItem->setText(1, QString());
                    iItem->setText(0, QStringLiteral("italic"));
                    iItem->setData(0, PropertiesDelegate::ModificationRole, iChanged);
                    iItem->setData(1, PropertiesDelegate::InitialValueRole, font.italic());
                    iItem->setData(1, PropertiesDelegate::TypeRole, PropertiesDelegate::Bool);
                    iItem->setData(1, PropertiesDelegate::CallbackRole, callback.toVariant());
                    fontChildren.append(iItem);

                    callback = PropertiesDelegate::makeCallback(&PropertiesController::onBoolPropertyEdit, this, QStringLiteral("font.underline"));
                    auto uItem = m_propertiesPane->propertiesTree()->delegate()->createItem();
                    uItem->setText(1, QString());
                    uItem->setText(0, QStringLiteral("underline"));
                    uItem->setData(0, PropertiesDelegate::ModificationRole, uChanged);
                    uItem->setData(1, PropertiesDelegate::InitialValueRole, font.underline());
                    uItem->setData(1, PropertiesDelegate::TypeRole, PropertiesDelegate::Bool);
                    uItem->setData(1, PropertiesDelegate::CallbackRole, callback.toVariant());
                    fontChildren.append(uItem);

                    callback = PropertiesDelegate::makeCallback(&PropertiesController::onBoolPropertyEdit, this, QStringLiteral("font.overline"));
                    auto oItem = m_propertiesPane->propertiesTree()->delegate()->createItem();
                    oItem->setText(1, QString());
                    oItem->setText(0, QStringLiteral("overline"));
                    oItem->setData(0, PropertiesDelegate::ModificationRole, oChanged);
                    oItem->setData(1, PropertiesDelegate::InitialValueRole, font.overline());
                    oItem->setData(1, PropertiesDelegate::TypeRole, PropertiesDelegate::Bool);
                    oItem->setData(1, PropertiesDelegate::CallbackRole, callback.toVariant());
                    fontChildren.append(oItem);

                    callback = PropertiesDelegate::makeCallback(&PropertiesController::onBoolPropertyEdit, this, QStringLiteral("font.strikeout"));
                    auto sItem = m_propertiesPane->propertiesTree()->delegate()->createItem();
                    sItem->setText(1, QString());
                    sItem->setText(0, QStringLiteral("strikeout"));
                    sItem->setData(0, PropertiesDelegate::ModificationRole, sChanged);
                    sItem->setData(1, PropertiesDelegate::InitialValueRole, font.strikeOut());
                    sItem->setData(1, PropertiesDelegate::TypeRole, PropertiesDelegate::Bool);
                    sItem->setData(1, PropertiesDelegate::CallbackRole, callback.toVariant());
                    fontChildren.append(sItem);

                    callback = PropertiesDelegate::makeCallback(&PropertiesController::onBoolPropertyEdit, this, QStringLiteral("font.kerning"));
                    auto kItem = m_propertiesPane->propertiesTree()->delegate()->createItem();
                    kItem->setText(1, QString());
                    kItem->setText(0, QStringLiteral("kerning"));
                    kItem->setData(0, PropertiesDelegate::ModificationRole, kChanged);
                    kItem->setData(1, PropertiesDelegate::InitialValueRole, font.kerning());
                    kItem->setData(1, PropertiesDelegate::TypeRole, PropertiesDelegate::Bool);
                    kItem->setData(1, PropertiesDelegate::CallbackRole, callback.toVariant());
                    fontChildren.append(kItem);

                    callback = PropertiesDelegate::makeCallback(&PropertiesController::onBoolPropertyEdit, this, QStringLiteral("font.preferShaping"));
                    auto prItem = m_propertiesPane->propertiesTree()->delegate()->createItem();
                    prItem->setText(1, QString());
                    prItem->setText(0, QStringLiteral("preferShaping"));
                    prItem->setData(0, PropertiesDelegate::ModificationRole, prChanged);
                    prItem->setData(1, PropertiesDelegate::InitialValueRole, !(font.styleStrategy() & QFont::PreferNoShaping));
                    prItem->setData(1, PropertiesDelegate::TypeRole, PropertiesDelegate::Bool);
                    prItem->setData(1, PropertiesDelegate::CallbackRole, callback.toVariant());
                    fontChildren.append(prItem);

                    fontItem->addChildren(fontChildren);
                    for (auto i : fontChildren)
                        m_propertiesPane->propertiesTree()->openPersistentEditor(i, 1);
                    m_propertiesPane->propertiesTree()->expandItem(fontItem);
                    break;
                }

                case QVariant::Color: {
                    auto callback = PropertiesDelegate::makeCallback(&PropertiesController::onColorPropertyEdit,
                                                                     this, propertyName);
                    const QColor& color = propertyValue.value<QColor>();
                    auto item = m_propertiesPane->propertiesTree()->delegate()->createItem();
                    item->setText(1, QString());
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
                    item->setText(1, QString());
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
                    item->setText(1, QString());
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
                    item->setText(1, QString());
                    item->setText(0, propertyName);
                    item->setData(0, PropertiesDelegate::ModificationRole, ParserUtils::exists(selectedControl->dir(), propertyName));
                    item->setData(1, PropertiesDelegate::InitialValueRole, displayText);
                    item->setData(1, PropertiesDelegate::TypeRole, PropertiesDelegate::String);
                    item->setData(1, PropertiesDelegate::CallbackRole, callback.toVariant());
                    children.append(item);
                    break;
                }

                case QVariant::Double: {
                    if (isXProperty(propertyName)) {
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

                        auto geometryItem = new QTreeWidgetItem;
                        geometryItem->setText(0, "geometry");
                        geometryItem->setText(1, geometryText);
                        geometryItem->setData(0, PropertiesDelegate::ModificationRole, geometryChanged);
                        classItem->addChild(geometryItem);

                        auto xItem = new QTreeWidgetItem;
                        xItem->setText(0, "x");
                        xItem->setData(0, PropertiesDelegate::ModificationRole, xChanged);
                        geometryItem->addChild(xItem);
                        m_propertiesPane->propertiesTree()->setItemWidget(
                                    xItem, 1, createNumberHandlerWidget("x", geometry.x(), selectedControl, false));

                        auto yItem = new QTreeWidgetItem;
                        yItem->setText(0, "y");
                        yItem->setData(0, PropertiesDelegate::ModificationRole, yChanged);
                        geometryItem->addChild(yItem);
                        m_propertiesPane->propertiesTree()->setItemWidget(
                                    yItem, 1, createNumberHandlerWidget("y", geometry.y(), selectedControl, false));

                        auto wItem = new QTreeWidgetItem;
                        wItem->setText(0, "width");
                        wItem->setData(0, PropertiesDelegate::ModificationRole, wChanged);
                        geometryItem->addChild(wItem);
                        m_propertiesPane->propertiesTree()->setItemWidget(
                                    wItem, 1, createNumberHandlerWidget("width", geometry.width(), selectedControl, false));

                        auto hItem = new QTreeWidgetItem;
                        hItem->setText(0, "height");
                        hItem->setData(0, PropertiesDelegate::ModificationRole, hChanged);
                        geometryItem->addChild(hItem);
                        m_propertiesPane->propertiesTree()->setItemWidget(
                                    hItem, 1, createNumberHandlerWidget("height", geometry.height(), selectedControl, false));

                        m_propertiesPane->propertiesTree()->expandItem(geometryItem);
                    } else {
                        if (isGeometryProperty(propertyName))
                            break;

                        double number = propertyValue.value<double>();
                        auto item = new QTreeWidgetItem;
                        item->setText(0, propertyName);
                        item->setData(0, PropertiesDelegate::ModificationRole, ParserUtils::exists(selectedControl->dir(), propertyName));
                        classItem->addChild(item);
                        m_propertiesPane->propertiesTree()->setItemWidget(item, 1,
                                                                          createNumberHandlerWidget(propertyName, number, selectedControl, false));
                    }
                    break;
                }

                case QVariant::Int: {
                    if (isXProperty(propertyName)) {
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

                        auto geometryItem = new QTreeWidgetItem;
                        geometryItem->setText(0, "geometry");
                        geometryItem->setText(1, geometryText);
                        geometryItem->setData(0, PropertiesDelegate::ModificationRole, geometryChanged);
                        classItem->addChild(geometryItem);

                        auto xItem = new QTreeWidgetItem;
                        xItem->setText(0, "x");
                        xItem->setData(0, PropertiesDelegate::ModificationRole, xChanged);
                        geometryItem->addChild(xItem);
                        m_propertiesPane->propertiesTree()->setItemWidget(
                                    xItem, 1, createNumberHandlerWidget("x", geometry.x(), selectedControl, true));

                        auto yItem = new QTreeWidgetItem;
                        yItem->setText(0, "y");
                        yItem->setData(0, PropertiesDelegate::ModificationRole, yChanged);
                        geometryItem->addChild(yItem);
                        m_propertiesPane->propertiesTree()->setItemWidget(
                                    yItem, 1, createNumberHandlerWidget("y", geometry.y(), selectedControl, true));

                        auto wItem = new QTreeWidgetItem;
                        wItem->setText(0, "width");
                        wItem->setData(0, PropertiesDelegate::ModificationRole, wChanged);
                        geometryItem->addChild(wItem);
                        m_propertiesPane->propertiesTree()->setItemWidget(
                                    wItem, 1, createNumberHandlerWidget("width", geometry.width(), selectedControl, true));

                        auto hItem = new QTreeWidgetItem;
                        hItem->setText(0, "height");
                        hItem->setData(0, PropertiesDelegate::ModificationRole, hChanged);
                        geometryItem->addChild(hItem);
                        m_propertiesPane->propertiesTree()->setItemWidget(
                                    hItem, 1, createNumberHandlerWidget("height", geometry.height(), selectedControl, true));

                        m_propertiesPane->propertiesTree()->expandItem(geometryItem);
                    } else {
                        if (isGeometryProperty(propertyName))
                            break;

                        int number = propertyValue.value<int>();
                        auto item = new QTreeWidgetItem;
                        item->setText(0, propertyName);
                        item->setData(0, PropertiesDelegate::ModificationRole, ParserUtils::exists(selectedControl->dir(), propertyName));
                        classItem->addChild(item);
                        m_propertiesPane->propertiesTree()->setItemWidget(item, 1,
                                                                          createNumberHandlerWidget(propertyName, number, selectedControl, true));
                    }
                    break;
                }

                default:
                    break;
                }
            }

            for (const Enum& _enum : enumList) {
                const QString& propertyName = _enum.name;
                QString value = _enum.value;
                if (selectedControl->window() && propertyName == "visibility") {
                    value = "AutomaticVisibility";
                    const QString& visibility = ParserUtils::property(selectedControl->dir(), propertyName);
                    if (!visibility.isEmpty()) {
                        for (const QString& key : _enum.keys.keys()) {
                            if (visibility.contains(key))
                                value = key;
                        }
                    }
                }
                auto callback = PropertiesDelegate::makeCallback(&PropertiesController::onEnumPropertyEdit,
                                                                 this, propertyName, _enum);
                auto item = m_propertiesPane->propertiesTree()->delegate()->createItem();
                item->setText(1, QString());
                item->setText(0, propertyName);
                item->setData(0, PropertiesDelegate::ModificationRole, ParserUtils::exists(selectedControl->dir(), propertyName));
                item->setData(1, PropertiesDelegate::ValuesRole, QVariant(_enum.keys.keys()));
                item->setData(1, PropertiesDelegate::InitialValueRole, value);
                item->setData(1, PropertiesDelegate::TypeRole, PropertiesDelegate::Enum);
                item->setData(1, PropertiesDelegate::CallbackRole, callback.toVariant());
                children.append(item);
            }

            classItem->addChildren(children);
            for (auto i : children)
                m_propertiesPane->propertiesTree()->openPersistentEditor(i, 1);
            m_propertiesPane->propertiesTree()->expandItem(classItem);
        }

        onSearchEditEditingFinish();

        m_propertiesPane->propertiesTree()->viewport()->setUpdatesEnabled(true);
        m_propertiesPane->propertiesTree()->setUpdatesEnabled(true);
        m_propertiesPane->setUpdatesEnabled(true);
    }
}

void PropertiesController::onControlZChange(Control* control)
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

void PropertiesController::onControlRenderInfoChange(Control* control, bool codeChanged)
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

void PropertiesController::onControlGeometryChange(const Control* control)
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

void PropertiesController::onControlIndexChange(Control* control)
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

void PropertiesController::onControlIdChange(Control* control, const QString& /*previousId*/)
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

void PropertiesController::onControlIndexEditingFinish()
{
    // NOTE: No need for previous value equality check, since this signal is only emitted
    // when the value is changed
    ControlPropertyManager::setIndex(control(), m_propertiesPane->indexEdit()->value(),
                                     ControlPropertyManager::SaveChanges |
                                     ControlPropertyManager::UpdateRenderer);
}

void PropertiesController::onFontSizePropertyEdit(QTreeWidgetItem* fontClassItem, const QString& propertyName, const QVariant& value)
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

void PropertiesController::onFontFamilyPropertyEdit(QTreeWidgetItem* fontClassItem, const QVariant& value)
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

void PropertiesController::onFontWeightPropertyEdit(const QMetaEnum& _enum, const QVariant& value)
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

void PropertiesController::onFontCapitalizationPropertyEdit(const QMetaEnum& _enum, const QVariant& value)
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

void PropertiesController::onUrlPropertyEdit(const QString& propertyName, const QVariant& value)
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

void PropertiesController::onColorPropertyEdit(const QString& propertyName, const QVariant& value)
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

void PropertiesController::onStringPropertyEdit(const QString& propertyName, const QVariant& value)
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

void PropertiesController::onEnumPropertyEdit(const QString& propertyName, const Enum& _enum, const QVariant& value)
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

void PropertiesController::onBoolPropertyEdit(const QString& propertyName, const QVariant& value)
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

Control* PropertiesController::control() const
{
    const QList<Control*>& selectedControls = m_designerScene->selectedControls();
    if (selectedControls.size() != 1)
        return nullptr;
    return selectedControls.first();
}
