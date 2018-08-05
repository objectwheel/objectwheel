#include <propertiespane.h>
#include <focuslesslineedit.h>
#include <filemanager.h>
#include <saveutils.h>
#include <controlpropertymanager.h>
#include <designerscene.h>
#include <parserutils.h>
#include <transparentstyle.h>

#include <QStyledItemDelegate>
#include <QPainter>
#include <QHeaderView>
#include <QScrollBar>
#include <QCheckBox>
#include <QJSEngine>
#include <QToolButton>
#include <QColorDialog>
#include <QSpinBox>
#include <QFontDatabase>
#include <QMetaEnum>
#include <QComboBox>
#include <QHBoxLayout>
#include <QApplication>

namespace {

int g_verticalScrollBarPosition = 99999;
int g_horizontalScrollBarPosition = 99999;

class WheelDisabler : public QObject {
    Q_OBJECT
    bool eventFilter(QObject* o, QEvent* e) override {
        if (e->type() == QEvent::Wheel && qobject_cast<QAbstractSpinBox*>(o)) {
            e->ignore();
            return true;
        }
        return QObject::eventFilter(o, e);
    }
} g_wheelDisabler;

bool isXProperty(const QString& propertyName)
{
    return propertyName == "x";
}

bool isGeometryProperty(const QString& propertyName)
{
    return propertyName == "x"
            || propertyName == "y"
            || propertyName == "width"
            || propertyName == "height";
}

template<typename SpinBox>
void fixPosForForm(Control* control, const QString& propertyName, SpinBox spinBox)
{
    if (control->form()) {
        if ((propertyName == "x" || propertyName == "y")
                && !ParserUtils::exists(control->url(), propertyName)) {
            spinBox->setValue(0);
        }
    }
}

void fixVisibleForWindow(Control* control, const QString& propertyName, QCheckBox* checkBox)
{
    if (control->window() && propertyName == "visible")
        checkBox->setChecked(ParserUtils::property(control->url(), propertyName) == "true");
}

void fixVisibilityForWindow(Control* control, const QString& propertyName, QComboBox* comboBox)
{
    if (control->window() && propertyName == "visibility") {
        comboBox->setCurrentText("AutomaticVisibility");

        const QString& visibility = ParserUtils::property(control->url(), propertyName);
        if (visibility.isEmpty())
            return;

        for (int i = 0; i < comboBox->count(); ++i) {
            if (visibility.contains(comboBox->itemText(i)))
                comboBox->setCurrentIndex(i);
        }
    }
}

void fixFontItemText(QTreeWidgetItem* fontItem, const QFont& font, bool isPx)
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

void setPalette(QWidget* widget)
{
    QPalette palette(widget->palette());
    palette.setColor(QPalette::Text, "#403121");
    palette.setColor(QPalette::WindowText, "#403121");
    widget->setPalette(palette);
}

void fillBackground(QPainter* painter, const QRectF& rect, int row, bool classRow, bool verticalLine)
{
    painter->save();

    QPainterPath path;
    path.addRect(rect);
    painter->setClipPath(path);
    painter->setClipping(true);

    // Fill background
    if (classRow)
        painter->fillRect(rect, "#9D7650");
    else if (row % 2)
        painter->fillRect(rect, "#faf1e8");
    else
        painter->fillRect(rect, Qt::white);

    // Draw top and bottom lines
    painter->setPen("#408C6A48");
    painter->drawLine(rect.topLeft() + QPointF{0.5, 0.0}, rect.topRight() - QPointF{0.5, 0.0});
    painter->drawLine(rect.bottomLeft() + QPointF{0.5, 0.0}, rect.bottomRight() - QPointF{0.5, 0.0});

    // Draw vertical line
    if (verticalLine) {
        painter->drawLine(rect.topRight() + QPointF(-0.5, 0.5),
                          rect.bottomRight() + QPointF(-0.5, -0.5));
    }

    painter->restore();
}

QImage colorToImage(const QSize& layoutSize, const QColor& color)
{
    QImage image(layoutSize * qApp->devicePixelRatio(), QImage::Format_ARGB32_Premultiplied);
    image.setDevicePixelRatio(qApp->devicePixelRatio());
    image.fill(color);
    QPainter p(&image);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen("#403121");
    p.drawRect(QRectF{{},layoutSize}.adjusted(0.5, 0.5, -0.5, -0.5));
    p.end();
    return image;
}

QString urlToDisplayText(const QUrl& url, const QString& controlDir)
{
    QString displayText = url.toDisplayString();
    if (url.isLocalFile()) {
        displayText = url.toLocalFile().remove(
                    SaveUtils::toThisDir(controlDir) + separator());
    }
    return displayText;
}

QString stringify(const QString& text)
{
    QJSEngine engine;
    engine.globalObject().setProperty("text", text);
    return engine.evaluate("JSON.stringify(text)").toString();
}

QRectF getGeometryFromProperties(const QList<PropertyNode>& properties)
{
    QRectF geometry;
    for (const PropertyNode& propertyNode : properties) {
        for (const QString& propertyName : propertyNode.properties.keys()) {
            if (propertyName == "x")
                geometry.moveLeft(propertyNode.properties.value(propertyName).toReal());
            else if (propertyName == "y")
                geometry.moveTop(propertyNode.properties.value(propertyName).toReal());
            else if (propertyName == "width")
                geometry.setWidth(propertyNode.properties.value(propertyName).toReal());
            else if (propertyName == "height")
                geometry.setHeight(propertyNode.properties.value(propertyName).toReal());
        }
    }
    return geometry;
}

QVariant getProperty(const QString& property, const QList<PropertyNode>& properties)
{
    for (const PropertyNode& propertyNode : properties) {
        for (const QString& propertyName : propertyNode.properties.keys()) {
            if (propertyName == property)
                return propertyNode.properties.value(propertyName);
        }
    }
    return QVariant();
}

Enum getEnum(const QString& name, const QList<PropertyNode>& properties)
{
    for (const PropertyNode& propertyNode : properties) {
        for (const Enum& enumm : propertyNode.enums) {
            if (enumm.name == name)
                return enumm;
        }
    }
    return Enum();
}

QList<QTreeWidgetItem*> topLevelItems(const QTreeWidget* treeWidget)
{
    QList<QTreeWidgetItem*> items;

    if (!treeWidget)
        return items;

    for (int i = 0; i < treeWidget->topLevelItemCount(); ++i)
        items.append(treeWidget->topLevelItem(i));

    return items;
}

QList<QTreeWidgetItem*> allSubChildItems(QTreeWidgetItem* parentItem, bool includeParent = true,
                                         bool includeCollapsed = true, bool includeHidden = false)
{
    QList<QTreeWidgetItem*> items;

    if (!parentItem)
        return items;

    if ((!includeCollapsed && !parentItem->isExpanded())
            || (!includeHidden && parentItem->isHidden())) {
        if (includeParent && (includeHidden || !parentItem->isHidden()))
            items.append(parentItem);

        return items;
    }

    if (includeParent)
        items.append(parentItem);

    for (int i = 0; i < parentItem->childCount(); i++) {
        if (includeHidden || !parentItem->child(i)->isHidden())
            items.append(parentItem->child(i));
        items.append(allSubChildItems(parentItem->child(i), false, includeCollapsed, includeHidden));
    }

    return items;
}

int calculateVisibleRow(const QTreeWidgetItem* item)
{
    QTreeWidget* treeWidget = item->treeWidget();
    Q_ASSERT(treeWidget);

    int totalCount = 0;
    for (QTreeWidgetItem* topLevelItem : topLevelItems(treeWidget))
        totalCount += allSubChildItems(topLevelItem, true, false).size();

    int count = 0;
    for (QTreeWidgetItem* topLevelItem : topLevelItems(treeWidget)) {
        for (QTreeWidgetItem* childItem : allSubChildItems(topLevelItem, true, false)) {
            if (childItem == item)
                return totalCount - count - 1;
            ++count;
        }
    }

    Q_ASSERT(0);
    return 0;
}

QWidget* createIdHandlerWidget(Control* control)
{
    auto lineEdit = new QLineEdit;
    lineEdit->setValidator(new QRegExpValidator(QRegExp("([a-z_][a-zA-Z0-9_]+)?"), lineEdit));
    lineEdit->setStyleSheet("QLineEdit { border: none; background: transparent; }");
    lineEdit->setAttribute(Qt::WA_MacShowFocusRect, false);
    lineEdit->setText(control->id());
    lineEdit->setFocusPolicy(Qt::StrongFocus);
    lineEdit->setSizePolicy(QSizePolicy::Ignored, lineEdit->sizePolicy().verticalPolicy());
    lineEdit->setMinimumWidth(1);
    setPalette(lineEdit);

    QObject::connect(lineEdit, &QLineEdit::editingFinished, [=]
    {
        if (control->id() != lineEdit->text()) {
            if (lineEdit->text().isEmpty()) {
                lineEdit->setText(control->id());
            } else {
                ControlPropertyManager::setId(control, lineEdit->text(),
                                              ControlPropertyManager::SaveChanges
                                              | ControlPropertyManager::UpdatePreviewer);
            }
        }
    });

    return lineEdit;
}

QWidget* createStringHandlerWidget(const QString& propertyName, const QString& text,
                                   Control* control)
{
    auto lineEdit = new QLineEdit;
    lineEdit->setStyleSheet("QLineEdit { border: none; background: transparent; }");
    lineEdit->setAttribute(Qt::WA_MacShowFocusRect, false);
    lineEdit->setText(text);
    lineEdit->setFocusPolicy(Qt::StrongFocus);
    lineEdit->setSizePolicy(QSizePolicy::Ignored, lineEdit->sizePolicy().verticalPolicy());
    lineEdit->setMinimumWidth(1);
    setPalette(lineEdit);

    QObject::connect(lineEdit, &QLineEdit::editingFinished, [=]
    {
        const QString& previousValue = getProperty(propertyName, control->properties()).value<QString>();

        if (previousValue == lineEdit->text())
            return;

        ControlPropertyManager::setProperty(control,
                                            propertyName, stringify(lineEdit->text()),
                                            lineEdit->text(),
                                            ControlPropertyManager::SaveChanges
                                            | ControlPropertyManager::UpdatePreviewer);
    });

    return lineEdit;
}

QWidget* createUrlHandlerWidget(const QString& propertyName, const QString& url,
                                Control* control)
{
    auto lineEdit = new QLineEdit;
    lineEdit->setStyleSheet("QLineEdit { border: none; background: transparent; }");
    lineEdit->setAttribute(Qt::WA_MacShowFocusRect, false);
    lineEdit->setText(url);
    lineEdit->setFocusPolicy(Qt::StrongFocus);
    lineEdit->setSizePolicy(QSizePolicy::Ignored, lineEdit->sizePolicy().verticalPolicy());
    lineEdit->setMinimumWidth(1);
    setPalette(lineEdit);

    QObject::connect(lineEdit, &QLineEdit::editingFinished, [=]
    {
        // TODO: Clear whitespaces in the url
        const QUrl& url = QUrl::fromUserInput(lineEdit->text(),
                                              SaveUtils::toThisDir(control->dir()),
                                              QUrl::AssumeLocalFile);
        const QString& displayText = urlToDisplayText(url, control->dir());
        const QUrl& previousUrl = getProperty(propertyName, control->properties()).value<QUrl>();

        if (url == previousUrl)
            return;

        ControlPropertyManager::setProperty(control, propertyName, stringify(displayText), url,
                                            ControlPropertyManager::SaveChanges
                                            | ControlPropertyManager::UpdatePreviewer);
    });

    return lineEdit;
}

QWidget* createEnumHandlerWidget(const Enum& enumm, Control* control)
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
    setPalette(comboBox);

