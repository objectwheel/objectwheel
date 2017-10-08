#include <propertieswidget.h>
#include <propertyitem.h>
#include <fit.h>
#include <toolboxtree.h>
#include <designmanager.h>
#include <css.h>
#include <savemanager.h>
#include <delayer.h>

#include <QtWidgets>

using namespace Fit;

enum NodeType {
    FontFamily,
    FontPtSize,
    FontPxSize,
    FontBold,
    FontItalic,
    FontUnderline,
    FontOverline,
    FontStrikeout,
    Color
};
Q_DECLARE_METATYPE(NodeType)

enum NodeRole {
    Type = Qt::UserRole + 1,
    Data
};

class ColorDelegate : public QStyledItemDelegate
{
        Q_OBJECT
        enum { BrushRole = 33 };
    public:
        explicit ColorDelegate(QTreeWidget* view, QObject* parent = 0);

        QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem &option,
                              const QModelIndex &index) const override;

        void setEditorData(QWidget* ed, const QModelIndex &index) const override;
        void setModelData(QWidget* ed, QAbstractItemModel* model,
                          const QModelIndex &index) const override;

        void updateEditorGeometry(QWidget* ed, const QStyleOptionViewItem &option,
                                  const QModelIndex &index) const override;

        void paint(QPainter* painter, const QStyleOptionViewItem &opt,
                   const QModelIndex &index) const override;
        QSize sizeHint(const QStyleOptionViewItem &opt, const QModelIndex &index) const override;

    public slots:
        void saveChanges(const NodeType& type, const QVariant& value) const;

    private:
        QTreeWidget* m_view;
};

ColorDelegate::ColorDelegate(QTreeWidget* view, QObject* parent) :
    QStyledItemDelegate(parent),
    m_view(view)
{
}

QWidget* ColorDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem &,
    const QModelIndex &index) const
{
    QWidget* ed = 0;

    if (index.column() == 0)
        return ed;

    auto type = index.data(NodeRole::Type).value<NodeType>();

    switch (type) {
        case FontFamily: {
            auto editor = new QComboBox(parent);
            editor->addItems(QFontDatabase().families());
            connect(editor, static_cast<void(QComboBox::*)(int)>(&QComboBox::activated),
                [this, editor] () { ((ColorDelegate*)this)->commitData(editor); });
            editor->setFocusPolicy(Qt::StrongFocus);
            ed = editor;
            break;
        }

        case FontPtSize:
        case FontPxSize: {
            auto editor = new QSpinBox(parent);
            connect(editor, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),
                [this, editor] () { ((ColorDelegate*)this)->commitData(editor); });
            editor->setFocusPolicy(Qt::StrongFocus);
            editor->setMaximum(72);
            editor->setMinimum(0);
            ed = editor;
            break;
        }

        case FontBold:
        case FontItalic:
        case FontUnderline:
        case FontOverline:
        case FontStrikeout: {
            auto editor = new QCheckBox(parent);
            connect(editor, &QCheckBox::toggled,
                [this, editor] () { ((ColorDelegate*)this)->commitData(editor); });
            editor->setFocusPolicy(Qt::StrongFocus);
            ed = editor;
            break;
        }

        case Color: {
            auto editor = new QToolButton(parent);
            editor->setFocusPolicy(Qt::StrongFocus);
            editor->setText("Change Color");
            ed = editor;

            connect(editor, &QCheckBox::clicked, [this, type, index] ()
            {
                auto color = index.data(NodeRole::Data).value<QColor>();
                color = QColorDialog::getColor(color, m_view, "Choose Color",
                    QColorDialog::ShowAlphaChannel | QColorDialog::DontUseNativeDialog);
                if (color.isValid()) {
                    m_view->model()->setData(index, color, NodeRole::Data);
                    m_view->model()->setData(index, color.name(QColor::HexArgb), Qt::EditRole);
                    saveChanges(type, color);
                }
            });
            break;
        }

        default:
            break;
    }

    return ed;
}

void ColorDelegate::setEditorData(QWidget* ed, const QModelIndex &index) const
{
    if (index.column() == 0)
        return;

    auto type = index.data(NodeRole::Type).value<NodeType>();

    switch (type) {
        case FontFamily: {
            auto val = index.model()->data(index, NodeRole::Data).value<QString>();
            auto editor = static_cast<QComboBox*>(ed);
            editor->setCurrentText(val);
            break;
        }

        case FontPtSize:
        case FontPxSize: {
            auto val = index.model()->data(index, NodeRole::Data).value<int>();
            auto editor = static_cast<QSpinBox*>(ed);
            editor->setValue(val);
            break;
        }

        case FontBold:
        case FontItalic:
        case FontUnderline:
        case FontOverline:
        case FontStrikeout: {
            auto val = index.model()->data(index, NodeRole::Data).value<bool>();
            auto editor = static_cast<QCheckBox*>(ed);
            editor->setChecked(val);
            break;
        }

        default:
            break;
    }
}

