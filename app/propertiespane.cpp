#include <propertiespane.h>
#include <focuslesslineedit.h>
#include <css.h>
#include <filemanager.h>
#include <saveutils.h>
#include <controlpropertymanager.h>
//#include <controlcreationmanager.h>
//#include <controlremovingmanager.h>
#include <designerscene.h>
//#include <form.h>
//#include <projectmanager.h>
#include <dpr.h>

#include <transparentcombobox.h>

#include <QStyledItemDelegate>
#include <QPainter>
#include <QHeaderView>
#include <QScrollBar>
#include <QCheckBox>
#include <QJSEngine>
#include <QToolButton>
#include <QColorDialog>

#include <QDebug>
namespace {

QImage colorToImage(const QSize& layoutSize, const QColor& color)
{
    QImage image(layoutSize * DPR, QImage::Format_ARGB32_Premultiplied);
    image.setDevicePixelRatio(DPR);
    image.fill(color);
    QPainter p(&image);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(Qt::black);
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
        painter->fillRect(rect, "#fffefc");

    // Draw top and bottom lines
    painter->setPen("#25000000");
    painter->drawLine(rect.topLeft() + QPointF{0.5, 0.0}, rect.topRight() - QPointF{0.5, 0.0});
    painter->drawLine(rect.bottomLeft() + QPointF{0.5, 0.0}, rect.bottomRight() - QPointF{0.5, 0.0});

    // Draw vertical line
    if (verticalLine) {
        painter->drawLine(rect.topRight() + QPointF(-0.5, 0.5),
                          rect.bottomRight() + QPointF(-0.5, -0.5));
    }

    painter->restore();
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
                                         bool includeCollapsed = true)
{
    QList<QTreeWidgetItem*> items;

    if (!parentItem)
        return items;

    if (!includeCollapsed && !parentItem->isExpanded()) {
        if (includeParent)
            items.append(parentItem);

        return items;
    }

    if (includeParent)
        items.append(parentItem);

    for (int i = 0; i < parentItem->childCount(); i++) {
        items.append(parentItem->child(i));
        items.append(allSubChildItems(parentItem->child(i), false, includeCollapsed));
    }

    return items;
}

int calculateVisibleRow(const QTreeWidgetItem* item, const QTreeWidget* treeWidget)
{
    int count = 0;
    for (QTreeWidgetItem* topLevelItem : topLevelItems(treeWidget)) {
        for (QTreeWidgetItem* childItem : allSubChildItems(topLevelItem, true, false)) {
            if (childItem == item)
                return count;

            ++count;
        }
    }

    return count;
}

QWidget* createIdHandlerWidget(Control* selectedControl)
{
    auto lineEdit = new QLineEdit;
    lineEdit->setValidator(new QRegExpValidator(QRegExp("([a-z_][a-zA-Z0-9_]+)?"), lineEdit));
    lineEdit->setStyleSheet("QLineEdit { border: none; background: transparent; }");
    lineEdit->setAttribute(Qt::WA_MacShowFocusRect, false);
    lineEdit->setText(selectedControl->id());

    QObject::connect(lineEdit, &QLineEdit::editingFinished, [=]
    {
        if (selectedControl->id() != lineEdit->text()) {
            if (lineEdit->text().isEmpty())
                lineEdit->setText(selectedControl->id());
            else
                ControlPropertyManager::setId(selectedControl, lineEdit->text());
        }
    });

    return lineEdit;
}

QWidget* createStringHandlerWidget(const QString& propertyName, const QString& text, Control* selectedControl)
{
    auto lineEdit = new QLineEdit;
    lineEdit->setStyleSheet("QLineEdit { border: none; background: transparent; }");
    lineEdit->setAttribute(Qt::WA_MacShowFocusRect, false);
    lineEdit->setText(text);

    QObject::connect(lineEdit, &QLineEdit::editingFinished, [=]
    {
        ControlPropertyManager::setProperty(selectedControl,
                                            propertyName, stringify(lineEdit->text()),
                                            lineEdit->text());
    });

    return lineEdit;
}

QWidget* createUrlHandlerWidget(const QString& propertyName, const QString& url, Control* selectedControl)
{
    auto lineEdit = new QLineEdit;
    lineEdit->setStyleSheet("QLineEdit { border: none; background: transparent; }");
    lineEdit->setAttribute(Qt::WA_MacShowFocusRect, false);
    lineEdit->setText(url);

    QObject::connect(lineEdit, &QLineEdit::editingFinished, [=]
    {
        const QUrl& url = QUrl::fromUserInput(lineEdit->text(),
                                              SaveUtils::toThisDir(selectedControl->dir()),
                                              QUrl::AssumeLocalFile);
        const QString& displayText = urlToDisplayText(url, selectedControl->dir());
        ControlPropertyManager::setProperty(selectedControl, propertyName, stringify(displayText), url);
    });

    return lineEdit;
}

QWidget* createEnumHandlerWidget(const Enum& enumm, Control* selectedControl)
{
    auto comboBox = new TransparentComboBox;
    comboBox->setAttribute(Qt::WA_MacShowFocusRect, false);
    comboBox->addItems(enumm.keys.keys());
    comboBox->setCurrentText(enumm.value);
    comboBox->setCursor(Qt::PointingHandCursor);

    QObject::connect(comboBox, qOverload<int>(&QComboBox::activated), [=]
    {
        ControlPropertyManager::setProperty(selectedControl,
                                            enumm.name, enumm.scope + "." + comboBox->currentText(),
                                            enumm.keys.value(comboBox->currentText()));
    });

    return comboBox;
}

QWidget* createBoolHandlerWidget(const QString& propertyName, bool checked, Control* selectedControl)
{
    auto checkBox = new QCheckBox;
    checkBox->setAttribute(Qt::WA_MacShowFocusRect, false);
    checkBox->setChecked(checked);

    QObject::connect(checkBox, qOverload<bool>(&QCheckBox::clicked), [=]
    {
        ControlPropertyManager::setProperty(selectedControl,
                                            propertyName, checkBox->isChecked() ? "true" : "false",
                                            checkBox->isChecked());
    });

    return checkBox;
}

QWidget* createColorHandlerWidget(const QString& propertyName, const QColor& color, Control* selectedControl)
{
    auto toolButton = new QToolButton;
    toolButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    toolButton->setText(color.name(QColor::HexArgb));
    toolButton->setIcon(QIcon(QPixmap::fromImage(colorToImage({12, 12}, color))));
    toolButton->setAttribute(Qt::WA_MacShowFocusRect, false);
    toolButton->setIconSize({12, 12});
    toolButton->setCursor(Qt::PointingHandCursor);

    QObject::connect(toolButton, &QCheckBox::clicked, [=]
    {
        const QColor& color = QColorDialog::getColor(Qt::white, nullptr, QObject::tr("Select Color"),
                                                     QColorDialog::ShowAlphaChannel
                                                     | QColorDialog::DontUseNativeDialog);

        if (color.isValid()) {
            toolButton->setText(color.name(QColor::HexArgb));
            toolButton->setIcon(QIcon(QPixmap::fromImage(colorToImage({12, 12}, color))));
            ControlPropertyManager::setProperty(selectedControl, propertyName,
                                                stringify(color.name(QColor::HexArgb)), color);
        }
    });

    return toolButton;
}
}