    QObject::connect(comboBox, qOverload<int>(&QComboBox::activated), [=]
    {
        const QString& previousValue = getEnum(enumm.name, control->properties()).value;

        if (previousValue == comboBox->currentText())
            return;

        QString fixedScope = enumm.scope;
        if (control->window() && fixedScope == "Window") {
            const QByteArray& qml = rdfile(control->url());
            if (!qml.contains("import QtQuick.Window"))
                fixedScope = "ApplicationWindow";
        }

        ControlPropertyManager::setProperty(control,
                                            enumm.name, fixedScope + "." + comboBox->currentText(),
                                            enumm.keys.value(comboBox->currentText()),
                                            ControlPropertyManager::SaveChanges
                                            | ControlPropertyManager::UpdatePreviewer);
    });

    return comboBox;
}

QWidget* createBoolHandlerWidget(const QString& propertyName, bool checked, Control* control)
{
    auto checkBox = new QCheckBox;
    checkBox->setAttribute(Qt::WA_MacShowFocusRect, false);
    checkBox->setCursor(Qt::PointingHandCursor);
    checkBox->setChecked(checked);
    checkBox->setFocusPolicy(Qt::ClickFocus);
    checkBox->setMinimumWidth(1);
    fixVisibleForWindow(control, propertyName, checkBox);
    setPalette(checkBox);

    QObject::connect(checkBox, qOverload<bool>(&QCheckBox::clicked), [=]
    {
        // NOTE: No need for previous value equality check, since this signal is only emitted
        // when the value is changed/toggled
        ControlPropertyManager::setProperty(control,
                                            propertyName, checkBox->isChecked() ? "true" : "false",
                                            checkBox->isChecked(),
                                            ControlPropertyManager::SaveChanges
                                            | ControlPropertyManager::UpdatePreviewer);
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

QWidget* createColorHandlerWidget(const QString& propertyName, const QColor& color,
                                  Control* control)
{
    auto toolButton = new QToolButton;
    toolButton->setStyleSheet("QToolButton { border: none; background: transparent; }");
    toolButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    toolButton->setText(color.name(QColor::HexArgb));
    toolButton->setIcon(QIcon(QPixmap::fromImage(colorToImage({12, 12}, color))));
    toolButton->setAttribute(Qt::WA_MacShowFocusRect, false);
    toolButton->setIconSize({12, 12});
    toolButton->setCursor(Qt::PointingHandCursor);
    toolButton->setFocusPolicy(Qt::ClickFocus);
    toolButton->setSizePolicy(QSizePolicy::Ignored, toolButton->sizePolicy().verticalPolicy());
    toolButton->setMinimumWidth(1);
    setPalette(toolButton);

    QObject::connect(toolButton, &QCheckBox::clicked, [=]
    {
        const QColor& previousColor = getProperty(propertyName, control->properties()).value<QColor>();

        QColorDialog cDialog;
        cDialog.setWindowTitle(QObject::tr("Select Color"));
        cDialog.setOptions(QColorDialog::ShowAlphaChannel | QColorDialog::DontUseNativeDialog);
        cDialog.setCurrentColor(previousColor);
        cDialog.exec();

        const QColor& color = cDialog.currentColor();
        if (color == previousColor || !color.isValid())
            return;

        toolButton->setText(color.name(QColor::HexArgb));
        toolButton->setIcon(QIcon(QPixmap::fromImage(colorToImage({12, 12}, color))));
        ControlPropertyManager::setProperty(control, propertyName,
                                            stringify(color.name(QColor::HexArgb)), color,
                                            ControlPropertyManager::SaveChanges
                                            | ControlPropertyManager::UpdatePreviewer);
    });

    return toolButton;
}

QWidget* createNumberHandlerWidget(const QString& propertyName, double number,
                                   Control* control, bool integer)
{
    QAbstractSpinBox* abstractSpinBox;
    if (integer)
        abstractSpinBox = new QSpinBox;
    else
        abstractSpinBox = new QDoubleSpinBox;

    TransparentStyle::attach(abstractSpinBox);
    abstractSpinBox->setCursor(Qt::PointingHandCursor);
    abstractSpinBox->installEventFilter(&g_wheelDisabler);
    abstractSpinBox->setFocusPolicy(Qt::StrongFocus);
    abstractSpinBox->setSizePolicy(QSizePolicy::Ignored, abstractSpinBox->sizePolicy().verticalPolicy());
    abstractSpinBox->setMinimumWidth(1);
    setPalette(abstractSpinBox);

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
                ControlPropertyManager::SaveChanges | ControlPropertyManager::UpdatePreviewer;

        if (control->form() && (propertyName == "x" || propertyName == "y"))
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
        spinBox->setMinimum(std::numeric_limits<int>::min());
        spinBox->setValue(number);
        fixPosForForm(control, propertyName, spinBox);
        QObject::connect(spinBox, qOverload<int>(&QSpinBox::valueChanged), updateFunction);
    } else {
        QDoubleSpinBox* spinBox = static_cast<QDoubleSpinBox*>(abstractSpinBox);
        spinBox->setMaximum(std::numeric_limits<double>::max());
        spinBox->setMinimum(std::numeric_limits<double>::min());
        spinBox->setValue(number);
        fixPosForForm(control, propertyName, spinBox);
        QObject::connect(spinBox, qOverload<double>(&QDoubleSpinBox::valueChanged), updateFunction);
    }

    return abstractSpinBox;
}

QWidget* createFontFamilyHandlerWidget(const QString& family, Control* control, QTreeWidgetItem* fontItem)
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
    setPalette(comboBox);

    QObject::connect(comboBox, qOverload<int>(&QComboBox::activated), [=]
    {
        const QFont& font = getProperty("font", control->properties()).value<QFont>();

        if (comboBox->currentText() == QFontInfo(font).family())
            return;

        const QString& previousFamily = QFontInfo(font).family();
        QString fontText = fontItem->text(1);
        fontText.replace(previousFamily, comboBox->currentText());
        fontItem->setText(1, fontText);

        ControlPropertyManager::setProperty(control, "font.family", stringify(comboBox->currentText()),
                                            comboBox->currentText(), ControlPropertyManager::SaveChanges
                                            | ControlPropertyManager::UpdatePreviewer);
    });

    return comboBox;
}

QWidget* createFontWeightHandlerWidget(int weight, Control* control)
{
    auto comboBox = new QComboBox;
    TransparentStyle::attach(comboBox);
    comboBox->setAttribute(Qt::WA_MacShowFocusRect, false);
    comboBox->setCursor(Qt::PointingHandCursor);
    comboBox->setFocusPolicy(Qt::ClickFocus);
    comboBox->setSizePolicy(QSizePolicy::Ignored, comboBox->sizePolicy().verticalPolicy());
    comboBox->setMinimumWidth(1);
    setPalette(comboBox);

    QMetaEnum weightEnum = QMetaEnum::fromType<QFont::Weight>();
    for (int i = weightEnum.keyCount(); i--;) { // Necessary somehow
        if (QString::fromUtf8(weightEnum.key(i)).contains(QRegularExpression("[^\\r\\n\\t\\f\\v ]")))
            comboBox->addItem(weightEnum.key(i));
    }

    comboBox->setCurrentText(QMetaEnum::fromType<QFont::Weight>().valueToKey(weight));

    QObject::connect(comboBox, qOverload<int>(&QComboBox::activated), [=]
    {
        int weightValue = weightEnum.keyToValue(comboBox->currentText().toUtf8().constData());
        const QFont& font = getProperty("font", control->properties()).value<QFont>();

        if (weightValue == font.weight())
            return;

        ControlPropertyManager::setProperty(control, "font.weight",
                                            "Font." + comboBox->currentText(), weightValue,
                                            ControlPropertyManager::SaveChanges
                                            | ControlPropertyManager::UpdatePreviewer);
    });

    return comboBox;
}

QWidget* createFontCapitalizationHandlerWidget(QFont::Capitalization capitalization, Control* control)
{
    auto comboBox = new QComboBox;
    TransparentStyle::attach(comboBox);
    comboBox->setAttribute(Qt::WA_MacShowFocusRect, false);
    comboBox->setCursor(Qt::PointingHandCursor);
    comboBox->setFocusPolicy(Qt::ClickFocus);
    comboBox->setSizePolicy(QSizePolicy::Ignored, comboBox->sizePolicy().verticalPolicy());
    comboBox->setMinimumWidth(1);
    setPalette(comboBox);

    QMetaEnum capitalizationEnum = QMetaEnum::fromType<QFont::Capitalization>();
    for (int i = capitalizationEnum.keyCount(); i--;) { // Necessary somehow
        if (QString::fromUtf8(capitalizationEnum.key(i)).contains(QRegularExpression("[^\\r\\n\\t\\f\\v ]")))
            comboBox->addItem(capitalizationEnum.key(i));
    }

    comboBox->setCurrentText(QMetaEnum::fromType<QFont::Capitalization>().valueToKey(capitalization));

    QObject::connect(comboBox, qOverload<int>(&QComboBox::activated), [=]
    {
        int capitalizationValue = capitalizationEnum.keyToValue(comboBox->currentText().toUtf8().constData());
        const QFont& font = getProperty("font", control->properties()).value<QFont>();

        if (capitalizationValue == font.capitalization())
            return;

        ControlPropertyManager::setProperty(control, "font.capitalization",
                                            "Font." + comboBox->currentText(),
                                            QFont::Capitalization(capitalizationValue),
                                            ControlPropertyManager::SaveChanges
                                            | ControlPropertyManager::UpdatePreviewer);
    });

    return comboBox;
}

QWidget* createFontSizeHandlerWidget(const QString& propertyName, int size, Control* control, QTreeWidgetItem* fontItem)
{
    QSpinBox* spinBox = new QSpinBox;
    TransparentStyle::attach(spinBox);
    spinBox->setCursor(Qt::PointingHandCursor);
    spinBox->installEventFilter(&g_wheelDisabler);
    spinBox->setFocusPolicy(Qt::StrongFocus);
    spinBox->setMinimum(0);
    spinBox->setMaximum(72);
    spinBox->setValue(size < 0 ? 0 : size);
    spinBox->setSizePolicy(QSizePolicy::Ignored, spinBox->sizePolicy().verticalPolicy());
    spinBox->setMinimumWidth(1);
    setPalette(spinBox);

    QObject::connect(spinBox, qOverload<int>(&QSpinBox::valueChanged), [=]
    {
        bool isPx = propertyName == "pixelSize" ? true : false;
        QFont font = getProperty("font", control->properties()).value<QFont>();

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
                                            ControlPropertyManager::UpdatePreviewer);
    });

    return spinBox;
}