void ColorDelegate::setModelData(QWidget* ed, QAbstractItemModel* model,
                                 const QModelIndex &index) const
{
    if (index.column() == 0)
        return;

    QVariant val;
    auto type = index.data(NodeRole::Type).value<NodeType>();

    switch (type) {
        case FontFamily: {
            auto editor = static_cast<QComboBox*>(ed);
            val = editor->currentText();
            auto preVal = model->data(index, Qt::EditRole).toString();
            model->setData(index, val, NodeRole::Data);
            model->setData(index, val, Qt::EditRole);

            // Update parent node
            auto pIndex = model->index(index.parent().row(), 1, index.parent().parent());
            auto pVal = model->data(pIndex, Qt::DisplayRole).toString();
            pVal.replace(preVal, val.toString());
            model->setData(pIndex, pVal, Qt::DisplayRole);
            break;
        }

        case FontPtSize:
        case FontPxSize: {
            auto editor = static_cast<QSpinBox*>(ed);
            val = editor->value();
            model->setData(index, val, NodeRole::Data);
            model->setData(index, val, Qt::EditRole);

            // Update parent node
            int pxSize, ptSize;
            if (type == FontPtSize) {
                auto bIndex = model->index(index.row() + 1, 1, index.parent());
                pxSize = model->data(bIndex, NodeRole::Data).toInt();
                ptSize = val.toInt();
            } else {
                auto bIndex = model->index(index.row() - 1, 1, index.parent());
                ptSize = model->data(bIndex, NodeRole::Data).toInt();
                pxSize = val.toInt();
            }
            bool px = pxSize > 0 ? true : false;
            auto pIndex = model->index(index.parent().row(), 1, index.parent().parent());
            auto pVal = model->data(pIndex, Qt::DisplayRole).toString();
            pVal.replace(QRegExp(",.*"), ", " + QString::number(px ? pxSize : ptSize) + (px ? "px]" : "pt]"));
            model->setData(pIndex, pVal, Qt::DisplayRole);
            break;
        }

        case FontBold:
        case FontItalic:
        case FontUnderline:
        case FontOverline:
        case FontStrikeout: {
            auto editor = static_cast<QCheckBox*>(ed);
            val = editor->isChecked();
            model->setData(index, val, NodeRole::Data);
            break;
        }

        default:
            break;
    }
    saveChanges(type, val);
}