class PropertiesListDelegate: public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit PropertiesListDelegate(PropertiesPane* parent) : QStyledItemDelegate(parent)
      , m_propertiesPane(parent)
    {}

    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override
    {
        painter->setRenderHint(QPainter::Antialiasing);

        const QAbstractItemModel* model = index.model();
        const bool isClassRow = !model->parent(index).isValid() && index.row() > 2;

        fillBackground(painter, option.rect,
                       calculateVisibleRow(m_propertiesPane->itemFromIndex(index), m_propertiesPane),
                       isClassRow, index.column() == 0 && !isClassRow);

        // Draw data
        if (index.column() == 0 || index.row() < 2) {
            if (isClassRow)
                painter->setPen(Qt::white);
            else
                painter->setPen(Qt::black);

            painter->drawText(option.rect.adjusted(5, 0, 0, 0), index.data(Qt::EditRole).toString(),
                              QTextOption(Qt::AlignLeft | Qt::AlignVCenter));
        }
    }

    QSize sizeHint(const QStyleOptionViewItem& opt, const QModelIndex& index) const override
    {
        const QSize& size = QStyledItemDelegate::sizeHint(opt, index);
        return QSize(size.width(), 21);
    }

private:
    PropertiesPane* m_propertiesPane;
};

PropertiesPane::PropertiesPane(DesignerScene* designerScene, QWidget* parent) : QTreeWidget(parent)
  , m_designerScene(designerScene)
  , m_searchEdit(new FocuslessLineEdit(this))
{
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

    QFont fontMedium(font());
    fontMedium.setWeight(QFont::Medium);

    header()->setFont(fontMedium);
    header()->setFixedHeight(23);
    header()->resizeSection(0, 180);
    header()->resizeSection(1, 120);
    headerItem()->setText(1, tr("Value"));
    headerItem()->setText(0, tr("Property"));
    verticalScrollBar()->setStyleSheet(CSS_SCROLLBAR);
    horizontalScrollBar()->setStyleSheet(CSS_SCROLLBAR_H);
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
    //    connect(m_searchEdit, SIGNAL(textChanged(QString)), SLOT(filterList(QString)));

    /* Prepare Properties Widget */
    connect(m_designerScene, &DesignerScene::selectionChanged,
            this, &PropertiesPane::onSelectionChange);
    //    connect(m_designerScene, SIGNAL(selectionChanged()), SLOT(handleSelectionChange()));
    // BUG   connect(ControlMonitoringManager::instance(), SIGNAL(geometryChanged(Control*)), SLOT(handleSelectionChange()));
    // BUG   connect(ControlMonitoringManager::instance(), SIGNAL(zValueChanged(Control*)), SLOT(handleSelectionChange()));
}

void PropertiesPane::sweep()
{
    // TODO
}