void createAndAddGeometryPropertiesBlock(QTreeWidgetItem* classItem,
                                         const QList<PropertyNode>& properties,
                                         Control* control, int integer)
{
    QTreeWidget* treeWidget = classItem->treeWidget();
    Q_ASSERT(treeWidget);

    const QRectF& geometry = getGeometryFromProperties(properties);

    bool xUnknown = false, yUnknown = false;
    if (control->form()) {
        xUnknown = !ParserUtils::exists(control->url(), "x");
        yUnknown = !ParserUtils::exists(control->url(), "y");
    }

    const QString& geometryText = QString::fromUtf8("[(%1, %2), %3 x %4]")
            .arg(xUnknown ? "?" : QString::number(int(geometry.x())))
            .arg(yUnknown ? "?" : QString::number(int(geometry.y())))
            .arg(int(geometry.width()))
            .arg(int(geometry.height()));

    const bool xChanged = ParserUtils::exists(control->url(), "x");
    const bool yChanged = ParserUtils::exists(control->url(), "y");
    const bool wChanged = ParserUtils::exists(control->url(), "width");
    const bool hChanged = ParserUtils::exists(control->url(), "height");
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

void createAndAddFontPropertiesBlock(QTreeWidgetItem* classItem, const QFont& font, Control* control)
{
    QTreeWidget* treeWidget = classItem->treeWidget();
    Q_ASSERT(treeWidget);

    const bool isPx = font.pixelSize() > 0 ? true : false;
    const QString& fontText = QString::fromUtf8("[%1, %2%3]")
            .arg(QFontInfo(font).family())
            .arg(isPx ? font.pixelSize() : font.pointSize())
            .arg(isPx ? "px" : "pt");

    const bool fChanged    = ParserUtils::exists(control->url(), "font.family");
    const bool bChanged    = ParserUtils::exists(control->url(), "font.bold");
    const bool iChanged    = ParserUtils::exists(control->url(), "font.italic");
    const bool uChanged    = ParserUtils::exists(control->url(), "font.underline");
    const bool poChanged   = ParserUtils::exists(control->url(), "font.pointSize");
    const bool piChanged   = ParserUtils::exists(control->url(), "font.pixelSize");
    const bool wChanged    = ParserUtils::exists(control->url(), "font.weight");
    const bool oChanged    = ParserUtils::exists(control->url(), "font.overline");
    const bool sChanged    = ParserUtils::exists(control->url(), "font.strikeout");
    const bool cChanged    = ParserUtils::exists(control->url(), "font.capitalization");
    const bool kChanged    = ParserUtils::exists(control->url(), "font.kerning");
    const bool prChanged   = ParserUtils::exists(control->url(), "font.preferShaping");
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
}

class PropertiesListDelegate: public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit PropertiesListDelegate(PropertiesPane* parent) : QStyledItemDelegate(parent)
      , m_propertiesPane(parent)
    {}

    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override
    {
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);

        const QAbstractItemModel* model = index.model();
        const bool isClassRow = !model->parent(index).isValid() && index.row() > 2;

        fillBackground(painter, option.rect,
                       calculateVisibleRow(m_propertiesPane->itemFromIndex(index)),
                       isClassRow, index.column() == 0 && !isClassRow);

        // Draw data
        if (isClassRow) {
            painter->setPen(Qt::white);
        } else {
            if (index.column() == 0 && index.data(Qt::DecorationRole).toBool()) {
                QFont font (option.font);
                font.setWeight(QFont::Medium);
                painter->setFont(font);
                painter->setPen("#56ab49");
            } else {
                painter->setPen("#403121");
            }
        }

        const QRectF& textRect = option.rect.adjusted(5, 0, 0, 0);
        const QString& text = index.data(Qt::DisplayRole).toString();
        painter->drawText(textRect,
                          option.fontMetrics.elidedText(text, Qt::ElideMiddle, textRect.width()),
                          QTextOption(Qt::AlignLeft | Qt::AlignVCenter));
        painter->restore();
    }

    QSize sizeHint(const QStyleOptionViewItem& opt, const QModelIndex& index) const override
    {
        const QSize& size = QStyledItemDelegate::sizeHint(opt, index);
        return QSize(size.width(), 21); // Fixed height for rows, 21
    }

