#include <propertiespane.h>
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

namespace { enum { ROW_HEIGHT = 21 }; }

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

static void fixVisible(Control* control, const QString& propertyName, QCheckBox* checkBox)
{
    if (propertyName == "visible")
        checkBox->setChecked(control->visible());
}

static void fixVisibilityForWindow(Control* control, const QString& propertyName, QComboBox* comboBox)
{
    if (control->window() && propertyName == "visibility") {
        comboBox->setCurrentText("AutomaticVisibility");

        const QString& visibility = ParserUtils::property(control->dir(), propertyName);
        if (visibility.isEmpty())
            return;

        for (int i = 0; i < comboBox->count(); ++i) {
            if (visibility.contains(comboBox->itemText(i)))
                comboBox->setCurrentIndex(i);
        }
    }
}

static void fixFontItemText(QTreeWidgetItem* fontItem, const QFont& font, bool isPx)
{
    QTreeWidget* treeWidget = fontItem->treeWidget();
    Q_ASSERT(treeWidget);

    QString fontText = fontItem->text(1);
    if (isPx)
        fontText.replace(QRegExp(",.*"), ", " + QString::number(font.pixelSize()) + "px]");
    else
        fontText.replace(QRegExp(",.*"), ", " + QString::number(font.pointSize()) + "pt]");
    fontItem->setText(1, fontText);

    for (int i = 0; i < fontItem->childCount(); ++i) {
        QTreeWidgetItem* chilItem = fontItem->child(i);
        if (chilItem->text(0) == (isPx ? "pointSize" : "pixelSize")) {
            QSpinBox* spinBox = qobject_cast<QSpinBox*>(treeWidget->itemWidget(chilItem, 1));
            Q_ASSERT(spinBox);
            spinBox->blockSignals(true);
            spinBox->setValue(0);
            spinBox->blockSignals(false);
            break;
        }
    }
}

static QString urlToDisplayText(const QUrl& url, const QString& controlDir)
{
    QString displayText = url.toDisplayString();
    if (url.isLocalFile()) {
        displayText = url.toLocalFile().remove(
                    SaveUtils::toControlThisDir(controlDir) + '/');
    }
    return displayText;
}

static QWidget* createStringHandlerWidget(const QString& propertyName, const QString& text,
                                   Control* control)
{
    auto lineEdit = new QLineEdit;
    lineEdit->setStyleSheet("QLineEdit { border: none; background: transparent; }");
    lineEdit->setAttribute(Qt::WA_MacShowFocusRect, false);
    lineEdit->setText(text);
    lineEdit->setFocusPolicy(Qt::StrongFocus);
    lineEdit->setSizePolicy(QSizePolicy::Ignored, lineEdit->sizePolicy().verticalPolicy());
    lineEdit->setMinimumWidth(1);

    QObject::connect(lineEdit, &QLineEdit::editingFinished, [=]
    {
        const QString& previousValue = UtilityFunctions::getProperty(propertyName, control->properties()).value<QString>();

        if (previousValue == lineEdit->text())
            return;

        ControlPropertyManager::setProperty(control,
                                            propertyName, UtilityFunctions::stringify(lineEdit->text()),
                                            lineEdit->text(),
                                            ControlPropertyManager::SaveChanges
                                            | ControlPropertyManager::UpdateRenderer);
    });

    return lineEdit;
}