void PropertiesPane::onSelectionChange()
{
    const int verticalScrollBarPosition = verticalScrollBar()->sliderPosition();
    const int horizontalScrollBarPosition = horizontalScrollBar()->sliderPosition();

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
    addTopLevelItem(typeItem);

    QTreeWidgetItem* uidItem = new QTreeWidgetItem;
    uidItem->setText(0, "uid");
    uidItem->setText(1, selectedControl->uid());
    addTopLevelItem(uidItem);

    QTreeWidgetItem* idItem = new QTreeWidgetItem;
    idItem->setText(0, "id");
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
                //                addFontChild(item, propertyName, font);
                break;
            }

            case QVariant::Color: {
                const QColor& color = propertyValue.value<QColor>();
                auto item = new QTreeWidgetItem;
                item->setText(0, propertyName);
                classItem->addChild(item);
                setItemWidget(item, 1, createColorHandlerWidget(propertyName, color, selectedControl));
                break;
            }

            case QVariant::Bool: {
                const bool checked = propertyValue.value<bool>();
                auto item = new QTreeWidgetItem;
                item->setText(0, propertyName);
                classItem->addChild(item);
                setItemWidget(item, 1, createBoolHandlerWidget(propertyName, checked, selectedControl));
                break;
            }

            case QVariant::String: {
                const QString& text = propertyValue.value<QString>();
                auto item = new QTreeWidgetItem;
                item->setText(0, propertyName);
                classItem->addChild(item);
                setItemWidget(item, 1, createStringHandlerWidget(propertyName, text, selectedControl));
                break;
            }

            case QVariant::Url: {
                const QUrl& url = propertyValue.value<QUrl>();
                const QString& displayText = urlToDisplayText(url, selectedControl->dir());
                auto item = new QTreeWidgetItem;
                item->setText(0, propertyName);
                classItem->addChild(item);
                setItemWidget(item, 1, createUrlHandlerWidget(propertyName, displayText, selectedControl));
                break;
            }

            case QVariant::Double: {
                if (propertyName == "x" || propertyName == "y" ||
                        propertyName == "width" || propertyName == "height") {
                    //                    if (propertyName == "x") //To make it called only once
                    //                        addGeometryChild(item, "geometry", selectedControl->rect(), true);
                } else {
                    //                    if (propertyName == "z")
                    //                        propertyMap[propertyName] = selectedControl->zValue();
                    const double value = propertyValue.value<double>();
                    //                    addChild(item, NodeType::Double, propertyName, value, value);
                }
                break;
            }

            case QVariant::Int: {
                if (propertyName == "x" || propertyName == "y" ||
                        propertyName == "width" || propertyName == "height") {
                    //                    if (propertyName == "x")
                    //                        addGeometryChild(item, "geometry", selectedControl->rect(), false);
                } else {
                    const int value = propertyValue.value<int>();
                    //                    addChild(item, NodeType::Int, propertyName, value, value);
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
            classItem->addChild(item);
            setItemWidget(item, 1, createEnumHandlerWidget(enumm, selectedControl));
        }

        expandItem(classItem);
    }

    //    filterList(m_searchEdit->text());

    verticalScrollBar()->setSliderPosition(verticalScrollBarPosition);
    horizontalScrollBar()->setSliderPosition(horizontalScrollBarPosition);
}

void PropertiesPane::drawBranches(QPainter* painter, const QRect& rect, const QModelIndex& index) const
{
    painter->setRenderHint(QPainter::Antialiasing);

    const qreal width = 10;
    const QAbstractItemModel* model = index.model();
    const bool hasChild = itemFromIndex(index)->childCount();
    const bool isClassRow = !model->parent(index).isValid() && index.row() > 2;

    QRectF handleRect(0, 0, width, width);
    handleRect.moveCenter(rect.center());
    handleRect.moveRight(rect.right() - 0.5);

    fillBackground(painter, rect, calculateVisibleRow(itemFromIndex(index), this), isClassRow, false);

    // Draw handle
    if (hasChild) {
        painter->setPen(isClassRow ? Qt::white : Qt::black);
        painter->setBrush(Qt::NoBrush);
        painter->drawRoundedRect(handleRect, 0, 0);

        painter->drawLine(QPointF(handleRect.left() + 2.5, handleRect.center().y()),
                          QPointF(handleRect.right() - 2.5, handleRect.center().y()));

        if (!isExpanded(index)) {
            painter->drawLine(QPointF(handleRect.center().x(), handleRect.top() + 2.5),
                              QPointF(handleRect.center().x(), handleRect.bottom() - 2.5));
        }
    }
}

void PropertiesPane::paintEvent(QPaintEvent* e)
{
    QPainter painter(viewport());

    /* Fill background */
    const qreal bandHeight = topLevelItemCount() ? rowHeight(indexFromItem(topLevelItem(0))) : 21;
    const qreal bandCount = viewport()->height() / bandHeight;
    const QList<Control*>& selectedControls = m_designerScene->selectedControls();

    painter.fillRect(rect(), "#fffefc");

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
    return QSize{340, 700};
}

#include "propertiespane.moc"