private:
    PropertiesPane* m_propertiesPane;
};

PropertiesPane::PropertiesPane(DesignerScene* designerScene, QWidget* parent) : QTreeWidget(parent)
  , m_designerScene(designerScene)
  , m_searchEdit(new FocuslessLineEdit(this))
{
    QFont fontMedium(font());
    fontMedium.setWeight(QFont::Medium);

    ::setPalette(this);
    header()->setFont(fontMedium);
    header()->setFixedHeight(23);
    header()->setDefaultSectionSize(1);
    header()->setMinimumSectionSize(1);
    header()->resizeSection(0, 165); // Don't resize the last (stretched) column

    headerItem()->setText(1, tr("Value"));
    headerItem()->setText(0, tr("Property"));

    setColumnCount(2);
    setIndentation(16);
    setDragEnabled(false);
    setUniformRowHeights(true);
    setDropIndicatorShown(false);
    setExpandsOnDoubleClick(true);
    setItemDelegate(new PropertiesListDelegate(this));
    setAttribute(Qt::WA_MacShowFocusRect, false);
    setSelectionBehavior(QTreeWidget::SelectRows);
    setSelectionMode(QTreeWidget::NoSelection);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setVerticalScrollMode(QTreeWidget::ScrollPerPixel);
    setHorizontalScrollMode(QTreeWidget::ScrollPerPixel);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setStyleSheet("QTreeView {"
                  "    border: 1px solid #8c6a48;"
                  "    outline: 0;"
                  "} QHeaderView::section {"
                  "    padding-left: 5px;"
                  "    color: white;"
                  "    border: none;"
                  "    border-bottom: 1px solid #8c6a48;"
                  "    background: qlineargradient(spread:pad, x1:0.5, y1:0, x2:0.5, y2:1,"
                  "                                stop:0 #AB8157, stop:1 #9C7650);"
                  "}");

    m_searchEdit->setPlaceholderText("Filter");
    m_searchEdit->setClearButtonEnabled(true);
    m_searchEdit->setFixedHeight(22);
    connect(m_searchEdit, &FocuslessLineEdit::textChanged, this, &PropertiesPane::filterList);

    connect(verticalScrollBar(), &QScrollBar::valueChanged, [=] (int value)
    {
        if (topLevelItemCount() > 0) {
            g_verticalScrollBarPosition = verticalScrollBar()->maximum()
                    - verticalScrollBar()->minimum() - value;
        }
    });
    connect(horizontalScrollBar(), &QScrollBar::valueChanged, [=] (int value)
    {
        if (topLevelItemCount() > 0) {
            g_horizontalScrollBarPosition = horizontalScrollBar()->maximum()
                    - horizontalScrollBar()->minimum() - value;
        }
    });
    connect(verticalScrollBar(), &QScrollBar::rangeChanged, [=] (int min, int max)
    {
        if (topLevelItemCount() > 0)
            g_verticalScrollBarPosition = max - min - verticalScrollBar()->value();
    });
    connect(horizontalScrollBar(), &QScrollBar::rangeChanged, [=] (int min, int max)
    {
        if (topLevelItemCount() > 0)
            g_horizontalScrollBarPosition = max - min - horizontalScrollBar()->value();
    });

    connect(m_designerScene, &DesignerScene::currentFormChanged,
            this, &PropertiesPane::onSelectionChange);
    connect(m_designerScene, &DesignerScene::selectionChanged,
            this, &PropertiesPane::onSelectionChange);
    connect(ControlPropertyManager::instance(), &ControlPropertyManager::zChanged,
            this, &PropertiesPane::onZChange);
    connect(ControlPropertyManager::instance(), &ControlPropertyManager::previewChanged,
            this, &PropertiesPane::onPreviewChange);
    connect(ControlPropertyManager::instance(), &ControlPropertyManager::geometryChanged,
            this, &PropertiesPane::onGeometryChange);
    connect(ControlPropertyManager::instance(), &ControlPropertyManager::propertyChanged,
            this, &PropertiesPane::onPropertyChange);
    connect(ControlPropertyManager::instance(), &ControlPropertyManager::idChanged,
            this, &PropertiesPane::onIdChange);
}