static QWidget* createUrlHandlerWidget(const QString& propertyName, const QString& url,
                                Control* control)
{
    auto lineEdit = new QLineEdit;
    lineEdit->setStyleSheet("QLineEdit { border: none; background: transparent; }");
    lineEdit->setAttribute(Qt::WA_MacShowFocusRect, false);
    lineEdit->setText(url);
    lineEdit->setFocusPolicy(Qt::StrongFocus);
    lineEdit->setSizePolicy(QSizePolicy::Ignored, lineEdit->sizePolicy().verticalPolicy());
    lineEdit->setMinimumWidth(1);

    QObject::connect(lineEdit, &QLineEdit::editingFinished, [=]
    {
        // TODO: Clear whitespaces in the url
        const QUrl& url = QUrl::fromUserInput(lineEdit->text(),
                                              SaveUtils::toControlThisDir(control->dir()),
                                              QUrl::AssumeLocalFile);
        const QString& displayText = urlToDisplayText(url, control->dir());
        const QUrl& previousUrl = UtilityFunctions::getProperty(propertyName, control->properties()).value<QUrl>();

        if (url == previousUrl)
            return;

        ControlPropertyManager::setProperty(control, propertyName, UtilityFunctions::stringify(displayText), url,
                                            ControlPropertyManager::SaveChanges
                                            | ControlPropertyManager::UpdateRenderer);
    });

    return lineEdit;
}

static QWidget* createEnumHandlerWidget(const Enum& enumm, Control* control)
{
    auto comboBox = new QComboBox;
    TransparentStyle::attach(comboBox);
    comboBox->setAttribute(Qt::WA_MacShowFocusRect, false);
    comboBox->addItems(enumm.keys.keys());
    comboBox->setCurrentText(enumm.value);
    comboBox->setCursor(Qt::PointingHandCursor);
    comboBox->setFocusPolicy(Qt::ClickFocus);
    comboBox->setSizePolicy(QSizePolicy::Ignored, comboBox->sizePolicy().verticalPolicy());
    comboBox->setMinimumWidth(1);
    fixVisibilityForWindow(control, enumm.name, comboBox);

    QObject::connect(comboBox, qOverload<int>(&QComboBox::activated), [=]
    {
        const QString& previousValue = UtilityFunctions::getEnum(enumm.name, control->properties()).value;

        if (previousValue == comboBox->currentText())
            return;

        QFile file(SaveUtils::toControlMainQmlFile(control->dir()));
        if (!file.open(QFile::ReadOnly)) {
            qWarning("createEnumHandlerWidget: Cannot open control main qml file");
            return;
        }

        QString fixedScope = enumm.scope;
        if (control->window() && fixedScope == "Window") {
            if (!file.readAll().contains("import QtQuick.Window"))
                fixedScope = "ApplicationWindow";
        }
        file.close();

        ControlPropertyManager::setProperty(control,
                                            enumm.name, fixedScope + "." + comboBox->currentText(),
                                            enumm.keys.value(comboBox->currentText()),
                                            ControlPropertyManager::SaveChanges
                                            | ControlPropertyManager::UpdateRenderer);
    });

    return comboBox;
}

static QWidget* createBoolHandlerWidget(const QString& propertyName, bool checked, Control* control)
{
    auto checkBox = new QCheckBox;
    checkBox->setAttribute(Qt::WA_MacShowFocusRect, false);
    checkBox->setCursor(Qt::PointingHandCursor);
    checkBox->setChecked(checked);
    checkBox->setFocusPolicy(Qt::ClickFocus);
    checkBox->setMinimumWidth(1);
    fixVisible(control, propertyName, checkBox);

    QObject::connect(checkBox, qOverload<bool>(&QCheckBox::clicked), [=]
    {
        // NOTE: No need for previous value equality check, since this signal is only emitted
        // when the value is changed/toggled
        ControlPropertyManager::setProperty(control,
                                            propertyName, checkBox->isChecked() ? "true" : "false",
                                            checkBox->isChecked(),
                                            ControlPropertyManager::SaveChanges
                                            | ControlPropertyManager::UpdateRenderer);
    });

    auto widget = new QWidget;
    widget->setMinimumWidth(1);
    widget->setAttribute(Qt::WA_MacShowFocusRect, false);
    widget->setFocusPolicy(Qt::ClickFocus);
    widget->setSizePolicy(QSizePolicy::Ignored, widget->sizePolicy().verticalPolicy());
    auto layout = new QHBoxLayout(widget);
    layout->addWidget(checkBox);
    layout->addStretch();
    layout->setSpacing(0);
    layout->setContentsMargins(2, 0, 0, 0);
    return widget;
}