static void processFont(QTreeWidgetItem* item, const QString& propertyName, PropertyMap& map)
{
    const auto value = map[propertyName].value<QFont>();
    const auto px = value.pixelSize() > 0 ? true : false;
    const auto ft = QString::fromUtf8("[%1, %2%3]").arg(value.family())
        .arg(px ? value.pixelSize() : value.pointSize()).arg(px ? "px" : "pt");

    auto iitem = new QTreeWidgetItem;
    iitem->setText(0, propertyName);
    iitem->setText(1, ft);

    auto item1 = new QTreeWidgetItem;
    item1->setFlags(item1->flags() | Qt::ItemIsEditable);
    item1->setText(0, "Family");
    item1->setData(1, Qt::EditRole, value.family());
    item1->setData(1, NodeRole::Type, NodeType::FontFamily);
    item1->setData(1, NodeRole::Data, value.family());
    iitem->addChild(item1);

    auto item2 = new QTreeWidgetItem;
    item2->setFlags(item2->flags() | Qt::ItemIsEditable);
    item2->setText(0, "Point size");
    item2->setData(1, Qt::EditRole, value.pointSize() < 0 ? 0 : value.pointSize());
    item2->setData(1, NodeRole::Type, NodeType::FontPtSize);
    item2->setData(1, NodeRole::Data, value.pointSize() < 0 ? 0 : value.pointSize());
    iitem->addChild(item2);

    auto item3 = new QTreeWidgetItem;
    item3->setFlags(item3->flags() | Qt::ItemIsEditable);
    item3->setText(0, "Pixel size");
    item3->setData(1, Qt::EditRole, value.pixelSize() < 0 ? 0 : value.pixelSize());
    item3->setData(1, NodeRole::Type, NodeType::FontPxSize);
    item3->setData(1, NodeRole::Data, value.pixelSize() < 0 ? 0 : value.pixelSize());
    iitem->addChild(item3);

    auto item4 = new QTreeWidgetItem;
    item4->setFlags(item4->flags() | Qt::ItemIsEditable);
    item4->setText(0, "Bold");
    item4->setData(1, NodeRole::Type, NodeType::FontBold);
    item4->setData(1, NodeRole::Data, value.bold());
    iitem->addChild(item4);

    auto item5 = new QTreeWidgetItem;
    item5->setFlags(item5->flags() | Qt::ItemIsEditable);
    item5->setText(0, "Italic");
    item5->setData(1, NodeRole::Type, NodeType::FontItalic);
    item5->setData(1, NodeRole::Data, value.italic());
    iitem->addChild(item5);

    auto item6 = new QTreeWidgetItem;
    item6->setFlags(item6->flags() | Qt::ItemIsEditable);
    item6->setText(0, "Underline");
    item6->setData(1, NodeRole::Type, NodeType::FontUnderline);
    item6->setData(1, NodeRole::Data, value.underline());
    iitem->addChild(item6);

    auto item7 = new QTreeWidgetItem;
    item7->setFlags(item7->flags() | Qt::ItemIsEditable);
    item7->setText(0, "Overline");
    item7->setData(1, NodeRole::Type, NodeType::FontOverline);
    item7->setData(1, NodeRole::Data, value.overline());
    iitem->addChild(item7);

    auto item8 = new QTreeWidgetItem;
    item8->setFlags(item8->flags() | Qt::ItemIsEditable);
    item8->setText(0, "Strikeout");
    item8->setData(1, NodeRole::Type, NodeType::FontStrikeout);
    item8->setData(1, NodeRole::Data, value.strikeOut());
    iitem->addChild(item8);

    item->addChild(iitem);
}

static void processColor(QTreeWidgetItem* item, const QString& propertyName, PropertyMap& map)
{
    const auto value = map[propertyName].value<QColor>();
    const auto cc = value.name(QColor::HexArgb);

    auto iitem = new QTreeWidgetItem;
    iitem->setText(0, propertyName);
    iitem->setData(1, Qt::EditRole, cc);
    iitem->setData(1, NodeRole::Data, value);
    iitem->setData(1, NodeRole::Type, NodeType::Color);
    iitem->setFlags(iitem->flags() | Qt::ItemIsEditable);

    item->addChild(iitem);
}

void ColorDelegate::updateEditorGeometry(QWidget* ed,
    const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyledItemDelegate::updateEditorGeometry(ed, option, index);

    auto type = index.data(NodeRole::Type).value<NodeType>();

    switch (type) {
        default:
            ed->setGeometry(ed->geometry().adjusted(0, 0, -1, -1));
            break;
    }
}