void PropertiesPane::sweep()
{
    g_verticalScrollBarPosition = 99999;
    g_horizontalScrollBarPosition = 99999;
    m_searchEdit->clear();
    clear();
}

void PropertiesPane::onSelectionChange()
{
    const int verticalScrollBarPosition = g_verticalScrollBarPosition;
    const int horizontalScrollBarPosition = g_horizontalScrollBarPosition;

    clear();

    if (m_designerScene->selectedControls().size() != 1)
        return;

    Control* selectedControl = m_designerScene->selectedControls().first();
    setDisabled(selectedControl->hasErrors());

    const QList<PropertyNode>& properties = selectedControl->properties();
    if (properties.isEmpty())
        return;

    QTreeWidgetItem* typeItem = new QTreeWidgetItem;
    typeItem->setText(0, tr("Type"));
    typeItem->setText(1, properties.first().cleanClassName);
    typeItem->setData(0, Qt::DecorationRole, false); // No 'property changed' indication
    addTopLevelItem(typeItem);

    QTreeWidgetItem* uidItem = new QTreeWidgetItem;
    uidItem->setText(0, "uid");
    uidItem->setText(1, selectedControl->uid());
    uidItem->setData(0, Qt::DecorationRole, false); // No 'property changed' indication
    addTopLevelItem(uidItem);

    QTreeWidgetItem* idItem = new QTreeWidgetItem;
    idItem->setText(0, "id");
    idItem->setData(0, Qt::DecorationRole, false); // No 'property changed' indication
    addTopLevelItem(idItem);
    setItemWidget(idItem, 1, createIdHandlerWidget(selectedControl));

    for (const PropertyNode& propertyNode : properties) {
        const QList<Enum>& enumList = propertyNode.enums;
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
                              ParserUtils::exists(selectedControl->url(), propertyName));
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
                              ParserUtils::exists(selectedControl->url(), propertyName));
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
                              ParserUtils::exists(selectedControl->url(), propertyName));
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
                              ParserUtils::exists(selectedControl->url(), propertyName));
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
                                  ParserUtils::exists(selectedControl->url(), propertyName));
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
                                  ParserUtils::exists(selectedControl->url(), propertyName));
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
            item->setData(0, Qt::DecorationRole, ParserUtils::exists(selectedControl->url(), enumm.name));
            classItem->addChild(item);
            setItemWidget(item, 1, createEnumHandlerWidget(enumm, selectedControl));
        }

        expandItem(classItem);
    }

    filterList(m_searchEdit->text());

    verticalScrollBar()->setSliderPosition(verticalScrollBar()->maximum()
                                           - verticalScrollBar()->minimum()
                                           - verticalScrollBarPosition);
    horizontalScrollBar()->setSliderPosition(horizontalScrollBar()->maximum()
                                             - horizontalScrollBar()->minimum()
                                             - horizontalScrollBarPosition);
}