static QWidget* createColorHandlerWidget(const QString& propertyName, const QColor& color,
                                  Control* control)
{
    auto toolButton = new QToolButton;
    toolButton->setStyleSheet("QToolButton { border: none; background: transparent; }");
    toolButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    toolButton->setText(color.name(QColor::HexArgb));
    toolButton->setIcon(QIcon(PaintUtils::renderPropertyColorPixmap({12, 12}, color, {Qt::black}, toolButton->devicePixelRatioF())));
    toolButton->setAttribute(Qt::WA_MacShowFocusRect, false);
    toolButton->setIconSize({12, 12});
    toolButton->setCursor(Qt::PointingHandCursor);
    toolButton->setFocusPolicy(Qt::ClickFocus);
    toolButton->setSizePolicy(QSizePolicy::Ignored, toolButton->sizePolicy().verticalPolicy());
    toolButton->setMinimumWidth(1);

    QObject::connect(toolButton, &QCheckBox::clicked, [=]
    {
        const QColor& previousColor = UtilityFunctions::getProperty(propertyName, control->properties()).value<QColor>();

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
        ControlPropertyManager::setProperty(control, propertyName,
                                            UtilityFunctions::stringify(color.name(QColor::HexArgb)), color,
                                            ControlPropertyManager::SaveChanges
                                            | ControlPropertyManager::UpdateRenderer);
    });

    return toolButton;
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

static QWidget* createFontFamilyHandlerWidget(const QString& family, Control* control, QTreeWidgetItem* fontItem)
{
    auto comboBox = new QComboBox;
    TransparentStyle::attach(comboBox);
    comboBox->setAttribute(Qt::WA_MacShowFocusRect, false);
    comboBox->addItems(QFontDatabase().families());
    comboBox->setCurrentText(family);
    comboBox->setCursor(Qt::PointingHandCursor);
    comboBox->setFocusPolicy(Qt::ClickFocus);
    comboBox->setSizePolicy(QSizePolicy::Ignored, comboBox->sizePolicy().verticalPolicy());
    comboBox->setMinimumWidth(1);

    QObject::connect(comboBox, qOverload<int>(&QComboBox::activated), [=]
    {
        const QFont& font = UtilityFunctions::getProperty("font", control->properties()).value<QFont>();

        if (comboBox->currentText() == QFontInfo(font).family())
            return;

        const QString& previousFamily = QFontInfo(font).family();
        QString fontText = fontItem->text(1);
        fontText.replace(previousFamily, comboBox->currentText());
        fontItem->setText(1, fontText);

        ControlPropertyManager::setProperty(control, "font.family", UtilityFunctions::stringify(comboBox->currentText()),
                                            comboBox->currentText(), ControlPropertyManager::SaveChanges
                                            | ControlPropertyManager::UpdateRenderer);
    });

    return comboBox;
}

static QWidget* createFontWeightHandlerWidget(int weight, Control* control)
{
    auto comboBox = new QComboBox;
    TransparentStyle::attach(comboBox);
    comboBox->setAttribute(Qt::WA_MacShowFocusRect, false);
    comboBox->setCursor(Qt::PointingHandCursor);
    comboBox->setFocusPolicy(Qt::ClickFocus);
    comboBox->setSizePolicy(QSizePolicy::Ignored, comboBox->sizePolicy().verticalPolicy());
    comboBox->setMinimumWidth(1);

    QMetaEnum weightEnum = QMetaEnum::fromType<QFont::Weight>();
    for (int i = weightEnum.keyCount(); i--;) { // Necessary somehow
        if (QString::fromUtf8(weightEnum.key(i)).contains(QRegularExpression("[^\\r\\n\\t\\f\\v ]")))
            comboBox->addItem(weightEnum.key(i));
    }

    comboBox->setCurrentText(QMetaEnum::fromType<QFont::Weight>().valueToKey(weight));

    QObject::connect(comboBox, qOverload<int>(&QComboBox::activated), [=]
    {
        int weightValue = weightEnum.keyToValue(comboBox->currentText().toUtf8().constData());
        const QFont& font = UtilityFunctions::getProperty("font", control->properties()).value<QFont>();

        if (weightValue == font.weight())
            return;

        ControlPropertyManager::setProperty(control, "font.weight",
                                            "Font." + comboBox->currentText(), weightValue,
                                            ControlPropertyManager::SaveChanges
                                            | ControlPropertyManager::UpdateRenderer);
    });

    return comboBox;
}

static QWidget* createFontCapitalizationHandlerWidget(QFont::Capitalization capitalization, Control* control)
{
    auto comboBox = new QComboBox;
    TransparentStyle::attach(comboBox);
    comboBox->setAttribute(Qt::WA_MacShowFocusRect, false);
    comboBox->setCursor(Qt::PointingHandCursor);
    comboBox->setFocusPolicy(Qt::ClickFocus);
    comboBox->setSizePolicy(QSizePolicy::Ignored, comboBox->sizePolicy().verticalPolicy());
    comboBox->setMinimumWidth(1);

    QMetaEnum capitalizationEnum = QMetaEnum::fromType<QFont::Capitalization>();
    for (int i = capitalizationEnum.keyCount(); i--;) { // Necessary somehow
        if (QString::fromUtf8(capitalizationEnum.key(i)).contains(QRegularExpression("[^\\r\\n\\t\\f\\v ]")))
            comboBox->addItem(capitalizationEnum.key(i));
    }

    comboBox->setCurrentText(QMetaEnum::fromType<QFont::Capitalization>().valueToKey(capitalization));

    QObject::connect(comboBox, qOverload<int>(&QComboBox::activated), [=]
    {
        int capitalizationValue = capitalizationEnum.keyToValue(comboBox->currentText().toUtf8().constData());
        const QFont& font = UtilityFunctions::getProperty("font", control->properties()).value<QFont>();

        if (capitalizationValue == font.capitalization())
            return;

        ControlPropertyManager::setProperty(control, "font.capitalization",
                                            "Font." + comboBox->currentText(),
                                            QFont::Capitalization(capitalizationValue),
                                            ControlPropertyManager::SaveChanges
                                            | ControlPropertyManager::UpdateRenderer);
    });

    return comboBox;
}

static QWidget* createFontSizeHandlerWidget(const QString& propertyName, int size, Control* control, QTreeWidgetItem* fontItem)
{
    QSpinBox* spinBox = new QSpinBox;
    TransparentStyle::attach(spinBox);
    spinBox->setCursor(Qt::PointingHandCursor);
    spinBox->setFocusPolicy(Qt::StrongFocus);
    spinBox->setMinimum(0);
    spinBox->setMaximum(72);
    spinBox->setValue(size < 0 ? 0 : size);
    spinBox->setSizePolicy(QSizePolicy::Ignored, spinBox->sizePolicy().verticalPolicy());
    spinBox->setMinimumWidth(1);
    UtilityFunctions::disableWheelEvent(spinBox);

    QObject::connect(spinBox, qOverload<int>(&QSpinBox::valueChanged), [=]
    {
        bool isPx = propertyName == "pixelSize" ? true : false;
        QFont font = UtilityFunctions::getProperty("font", control->properties()).value<QFont>();

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

        fixFontItemText(fontItem, font, isPx);

        // FIXME: Remove related property instead of setting its value to 0
        ControlPropertyManager::setProperty(control, QString::fromUtf8("font.") +
                                            (isPx ? "pointSize" : "pixelSize"),
                                            QString::number(0), 0,
                                            ControlPropertyManager::SaveChanges);
        ControlPropertyManager::setProperty(control, "font." + propertyName,
                                            QString::number(spinBox->value()), spinBox->value(),
                                            ControlPropertyManager::SaveChanges);
        ControlPropertyManager::setProperty(control, "font", QString(), font,
                                            ControlPropertyManager::UpdateRenderer);
    });

    return spinBox;
}

static void createAndAddGeometryPropertiesBlock(QTreeWidgetItem* classItem,
                                         const QVector<PropertyNode>& properties,
                                         Control* control, int integer)
{
    QTreeWidget* treeWidget = classItem->treeWidget();
    Q_ASSERT(treeWidget);

    const QRectF& geometry = UtilityFunctions::getGeometryFromProperties(properties);

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

    auto geometryItem = new QTreeWidgetItem;
    geometryItem->setText(0, "geometry");
    geometryItem->setText(1, geometryText);
    geometryItem->setData(0, Qt::DecorationRole, geometryChanged);
    classItem->addChild(geometryItem);

    auto xItem = new QTreeWidgetItem;
    xItem->setText(0, "x");
    xItem->setData(0, Qt::DecorationRole, xChanged);
    geometryItem->addChild(xItem);
    treeWidget->setItemWidget(
                xItem, 1, createNumberHandlerWidget("x", geometry.x(), control, integer));

    auto yItem = new QTreeWidgetItem;
    yItem->setText(0, "y");
    yItem->setData(0, Qt::DecorationRole, yChanged);
    geometryItem->addChild(yItem);
    treeWidget->setItemWidget(
                yItem, 1, createNumberHandlerWidget("y", geometry.y(), control, integer));

    auto wItem = new QTreeWidgetItem;
    wItem->setText(0, "width");
    wItem->setData(0, Qt::DecorationRole, wChanged);
    geometryItem->addChild(wItem);
    treeWidget->setItemWidget(
                wItem, 1, createNumberHandlerWidget("width", geometry.width(), control, integer));

    auto hItem = new QTreeWidgetItem;
    hItem->setText(0, "height");
    hItem->setData(0, Qt::DecorationRole, hChanged);
    geometryItem->addChild(hItem);
    treeWidget->setItemWidget(
                hItem, 1, createNumberHandlerWidget("height", geometry.height(), control, integer));

    treeWidget->expandItem(geometryItem);
}

static void createAndAddFontPropertiesBlock(QTreeWidgetItem* classItem, const QFont& font, Control* control)
{
    QTreeWidget* treeWidget = classItem->treeWidget();
    Q_ASSERT(treeWidget);

    const bool isPx = font.pixelSize() > 0 ? true : false;
    const QString& fontText = QString::fromUtf8("[%1, %2%3]")
            .arg(QFontInfo(font).family())
            .arg(isPx ? font.pixelSize() : font.pointSize())
            .arg(isPx ? "px" : "pt");

    const bool fChanged    = ParserUtils::exists(control->dir(), "font.family");
    const bool bChanged    = ParserUtils::exists(control->dir(), "font.bold");
    const bool iChanged    = ParserUtils::exists(control->dir(), "font.italic");
    const bool uChanged    = ParserUtils::exists(control->dir(), "font.underline");
    const bool poChanged   = ParserUtils::exists(control->dir(), "font.pointSize");
    const bool piChanged   = ParserUtils::exists(control->dir(), "font.pixelSize");
    const bool wChanged    = ParserUtils::exists(control->dir(), "font.weight");
    const bool oChanged    = ParserUtils::exists(control->dir(), "font.overline");
    const bool sChanged    = ParserUtils::exists(control->dir(), "font.strikeout");
    const bool cChanged    = ParserUtils::exists(control->dir(), "font.capitalization");
    const bool kChanged    = ParserUtils::exists(control->dir(), "font.kerning");
    const bool prChanged   = ParserUtils::exists(control->dir(), "font.preferShaping");
    const bool fontChanged = fChanged || bChanged || iChanged || uChanged || poChanged || piChanged
            || wChanged || oChanged || sChanged || cChanged || kChanged || prChanged;

    auto fontItem = new QTreeWidgetItem;
    fontItem->setText(0, "font");
    fontItem->setText(1, fontText);
    fontItem->setData(0, Qt::DecorationRole, fontChanged);
    classItem->addChild(fontItem);

    auto poItem = new QTreeWidgetItem;
    poItem->setText(0, "pointSize");
    poItem->setData(0, Qt::DecorationRole, poChanged);
    fontItem->addChild(poItem);
    treeWidget->setItemWidget(
                poItem, 1, createFontSizeHandlerWidget("pointSize", font.pointSize(), control, fontItem));

    auto pxItem = new QTreeWidgetItem;
    pxItem->setText(0, "pixelSize");
    pxItem->setData(0, Qt::DecorationRole, piChanged);
    fontItem->addChild(pxItem);
    treeWidget->setItemWidget(
                pxItem, 1, createFontSizeHandlerWidget("pixelSize", font.pixelSize(), control, fontItem));

    auto fItem = new QTreeWidgetItem;
    fItem->setText(0, "family");
    fItem->setData(0, Qt::DecorationRole, fChanged);
    fontItem->addChild(fItem);
    treeWidget->setItemWidget(
                fItem, 1, createFontFamilyHandlerWidget(QFontInfo(font).family(), control, fontItem));

    auto wItem = new QTreeWidgetItem;
    wItem->setText(0, "weight");
    wItem->setData(0, Qt::DecorationRole, wChanged);
    fontItem->addChild(wItem);
    treeWidget->setItemWidget(wItem, 1, createFontWeightHandlerWidget(font.weight(), control));

    auto cItem = new QTreeWidgetItem;
    cItem->setText(0, "capitalization");
    cItem->setData(0, Qt::DecorationRole, cChanged);
    fontItem->addChild(cItem);
    treeWidget->setItemWidget(cItem, 1,
                              createFontCapitalizationHandlerWidget(font.capitalization(), control));

    auto bItem = new QTreeWidgetItem;
    bItem->setText(0, "bold");
    bItem->setData(0, Qt::DecorationRole, bChanged);
    fontItem->addChild(bItem);
    treeWidget->setItemWidget(bItem, 1, createBoolHandlerWidget("font.bold", font.bold(), control));

    auto iItem = new QTreeWidgetItem;
    iItem->setText(0, "italic");
    iItem->setData(0, Qt::DecorationRole, iChanged);
    fontItem->addChild(iItem);
    treeWidget->setItemWidget(
                iItem, 1, createBoolHandlerWidget("font.italic", font.italic(), control));

    auto uItem = new QTreeWidgetItem;
    uItem->setText(0, "underline");
    uItem->setData(0, Qt::DecorationRole, uChanged);
    fontItem->addChild(uItem);
    treeWidget->setItemWidget(
                uItem, 1, createBoolHandlerWidget("font.underline", font.underline(), control));

    auto oItem = new QTreeWidgetItem;
    oItem->setText(0, "overline");
    oItem->setData(0, Qt::DecorationRole, oChanged);
    fontItem->addChild(oItem);
    treeWidget->setItemWidget(
                oItem, 1, createBoolHandlerWidget("font.overline", font.overline(), control));

    auto sItem = new QTreeWidgetItem;
    sItem->setText(0, "strikeout");
    sItem->setData(0, Qt::DecorationRole, sChanged);
    fontItem->addChild(sItem);
    treeWidget->setItemWidget(
                sItem, 1, createBoolHandlerWidget("font.strikeout", font.strikeOut(), control));

    auto kItem = new QTreeWidgetItem;
    kItem->setText(0, "kerning");
    kItem->setData(0, Qt::DecorationRole, kChanged);
    fontItem->addChild(kItem);
    treeWidget->setItemWidget(
                kItem, 1, createBoolHandlerWidget("font.kerning", font.kerning(), control));

    auto prItem = new QTreeWidgetItem;
    prItem->setText(0, "preferShaping");
    prItem->setData(0, Qt::DecorationRole, prChanged);
    fontItem->addChild(prItem);
    treeWidget->setItemWidget(prItem, 1, createBoolHandlerWidget(
                                  "font.preferShaping",
                                  !(font.styleStrategy() & QFont::PreferNoShaping), control));

    treeWidget->expandItem(fontItem);
}

PropertiesPane::PropertiesPane(DesignerScene* designerScene, QWidget* parent) : QTreeWidget(parent)
  , m_designerScene(designerScene)
  , m_searchEdit(new LineEdit(this))
  , m_typeItem(new QTreeWidgetItem)
  , m_uidItem(new QTreeWidgetItem)
  , m_idItem(new QTreeWidgetItem)
  , m_indexItem(new QTreeWidgetItem)
  , m_idEdit(new QLineEdit(this))
  , m_indexEdit(new QSpinBox(this))
{
    m_idEdit->setValidator(new QRegExpValidator(QRegExp("([a-z_][a-zA-Z0-9_]+)?"), m_idEdit));
    m_idEdit->setStyleSheet("QLineEdit { border: none; background: transparent; }");
    m_idEdit->setAttribute(Qt::WA_MacShowFocusRect, false);
    m_idEdit->setFocusPolicy(Qt::StrongFocus);
    m_idEdit->setSizePolicy(QSizePolicy::Ignored, m_idEdit->sizePolicy().verticalPolicy());
    m_idEdit->setMinimumWidth(1);

    TransparentStyle::attach(m_indexEdit);
    m_indexEdit->setCursor(Qt::PointingHandCursor);
    m_indexEdit->setFocusPolicy(Qt::StrongFocus);
    m_indexEdit->setSizePolicy(QSizePolicy::Ignored, m_indexEdit->sizePolicy().verticalPolicy());
    m_indexEdit->setMinimumWidth(1);
    m_indexEdit->setMaximum(std::numeric_limits<int>::max());
    m_indexEdit->setMinimum(std::numeric_limits<int>::lowest());
    UtilityFunctions::disableWheelEvent(m_indexEdit);

    m_typeItem->setText(0, tr("Type"));
    m_typeItem->setData(0, Qt::DecorationRole, false); // No 'property changed' indication
    addTopLevelItem(m_typeItem);

    m_uidItem->setText(0, "uid");
    m_uidItem->setData(0, Qt::DecorationRole, false); // No 'property changed' indication
    addTopLevelItem(m_uidItem);

    m_idItem->setText(0, "id");
    m_idItem->setData(0, Qt::DecorationRole, false); // No 'property changed' indication
    addTopLevelItem(m_idItem);
    setItemWidget(m_idItem, 1, m_idEdit);

    m_indexItem->setText(0, "index");
    m_indexItem->setData(0, Qt::DecorationRole, false); // No 'property changed' indication
    addTopLevelItem(m_indexItem);
    setItemWidget(m_indexItem, 1, m_indexEdit);

    m_searchEdit->addAction(QIcon(PaintUtils::renderOverlaidPixmap(":/images/search.svg", "#595959", m_searchEdit->devicePixelRatioF())),
                            QLineEdit::LeadingPosition);
    m_searchEdit->setPlaceholderText(tr("Search"));
    m_searchEdit->setClearButtonEnabled(true);
    // Since PropertiesPane (parent of the m_searchEdit) has
    // its own layout and we don't add m_searchEdit into it
    // QWidget::setVisible does not adjust the size. So we
    // must call it manually.
    m_searchEdit->adjustSize();
}

void PropertiesPane::filterList(const QString& filter)
{
    for (int i = 0; i < topLevelItemCount(); i++) {
        auto tli = topLevelItem(i);
        auto tlv = false;

        for (int j = 0; j < tli->childCount(); j++) {
            auto tci = tli->child(j);
            auto tcv = false;
            auto vv = tci->text(0).contains(filter, Qt::CaseInsensitive);

            for (int z = 0; z < tci->childCount(); z++) {
                auto tdi = tci->child(z);
                auto v = (filter.isEmpty() || vv) ? true :
                                                    tdi->text(0).contains(filter, Qt::CaseInsensitive);

                tdi->setHidden(!v);
                if (v)
                    tcv = v;
            }

            auto v = filter.isEmpty() ? true : (tci->childCount() > 0 ? tcv : vv);
            tci->setHidden(!v);
            if (v)
                tlv = v;
        }

        auto v = filter.isEmpty() ? true : tlv;
        tli->setHidden(!v);
    }
}

void PropertiesPane::drawBranches(QPainter* painter, const QRect& rect, const QModelIndex& index) const
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    const qreal width = 10;
    const QAbstractItemModel* model = index.model();
    const bool hasChild = itemFromIndex(index)->childCount();
    const bool isClassRow = !model->parent(index).isValid() && index.row() > 3; // FIXME: For Temporary "index" entry, should be 2 otherwise

    QRectF handleRect(0, 0, width, width);
    handleRect.moveCenter(rect.center());
    handleRect.moveRight(rect.right() - 0.5);

    QStyleOptionViewItem option;
    option.initFrom(this);
    option.rect = rect;

    fillBackground(painter, option, calculateVisibleRow(itemFromIndex(index)), isClassRow, false);

    // Draw handle
    if (hasChild) {
        QPen pen;
        pen.setWidthF(1.2);
        pen.setColor(isClassRow ? palette().highlightedText().color() : palette().text().color());
        painter->setPen(pen);
        painter->setBrush(Qt::NoBrush);
        painter->drawRoundedRect(handleRect, 0, 0);

        painter->drawLine(QPointF(handleRect.left() + 2.5, handleRect.center().y()),
                          QPointF(handleRect.right() - 2.5, handleRect.center().y()));

        if (!isExpanded(index)) {
            painter->drawLine(QPointF(handleRect.center().x(), handleRect.top() + 2.5),
                              QPointF(handleRect.center().x(), handleRect.bottom() - 2.5));
        }
    }

    painter->restore();
}