void ColorDelegate::paint(QPainter* painter, const QStyleOptionViewItem &opt,
                          const QModelIndex &index) const
{
    QStyleOptionViewItem option = opt;
    const QAbstractItemModel* model = index.model();
    Q_ASSERT(model);

    if (model->parent(index).isValid()) {
        if (index.row() % 2) {
            if (index.column() == 0) {
                QRectF mrect(option.rect);
                mrect.setX(0.5);
                painter->fillRect(mrect, QColor("#EDF3FE"));
            } else {
                painter->fillRect(option.rect, QColor("#EDF3FE"));
            }
        }
    } else {
        painter->fillRect(option.rect, QColor("#C5C9CC"));
        option.palette.setColor(QPalette::Highlight, QColor("#C5C9CC"));
    }

    if (index.column() == 0) {
        QRectF branchRect = QRectF(0, option.rect.top(),
                                   option.rect.x(), option.rect.height());

        QBrush branchColor = option.palette.base();
        if(option.state & QStyle::State_Selected) {
            branchColor = option.palette.highlight();
        } else {
            if (!model->parent(index).isValid())
                branchColor = QColor("#C5C9CC");
            else if (index.row() % 2)
                branchColor = QColor("#EDF3FE");
        }
        painter->fillRect(branchRect, branchColor);

        if (model->rowCount(index)) {
            static const int i = 9; // ### hardcoded in qcommonstyle.cpp
            QRect r = option.rect;
            QStyleOption branchOption;
            branchOption.rect = QRect(r.left() - i,
                                      r.top() + (r.height() - i)/2, i, i);
            branchOption.state = QStyle::State_Children;

            if (m_view->isExpanded(index))
                branchOption.state |= QStyle::State_Open;

            qApp->style()->drawPrimitive(QStyle::PE_IndicatorBranch, &branchOption, painter, m_view);
        }
    }

    const bool mask = qvariant_cast<bool>(index.model()->data(index, Qt::EditRole));
    if (!model->parent(index).isValid() && mask) {
        option.font.setBold(true);
    }

    QStyledItemDelegate::paint(painter, option, index);

    auto type = index.data(NodeRole::Type).value<NodeType>();
    QStyleOptionButton eoption;
    eoption.initFrom(m_view);
    eoption.rect = option.rect;

    switch (type) {
        case FontBold:
        case FontItalic:
        case FontUnderline:
        case FontOverline:
        case FontStrikeout: {
            bool value = index.data(NodeRole::Data).value<bool>();
            eoption.state |= value ? QStyle::State_On : QStyle::State_Off;
            m_view->style()->drawControl(QStyle::CE_CheckBox, &eoption, painter, m_view);
            break;
        }

        default:
            break;
    }

    const QColor color = static_cast<QRgb>(qApp->style()->styleHint(QStyle::SH_Table_GridLineColor, &option));
    const QPen oldPen = painter->pen();
    painter->setPen(QPen(color));

    if (index.column() == 0) {
        painter->drawLine(QPointF(0.5, option.rect.y() + 0.5),
                          QPointF(0.5, option.rect.bottom() + 0.5));
        if (model->parent(index).isValid()) {
            painter->drawLine(option.rect.right() + 0.5, option.rect.y() + 0.5,
                              option.rect.right() + 0.5, option.rect.bottom() + 0.5);
        }
    } else {
        painter->drawLine(option.rect.right() + 0.5, option.rect.y() + 0.5,
                          option.rect.right() + 0.5, option.rect.bottom() + 0.5);
    }

    painter->drawLine(QPointF(0.5, option.rect.bottom() + 0.5),
                      QPointF(option.rect.right() + 0.5, option.rect.bottom() + 0.5));
    painter->setPen(oldPen);

}

QSize ColorDelegate::sizeHint(const QStyleOptionViewItem &opt, const QModelIndex &index) const
{
    return QStyledItemDelegate::sizeHint(opt, index) + QSize(4, 4);
}

void ColorDelegate::saveChanges(const NodeType& type, const QVariant& value) const
{
    auto selectedControl = DesignManager::currentScene()->selectedControls().at(0);

    switch (type) {
        case FontFamily:
            SaveManager::setProperty(selectedControl, "font.family", value);
            break;

        case FontPtSize:
            SaveManager::setProperty(selectedControl, "font.pointSize", value);
            break;

        case FontPxSize:
            SaveManager::setProperty(selectedControl, "font.pixelSize", value);
            break;

        case FontBold:
            SaveManager::setProperty(selectedControl, "font.bold", value);
            break;

        case FontItalic:
            SaveManager::setProperty(selectedControl, "font.italic", value);
            break;

        case FontUnderline:
            SaveManager::setProperty(selectedControl, "font.underline", value);
            break;

        case FontOverline:
            SaveManager::setProperty(selectedControl, "font.overline", value);
            break;

        case FontStrikeout:
            SaveManager::setProperty(selectedControl, "font.strikeout", value);
            break;

        case Color:
            SaveManager::setProperty(selectedControl, "color", value);
            break;

        default:
            break;
    }

    QMetaObject::Connection connection;
    connection = connect(SaveManager::instance(), &SaveManager::parserRunningChanged,
      [selectedControl, connection] {
        if (SaveManager::parserWorking() == false) {
            selectedControl->refresh();
            disconnect(connection);
        }
    });
}

static void cleanProperties(PropertyMap& map)
{
    for (auto key : map.keys()) {
        if (key.startsWith("__") ||
            QString::fromUtf8(map.value(key).typeName()).isEmpty() ||
            QString::fromUtf8(map.value(key).typeName()).
            contains(QRegExp("Q([A-Za-z_][A-Za-z0-9_]*)\\*")))
            map.remove(key);
    }
}