void PropertiesPane::onZChange(Control* control)
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

                childItem->setData(0, Qt::DecorationRole, ParserUtils::exists(control->url(), "z"));
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

void PropertiesPane::onPreviewChange(Control* control, bool codeChanged)
{
    if (m_designerScene->selectedControls().size() != 1)
        return;

    Control* selectedControl = m_designerScene->selectedControls().first();
    if (selectedControl != control)
        return;

    if (topLevelItemCount() <= 0)
        return onSelectionChange();

    if (codeChanged)
        return onSelectionChange();
}

void PropertiesPane::onGeometryChange(Control* control)
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
    if (control->form()) {
        xUnknown = !ParserUtils::exists(control->url(), "x");
        yUnknown = !ParserUtils::exists(control->url(), "y");
    }

    const QString& geometryText = QString::fromUtf8("[(%1, %2), %3 x %4]")
            .arg(xUnknown ? "?" : QString::number(int(geometry.x())))
            .arg(yUnknown ? "?" : QString::number(int(geometry.y())))
            .arg(int(geometry.width()))
            .arg(int(geometry.height()));

    const bool xChanged = ParserUtils::exists(control->url(), "x");
    const bool yChanged = ParserUtils::exists(control->url(), "y");
    const bool wChanged = ParserUtils::exists(control->url(), "width");
    const bool hChanged = ParserUtils::exists(control->url(), "height");
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
                    dSpinBox->setValue(control->x());
                    fixPosForForm(control, "x", dSpinBox);
                } else {
                    iSpinBox->setValue(control->x());
                    fixPosForForm(control, "x", iSpinBox);
                }
            } else if (childItem->text(0) == "y") {
                childItem->setData(0, Qt::DecorationRole, yChanged);
                if (dSpinBox) {
                    dSpinBox->setValue(control->y());
                    fixPosForForm(control, "y", dSpinBox);
                } else {
                    iSpinBox->setValue(control->y());
                    fixPosForForm(control, "y", iSpinBox);
                }
            } else if (childItem->text(0) == "width") {
                childItem->setData(0, Qt::DecorationRole, wChanged);
                if (dSpinBox)
                    dSpinBox->setValue(control->size().width());
                else
                    iSpinBox->setValue(control->size().width());
            } else if (childItem->text(0) == "height") {
                childItem->setData(0, Qt::DecorationRole, hChanged);
                if (dSpinBox)
                    dSpinBox->setValue(control->size().height());
                else
                    iSpinBox->setValue(control->size().height());
            }

            if (dSpinBox)
                dSpinBox->blockSignals(false);
            else
                iSpinBox->blockSignals(false);
        }
    }
}