void PropertiesPane::paintEvent(QPaintEvent* e)
{
    QPainter painter(viewport());
    painter.fillRect(rect(), palette().base());
    painter.setClipping(true);

    QColor lineColor(palette().dark().color());
    lineColor.setAlpha(50);
    painter.setPen(lineColor);

    qreal rowCount = viewport()->height() / qreal(ROW_HEIGHT);
    const QList<Control*>& selectedControls = m_designerScene->selectedControls();
    for (int i = 0; i < rowCount; ++i) {
        painter.save();
        QRectF rect(0, i * ROW_HEIGHT, viewport()->width(), ROW_HEIGHT);
        QPainterPath path;
        path.addRect(rect);
        painter.setClipPath(path);

        if (i % 2) {
            painter.fillRect(rect, palette().alternateBase());
        } else if (topLevelItemCount() == 0) {
            if (i == int((rowCount - 1) / 2.0) || i == int((rowCount - 1)/ 2.0) + 1) {
                QString message;
                if (selectedControls.size() == 0)
                    message = tr("No controls selected");
                else if (selectedControls.size() == 1)
                    message = tr("Control has errors");
                else
                    message = tr("Multiple controls selected");

                QColor messageColor = selectedControls.size() == 1
                        ? palette().linkVisited().color()
                        : palette().dark().color();
                messageColor.setAlpha(180);

                painter.setPen(messageColor);
                painter.drawText(rect, Qt::AlignCenter, message);
                painter.setPen(lineColor);
            }
        }

        // Draw top and bottom lines
        painter.drawLine(rect.topLeft() + QPointF{0.5, 0.0}, rect.topRight() - QPointF{0.5, 0.0});
        painter.drawLine(rect.bottomLeft() + QPointF{0.5, 0.0}, rect.bottomRight() - QPointF{0.5, 0.0});
        painter.restore();
    }

    QTreeWidget::paintEvent(e);
}

QSize PropertiesPane::sizeHint() const
{
    return QSize{310, 530};
}

#include "propertiespane.moc"