PropertiesWidget::PropertiesWidget(QWidget* parent) : QWidget(parent)
{
    setAutoFillBackground(true);
    QPalette p(palette());
    p.setColor(QPalette::Window, QColor("#E0E4E7"));
    setPalette(p);

    QPalette p2(_treeWidget.palette());
    p2.setColor(QPalette::Base, QColor("#F3F7FA"));
    p2.setColor(QPalette::Highlight, QColor("#E0E4E7"));
    p2.setColor(QPalette::Text, QColor("#202427"));
    p2.setColor(QPalette::HighlightedText, QColor("#202427"));
    _treeWidget.setPalette(p2);

    _treeWidget.setHorizontalScrollMode(ToolboxTree::ScrollPerPixel);
    _treeWidget.setVerticalScrollMode(ToolboxTree::ScrollPerPixel);
    _treeWidget.setSelectionBehavior(ToolboxTree::SelectRows);
    _treeWidget.setFocusPolicy(Qt::NoFocus);
    _treeWidget.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    _treeWidget.setEditTriggers(QAbstractItemView::AllEditTriggers);
    _treeWidget.setDragEnabled(false);
    _treeWidget.setDropIndicatorShown(false);
    _treeWidget.setColumnCount(2);
    _treeWidget.headerItem()->setText(0, "Property");
    _treeWidget.headerItem()->setText(1, "Value");
    _treeWidget.verticalScrollBar()->setStyleSheet(CSS::ScrollBar);
    _treeWidget.horizontalScrollBar()->setStyleSheet(CSS::ScrollBarH);
    _treeWidget.setIndentation(fit(10));
    _treeWidget.setItemDelegate(new ColorDelegate(&_treeWidget, &_treeWidget));
    _treeWidget.header()->resizeSection(0, fit(170));

    _layout.setSpacing(fit(2));
    _layout.setContentsMargins(fit(3), fit(3), fit(3), fit(3));

    _searchEdit.setPlaceholderText("Filter");
    connect(&_searchEdit, SIGNAL(textEdited(QString)), SLOT(refreshList()));

    _layout.addWidget(&_searchEdit);
    _layout.addWidget(&_treeWidget);

    setLayout(&_layout);

    /* Prepare Properties Widget */
    connect(DesignManager::formScene(), SIGNAL(selectionChanged()), SLOT(handleSelectionChange()));
    connect(DesignManager::controlScene(), SIGNAL(selectionChanged()), SLOT(handleSelectionChange()));
    connect(DesignManager::instance(), SIGNAL(modeChanged()), SLOT(handleSelectionChange()));
}

void PropertiesWidget::clearList()
{
    for (int i = 0; i < _treeWidget.topLevelItemCount(); i++) {
        auto item = _treeWidget.topLevelItem(i);
        auto itemWidget = _treeWidget.itemWidget(item, 1);
        if (itemWidget)
            itemWidget->deleteLater();
    }
    _treeWidget.clear();
}

void PropertiesWidget::refreshList()
{
    clearList();

    auto selectedControls = DesignManager::currentScene()->selectedControls();

    if (selectedControls.size() != 1)
        return;

    auto properties = selectedControls[0]->properties();

    {
        QTreeWidgetItem* item = new QTreeWidgetItem;
        item->setText(0, "Type");
        {
            QTreeWidgetItem* iitem = new QTreeWidgetItem;
            iitem->setText(0, "Type");
            iitem->setText(1, properties.first().first);
            item->addChild(iitem);

            QTreeWidgetItem* jitem = new QTreeWidgetItem;
            jitem->setText(0, "id");
            jitem->setText(1, selectedControls[0]->id());
            item->addChild(jitem);
        }
        _treeWidget.addTopLevelItem(item);
        _treeWidget.expandItem(item);
    }

    for (auto property : properties) {
        PropertyMap map = property.second;
        cleanProperties(map);

        if (map.isEmpty())
            continue;

        QTreeWidgetItem* item = new QTreeWidgetItem;
        item->setText(0, property.first);

        for (auto propertyName : map.keys()) {
            switch (map[propertyName].type())
            {
                case QVariant::Font: {
                    processFont(item, propertyName, map);
                    break;
                }

                case QVariant::Color: {
                    processColor(item, propertyName, map);
                    break;
                }

                default: {
                    QTreeWidgetItem* iitem = new QTreeWidgetItem;
                    iitem->setText(0, propertyName);
                    iitem->setText(1, map[propertyName].typeName());
                    item->addChild(iitem);
                    break;
                }
            }
        }
        _treeWidget.addTopLevelItem(item);
        _treeWidget.expandItem(item);
    }
}

void PropertiesWidget::handleSelectionChange()
{
    auto selectedControls = DesignManager::currentScene()->selectedControls();

    if (selectedControls.size() != 1) {
        clearList();
        return;
    }

    refreshList();
}

QSize PropertiesWidget::sizeHint() const
{
    return QSize(fit(300), fit(400));
}

#include "propertieswidget.moc"