void PropertiesPane::onIdChange(Control* control, const QString& /*previousId*/)
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

void PropertiesPane::onPropertyChange()
{
    Q_UNUSED(0)
    /*
        FIXME: Empty for now, because the only user of the setProperty function of
               ControlPropertyManager is this class. Hence no need to handle property
               changes which made by us already.
    */
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
    const bool isClassRow = !model->parent(index).isValid() && index.row() > 2;

    QRectF handleRect(0, 0, width, width);
    handleRect.moveCenter(rect.center());
    handleRect.moveRight(rect.right() - 0.5);

    fillBackground(painter, rect, calculateVisibleRow(itemFromIndex(index)), isClassRow, false);

    // Draw handle
    if (hasChild) {
        QPen pen;
        pen.setWidthF(1.3);
        pen.setColor(isClassRow ? QColor(Qt::white) : QColor("#403121"));
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

    /* Fill background */
    const qreal bandHeight = 21;
    const qreal bandCount = viewport()->height() / bandHeight;
    const QList<Control*>& selectedControls = m_designerScene->selectedControls();

    painter.fillRect(rect(), Qt::white);

    for (int i = 0; i < bandCount; ++i) {
        if (i % 2) {
            painter.fillRect(0, i * bandHeight, viewport()->width(), bandHeight, "#faf1e8");
        } else if (topLevelItemCount() == 0) {
            if (i == int(bandCount / 2.0) || i == int(bandCount / 2.0) + 1) {
                QString message;
                if (selectedControls.size() == 0)
                    message = tr("No controls selected");
                else if (selectedControls.size() == 1)
                    message = tr("Control has errors");
                else
                    message = tr("Multiple controls selected");

                painter.setPen(selectedControls.size() == 1 ? "#d98083" : "#b5aea7");
                painter.drawText(0, i * bandHeight, viewport()->width(), bandHeight,
                                 Qt::AlignCenter, message);
            }
        }
    }

    QTreeWidget::paintEvent(e);
}

void PropertiesPane::updateGeometries()
{
    QTreeWidget::updateGeometries();
    QMargins vm = viewportMargins();
    vm.setBottom(m_searchEdit->height());
    QRect vg = viewport()->geometry();
    QRect geometryRect(vg.left(), vg.bottom(), vg.width(), m_searchEdit->height());
    setViewportMargins(vm);
    m_searchEdit->setGeometry(geometryRect);
}

QSize PropertiesPane::sizeHint() const
{
    return QSize{310, 700};
}

#include "propertiespane.moc"
