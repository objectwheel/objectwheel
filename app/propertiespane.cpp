#include <propertiespane.h>
#include <centralwidget.h>
#include <css.h>
#include <saveutils.h>
#include <delayer.h>
#include <filemanager.h>
#include <focuslesslineedit.h>
#include <previewresult.h>
#include <control.h>
#include <designerscene.h>
#include <controlpreviewingmanager.h>
#include <windowmanager.h>
#include <mainwindow.h>
#include <inspectorpane.h>

#include <QtWidgets>

namespace {

enum NodeType {
    EnumType,
    FontFamily,
    FontWeight,
    FontPtSize,
    FontPxSize,
    FontBold,
    FontItalic,
    FontUnderline,
    FontOverline,
    FontStrikeout,
    Color,
    Bool,
    String,
    Id,
    Url,
    Double,
    Int,
    GeometryX,
    GeometryY,
    GeometryWidth,
    GeometryHeight,
    GeometryFX,
    GeometryFY,
    GeometryFWidth,
    GeometryFHeight
};

enum NodeRole {
    Type = Qt::UserRole + 1,
    Data
};

void processFont(QTreeWidgetItem* item, const QString& propertyName, const QMap<QString, QVariant>& map)
{
    const auto value = map.value(propertyName).value<QFont>();
    const auto px = value.pixelSize() > 0 ? true : false;
    const auto ft = QString::fromUtf8("[%1, %2%3]").arg(value.family())
            .arg(px ? value.pixelSize() : value.pointSize()).arg(px ? "px" : "pt");

    auto iitem = new QTreeWidgetItem;
    iitem->setText(0, propertyName);
    iitem->setText(1, ft);

    auto item0 = new QTreeWidgetItem;
    item0->setFlags(item0->flags() | Qt::ItemIsEditable);
    item0->setText(0, "family");
    item0->setData(1, Qt::EditRole, value.family());
    item0->setData(1, NodeRole::Type, NodeType::FontFamily);
    item0->setData(1, NodeRole::Data, value.family());
    iitem->addChild(item0);

    auto item1 = new QTreeWidgetItem;
    item1->setFlags(item1->flags() | Qt::ItemIsEditable);
    item1->setText(0, "weight");
    item1->setData(1, Qt::EditRole, QMetaEnum::fromType<QFont::Weight>().valueToKey(value.weight()));
    item1->setData(1, NodeRole::Type, NodeType::FontWeight);
    item1->setData(1, NodeRole::Data, value.weight());
    iitem->addChild(item1);

    auto item2 = new QTreeWidgetItem;
    item2->setFlags(item2->flags() | Qt::ItemIsEditable);
    item2->setText(0, "pointSize");
    item2->setData(1, Qt::EditRole, value.pointSize() < 0 ? 0 : value.pointSize());
    item2->setData(1, NodeRole::Type, NodeType::FontPtSize);
    item2->setData(1, NodeRole::Data, value.pointSize() < 0 ? 0 : value.pointSize());
    iitem->addChild(item2);

    auto item3 = new QTreeWidgetItem;
    item3->setFlags(item3->flags() | Qt::ItemIsEditable);
    item3->setText(0, "pixelSize");
    item3->setData(1, Qt::EditRole, value.pixelSize() < 0 ? 0 : value.pixelSize());
    item3->setData(1, NodeRole::Type, NodeType::FontPxSize);
    item3->setData(1, NodeRole::Data, value.pixelSize() < 0 ? 0 : value.pixelSize());
    iitem->addChild(item3);

    auto item4 = new QTreeWidgetItem;
    item4->setFlags(item4->flags() | Qt::ItemIsEditable);
    item4->setText(0, "bold");
    item4->setData(1, NodeRole::Type, NodeType::FontBold);
    item4->setData(1, NodeRole::Data, value.bold());
    iitem->addChild(item4);

    auto item5 = new QTreeWidgetItem;
    item5->setFlags(item5->flags() | Qt::ItemIsEditable);
    item5->setText(0, "italic");
    item5->setData(1, NodeRole::Type, NodeType::FontItalic);
    item5->setData(1, NodeRole::Data, value.italic());
    iitem->addChild(item5);

    auto item6 = new QTreeWidgetItem;
    item6->setFlags(item6->flags() | Qt::ItemIsEditable);
    item6->setText(0, "underline");
    item6->setData(1, NodeRole::Type, NodeType::FontUnderline);
    item6->setData(1, NodeRole::Data, value.underline());
    iitem->addChild(item6);

    auto item7 = new QTreeWidgetItem;
    item7->setFlags(item7->flags() | Qt::ItemIsEditable);
    item7->setText(0, "overline");
    item7->setData(1, NodeRole::Type, NodeType::FontOverline);
    item7->setData(1, NodeRole::Data, value.overline());
    iitem->addChild(item7);

    auto item8 = new QTreeWidgetItem;
    item8->setFlags(item8->flags() | Qt::ItemIsEditable);
    item8->setText(0, "strikeout");
    item8->setData(1, NodeRole::Type, NodeType::FontStrikeout);
    item8->setData(1, NodeRole::Data, value.strikeOut());
    iitem->addChild(item8);

    item->addChild(iitem);
}

void processGeometry(QTreeWidgetItem* item, const QString& propertyName, Control* control)
{
    const auto value = QRect(control->pos().toPoint(), control->size().toSize());
    const auto gt = QString::fromUtf8("[(%1, %2), %3 x %4]").
            arg(value.x()).arg(value.y()).arg(value.width()).arg(value.height());

    auto iitem = new QTreeWidgetItem;
    iitem->setText(0, propertyName);
    iitem->setText(1, gt);

    auto item1 = new QTreeWidgetItem;
    item1->setFlags(item1->flags() | Qt::ItemIsEditable);
    item1->setText(0, "x");
    item1->setData(1, Qt::EditRole, value.x());
    item1->setData(1, NodeRole::Type, NodeType::GeometryX);
    item1->setData(1, NodeRole::Data, value.x());
    iitem->addChild(item1);

    auto item2 = new QTreeWidgetItem;
    item2->setFlags(item2->flags() | Qt::ItemIsEditable);
    item2->setText(0, "y");
    item2->setData(1, Qt::EditRole, value.y());
    item2->setData(1, NodeRole::Type, NodeType::GeometryY);
    item2->setData(1, NodeRole::Data, value.y());
    iitem->addChild(item2);

    auto item3 = new QTreeWidgetItem;
    item3->setFlags(item3->flags() | Qt::ItemIsEditable);
    item3->setText(0, "width");
    item3->setData(1, Qt::EditRole, value.width());
    item3->setData(1, NodeRole::Type, NodeType::GeometryWidth);
    item3->setData(1, NodeRole::Data, value.width());
    iitem->addChild(item3);

    auto item4 = new QTreeWidgetItem;
    item4->setFlags(item4->flags() | Qt::ItemIsEditable);
    item4->setText(0, "height");
    item4->setData(1, Qt::EditRole, value.height());
    item4->setData(1, NodeRole::Type, NodeType::GeometryHeight);
    item4->setData(1, NodeRole::Data, value.height());
    iitem->addChild(item4);

    item->addChild(iitem);
}

void processGeometryF(QTreeWidgetItem* item, const QString& propertyName, Control* control)
{
    const auto value = QRectF(control->pos(), control->size());
    const auto gt = QString::fromUtf8("[(%1, %2), %3 x %4]").
            arg((int)value.x()).arg((int)value.y()).
            arg((int)value.width()).arg((int)value.height());

    auto iitem = new QTreeWidgetItem;
    iitem->setText(0, propertyName);
    iitem->setText(1, gt);

    auto item1 = new QTreeWidgetItem;
    item1->setFlags(item1->flags() | Qt::ItemIsEditable);
    item1->setText(0, "x");
    item1->setData(1, Qt::EditRole, value.x());
    item1->setData(1, NodeRole::Type, NodeType::GeometryFX);
    item1->setData(1, NodeRole::Data, value.x());
    iitem->addChild(item1);

    auto item2 = new QTreeWidgetItem;
    item2->setFlags(item2->flags() | Qt::ItemIsEditable);
    item2->setText(0, "y");
    item2->setData(1, Qt::EditRole, value.y());
    item2->setData(1, NodeRole::Type, NodeType::GeometryFY);
    item2->setData(1, NodeRole::Data, value.y());
    iitem->addChild(item2);

    auto item3 = new QTreeWidgetItem;
    item3->setFlags(item3->flags() | Qt::ItemIsEditable);
    item3->setText(0, "width");
    item3->setData(1, Qt::EditRole, value.width());
    item3->setData(1, NodeRole::Type, NodeType::GeometryFWidth);
    item3->setData(1, NodeRole::Data, value.width());
    iitem->addChild(item3);

    auto item4 = new QTreeWidgetItem;
    item4->setFlags(item4->flags() | Qt::ItemIsEditable);
    item4->setText(0, "height");
    item4->setData(1, Qt::EditRole, value.height());
    item4->setData(1, NodeRole::Type, NodeType::GeometryFHeight);
    item4->setData(1, NodeRole::Data, value.height());
    iitem->addChild(item4);

    item->addChild(iitem);
}

void processColor(QTreeWidgetItem* item, const QString& propertyName, const QMap<QString, QVariant>& map)
{
    const auto value = map.value(propertyName).value<QColor>();
    const auto cc = value.name(QColor::HexArgb);

    auto iitem = new QTreeWidgetItem;
    iitem->setText(0, propertyName);
    iitem->setData(1, Qt::EditRole, cc);
    iitem->setData(1, NodeRole::Data, value);
    iitem->setData(1, NodeRole::Type, NodeType::Color);
    iitem->setFlags(iitem->flags() | Qt::ItemIsEditable);

    item->addChild(iitem);
}

void processBool(QTreeWidgetItem* item, const QString& propertyName, const QMap<QString, QVariant>& map)
{
    const auto value = map.value(propertyName).value<bool>();

    auto iitem = new QTreeWidgetItem;
    iitem->setText(0, propertyName);
    iitem->setData(1, NodeRole::Data, value);
    iitem->setData(1, NodeRole::Type, NodeType::Bool);
    iitem->setFlags(iitem->flags() | Qt::ItemIsEditable);

    item->addChild(iitem);
}

void processString(QTreeWidgetItem* item, const QString& propertyName, const QMap<QString, QVariant>& map)
{
    const auto value = map.value(propertyName).value<QString>();

    auto iitem = new QTreeWidgetItem;
    iitem->setText(0, propertyName);
    iitem->setData(1, Qt::EditRole, value);
    iitem->setData(1, NodeRole::Data, value);
    iitem->setData(1, NodeRole::Type, NodeType::String);
    iitem->setFlags(iitem->flags() | Qt::ItemIsEditable);

    item->addChild(iitem);
}

void processUrl(QTreeWidgetItem* item, const QString& propertyName, const QString& relativePath, const QMap<QString, QVariant>& map)
{
    const auto value = map.value(propertyName).value<QUrl>();
    auto dispText = value.toDisplayString();
    if (value.isLocalFile())
        dispText = value.toLocalFile().remove(SaveUtils::toThisDir(relativePath) + separator());

    auto iitem = new QTreeWidgetItem;
    iitem->setText(0, propertyName);
    iitem->setData(1, Qt::EditRole, dispText);
    iitem->setData(1, NodeRole::Data, value);
    iitem->setData(1, NodeRole::Type, NodeType::Url);
    iitem->setFlags(iitem->flags() | Qt::ItemIsEditable);

    item->addChild(iitem);
}

void processDouble(QTreeWidgetItem* item, const QString& propertyName, const QMap<QString, QVariant>& map)
{
    const auto value = map.value(propertyName).value<double>();

    auto iitem = new QTreeWidgetItem;
    iitem->setText(0, propertyName);
    iitem->setData(1, Qt::EditRole, value);
    iitem->setData(1, NodeRole::Data, value);
    iitem->setData(1, NodeRole::Type, NodeType::Double);
    iitem->setFlags(iitem->flags() | Qt::ItemIsEditable);

    item->addChild(iitem);
}

void processInt(QTreeWidgetItem* item, const QString& propertyName, const QMap<QString, QVariant>& map)
{
    const auto value = map.value(propertyName).value<int>();

    auto iitem = new QTreeWidgetItem;
    iitem->setText(0, propertyName);
    iitem->setData(1, Qt::EditRole, value);
    iitem->setData(1, NodeRole::Data, value);
    iitem->setData(1, NodeRole::Type, NodeType::Int);
    iitem->setFlags(iitem->flags() | Qt::ItemIsEditable);

    item->addChild(iitem);
}
}
// FIXME: Fix this from scratch
// TODO: Visibility control->properties()'den alınmayacak, ParserWorker ile alınacak

Q_DECLARE_METATYPE(NodeType)

class PropertiesListDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit PropertiesListDelegate(PropertiesPane* parent) : QStyledItemDelegate(parent)
      , m_propertiesPane(parent)
    {}

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    void setEditorData(QWidget* ed, const QModelIndex& index) const override;
    void setModelData(QWidget* ed, QAbstractItemModel* model, const QModelIndex& index) const override;
    void updateEditorGeometry(QWidget* ed, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    QSize sizeHint(const QStyleOptionViewItem& opt, const QModelIndex& index) const override;

private:
    PropertiesPane* m_propertiesPane;
};

QWidget* PropertiesListDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& , const QModelIndex& index) const
{
    QWidget* ed = 0;

    if (index.column() == 0)
        return ed;

    auto type = index.data(NodeRole::Type).value<NodeType>();
    auto pIndex = m_propertiesPane->model()->index(index.row(), 0, index.parent());
    auto property = m_propertiesPane->model()->data(pIndex, Qt::DisplayRole).toString();

    switch (type) {
    case NodeType::EnumType: {
        auto editor = new QComboBox(parent);
        auto e = index.data(NodeRole::Data).value<Enum>();
        editor->addItems(e.keys.keys());
        connect(editor, static_cast<void(QComboBox::*)(int)>(&QComboBox::activated),
                [this, editor] () { ((PropertiesListDelegate*)this)->commitData(editor); });
        editor->setFocusPolicy(Qt::StrongFocus);
        ed = editor;
        break;
    }

    case NodeType::FontFamily: {
        auto editor = new QComboBox(parent);
        editor->addItems(QFontDatabase().families());
        connect(editor, static_cast<void(QComboBox::*)(int)>(&QComboBox::activated),
                [this, editor] () { ((PropertiesListDelegate*)this)->commitData(editor); });
        editor->setFocusPolicy(Qt::StrongFocus);
        ed = editor;
        break;
    }

    case NodeType::FontWeight: {
        auto editor = new QComboBox(parent);
        auto fenum = QMetaEnum::fromType<QFont::Weight>();
        for (int i = fenum.keyCount(); i--;)
            if (QString(fenum.key(i)).contains(QRegularExpression("[^\\r\\n\\t\\f\\v ]")))
                editor->addItem(fenum.key(i));
        connect(editor, static_cast<void(QComboBox::*)(int)>(&QComboBox::activated),
                [this, editor] () { ((PropertiesListDelegate*)this)->commitData(editor); });
        editor->setFocusPolicy(Qt::StrongFocus);
        ed = editor;
        break;
    }

    case NodeType::FontPtSize:
    case NodeType::FontPxSize: {
        auto editor = new QSpinBox(parent);
        connect(editor, &QSpinBox::editingFinished,
                [this, editor] () { ((PropertiesListDelegate*)this)->commitData(editor); });
        editor->setFocusPolicy(Qt::StrongFocus);
        editor->setMaximum(72);
        editor->setMinimum(0);
        ed = editor;
        break;
    }

    case NodeType::FontBold:
    case NodeType::FontItalic:
    case NodeType::FontUnderline:
    case NodeType::FontOverline:
    case NodeType::FontStrikeout:
    case NodeType::Bool: {
        auto editor = new QCheckBox(parent);
        connect(editor, &QCheckBox::toggled,
                [this, editor] () { ((PropertiesListDelegate*)this)->commitData(editor); });
        editor->setFocusPolicy(Qt::StrongFocus);
        ed = editor;
        break;
    }

    case NodeType::Color: {
        auto editor = new QToolButton(parent);
        editor->setFocusPolicy(Qt::StrongFocus);
        editor->setText("Change Color");
        ed = editor;

        connect(editor, &QCheckBox::clicked, [this, property, index] ()
        {
            auto color = index.data(NodeRole::Data).value<QColor>();
            color = QColorDialog::getColor(
                        Qt::white,
                        m_propertiesPane,
                        "Select Color",
                        QColorDialog::ShowAlphaChannel | QColorDialog::DontUseNativeDialog
                        );

            if (color.isValid()) {
                m_propertiesPane->model()->setData(index, color, NodeRole::Data);
                m_propertiesPane->model()->setData(index, color.name(QColor::HexArgb), Qt::EditRole);
                m_propertiesPane->saveChanges(property, "\"" + color.name(QColor::HexArgb) + "\"", color);
            }
        });
        break;
    }

    case NodeType::String: {
        auto editor = new QLineEdit(parent);
        connect(editor, &QLineEdit::editingFinished,
                [this, editor] () { ((PropertiesListDelegate*)this)->commitData(editor); });
        editor->setFocusPolicy(Qt::StrongFocus);
        ed = editor;
        break;
    }

    case NodeType::Id: {
        auto editor = new QLineEdit(parent);
        QRegExp rx("[a-z_][a-zA-Z0-9_]+");
        QValidator* validator = new QRegExpValidator(rx, editor);
        connect(editor, &QLineEdit::editingFinished,
                [this, editor] () { ((PropertiesListDelegate*)this)->commitData(editor); });
        editor->setValidator(validator);
        editor->setFocusPolicy(Qt::StrongFocus);
        ed = editor;
        break;
    }

    case NodeType::Url: {
        auto editor = new QLineEdit(parent);
        connect(editor, &QLineEdit::editingFinished,
                [this, editor] () { ((PropertiesListDelegate*)this)->commitData(editor); });
        editor->setFocusPolicy(Qt::StrongFocus);
        ed = editor;
        break;
    }

    case NodeType::Double: {
        auto editor = new QDoubleSpinBox(parent);
        connect(editor, &QDoubleSpinBox::editingFinished,
                [this, editor] () { ((PropertiesListDelegate*)this)->commitData(editor); });
        editor->setFocusPolicy(Qt::StrongFocus);
        if (property == "opacity") {
            editor->setMaximum(1.0);
            editor->setMinimum(0.0);
            editor->setSingleStep(0.1);
        } else {
            editor->setMaximum(std::numeric_limits<double>::max());
            editor->setMinimum(std::numeric_limits<double>::min());
        }
        ed = editor;
        break;
    }

    case NodeType::Int: {
        auto editor = new QSpinBox(parent);
        connect(editor, &QSpinBox::editingFinished,
                [this, editor] () { ((PropertiesListDelegate*)this)->commitData(editor); });
        editor->setFocusPolicy(Qt::StrongFocus);
        editor->setMaximum(std::numeric_limits<int>::max());
        editor->setMinimum(std::numeric_limits<int>::min());
        ed = editor;
        break;
    }

    case NodeType::GeometryX:
    case NodeType::GeometryY:
    case NodeType::GeometryWidth:
    case NodeType::GeometryHeight: {
        auto editor = new QSpinBox(parent);
        connect(editor, &QSpinBox::editingFinished,
                [this, editor] () { ((PropertiesListDelegate*)this)->commitData(editor); });
        editor->setFocusPolicy(Qt::StrongFocus);
        editor->setMaximum(std::numeric_limits<int>::max());
        editor->setMinimum(std::numeric_limits<int>::min());
        ed = editor;
        break;
    }

    case NodeType::GeometryFX:
    case NodeType::GeometryFY:
    case NodeType::GeometryFWidth:
    case NodeType::GeometryFHeight: {
        auto editor = new QDoubleSpinBox(parent);
        connect(editor, &QDoubleSpinBox::editingFinished,
                [this, editor] () { ((PropertiesListDelegate*)this)->commitData(editor); });
        editor->setFocusPolicy(Qt::StrongFocus);
        editor->setMaximum(std::numeric_limits<double>::max());
        editor->setMinimum(std::numeric_limits<double>::min());
        ed = editor;
        break;
    }

    default:
        break;
    }

    return ed;
}

void PropertiesListDelegate::setEditorData(QWidget* ed, const QModelIndex& index) const
{
    if (index.column() == 0)
        return;

    auto type = index.data(NodeRole::Type).value<NodeType>();

    switch (type) {
    case NodeType::EnumType: {
        auto val = index.model()->data(index, NodeRole::Data).value<Enum>();
        auto editor = static_cast<QComboBox*>(ed);
        editor->setCurrentText(val.value);
        break;
    }

    case NodeType::FontFamily: {
        auto val = index.model()->data(index, NodeRole::Data).value<QString>();
        auto editor = static_cast<QComboBox*>(ed);
        editor->setCurrentText(val);
        break;
    }

    case NodeType::FontWeight: {
        auto val = index.model()->data(index, NodeRole::Data).value<QFont::Weight>();
        auto editor = static_cast<QComboBox*>(ed);
        editor->setCurrentText(QMetaEnum::fromType<QFont::Weight>().valueToKey(val));
        break;
    }

    case NodeType::FontPtSize:
    case NodeType::FontPxSize: {
        auto val = index.model()->data(index, NodeRole::Data).value<int>();
        auto editor = static_cast<QSpinBox*>(ed);
        editor->setValue(val);
        break;
    }

    case NodeType::FontBold:
    case NodeType::FontItalic:
    case NodeType::FontUnderline:
    case NodeType::FontOverline:
    case NodeType::FontStrikeout:
    case NodeType::Bool: {
        auto val = index.model()->data(index, NodeRole::Data).value<bool>();
        auto editor = static_cast<QCheckBox*>(ed);
        editor->setChecked(val);
        break;
    }

    case NodeType::Id:
    case NodeType::String: {
        auto val = index.model()->data(index, NodeRole::Data).value<QString>();
        auto editor = static_cast<QLineEdit*>(ed);
        editor->setText(val);
        break;
    }

    case NodeType::Url: {
        auto selectedControl = m_propertiesPane->m_designerScene->selectedControls().at(0);
        auto val = index.model()->data(index, NodeRole::Data).value<QUrl>();
        auto editor = static_cast<QLineEdit*>(ed);
        auto dispText = val.toDisplayString();
        if (val.isLocalFile()) {
            dispText = val.toLocalFile().
                    remove(selectedControl->dir() + separator() + DIR_THIS + separator());
        }
        editor->setText(dispText);
        break;
    }

    case NodeType::Double: {
        auto val = index.model()->data(index, NodeRole::Data).value<double>();
        auto editor = static_cast<QDoubleSpinBox*>(ed);
        editor->setValue(val);
        break;
    }

    case NodeType::Int: {
        auto val = index.model()->data(index, NodeRole::Data).value<int>();
        auto editor = static_cast<QSpinBox*>(ed);
        editor->setValue(val);
        break;
    }

    case NodeType::GeometryX:
    case NodeType::GeometryY:
    case NodeType::GeometryWidth:
    case NodeType::GeometryHeight: {
        auto val = index.model()->data(index, NodeRole::Data).value<int>();
        auto editor = static_cast<QSpinBox*>(ed);
        editor->setValue(val);
        break;
    }

    case NodeType::GeometryFX:
    case NodeType::GeometryFY:
    case NodeType::GeometryFWidth:
    case NodeType::GeometryFHeight: {
        auto val = index.model()->data(index, NodeRole::Data).value<double>();
        auto editor = static_cast<QDoubleSpinBox*>(ed);
        editor->setValue(val);
        break;
    }

    default:
        break;
    }
}

void PropertiesListDelegate::setModelData(QWidget* ed, QAbstractItemModel* model, const QModelIndex& index) const
{
    if (index.column() == 0)
        return;

    QVariant val;
    auto type = index.data(NodeRole::Type).value<NodeType>();
    auto pIndex = model->index(index.row(), 0, index.parent());
    auto property = model->data(pIndex, Qt::DisplayRole).toString();

    switch (type) {
    case NodeType::EnumType: {
        auto editor = static_cast<QComboBox*>(ed);
        val = editor->currentText();
        auto preVal = model->data(index, Qt::EditRole).toString();

        if (val == preVal)
            return;

        auto e = index.data(NodeRole::Data).value<Enum>();
        e.value = val.toString();

        model->setData(index, QVariant::fromValue(e), NodeRole::Data);
        model->setData(index, e.value, Qt::EditRole);

        m_propertiesPane->saveChanges(property, e.scope + "." + e.value, e.keys.value(e.value));
        break;
    }

    case NodeType::FontFamily: {
        auto editor = static_cast<QComboBox*>(ed);
        val = editor->currentText();
        auto preVal = model->data(index, Qt::EditRole).toString();

        if (val == preVal)
            return;

        model->setData(index, val, NodeRole::Data);
        model->setData(index, val, Qt::EditRole);

        // Update parent node
        auto pIndex = model->index(index.parent().row(), 1, index.parent().parent());
        auto pVal = model->data(pIndex, Qt::DisplayRole).toString();
        pVal.replace(preVal, val.toString());
        model->setData(pIndex, pVal, Qt::DisplayRole);

        m_propertiesPane->saveChanges("font." + property, "\"" + val.toString() + "\"", val);
        break;
    }

    case NodeType::FontWeight: {
        auto editor = static_cast<QComboBox*>(ed);
        val = editor->currentText();
        auto preVal = model->data(index, Qt::EditRole).toString();

        if (val == preVal)
            return;

        auto fenum = QMetaEnum::fromType<QFont::Weight>();

        model->setData(index, fenum.keyToValue(val.toString().toUtf8().data()), NodeRole::Data);
        model->setData(index, val, Qt::EditRole);

        m_propertiesPane->saveChanges("font." + property, "Font." + val.toString(), val);
        break;
    }

    case NodeType::FontPtSize:
    case NodeType::FontPxSize: {
        auto editor = static_cast<QSpinBox*>(ed);
        val = editor->value();
        auto preVal = model->data(index, Qt::EditRole).toString();

        if (val == preVal)
            return;

        model->setData(index, val, NodeRole::Data);
        model->setData(index, val, Qt::EditRole);

        // Update parent node
        int pxSize, ptSize;
        if (type == NodeType::FontPtSize) {
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
        m_propertiesPane->saveChanges("font." + property, val.toString(), val);
        break;
    }

    case NodeType::FontBold:
    case NodeType::FontItalic:
    case NodeType::FontUnderline:
    case NodeType::FontOverline:
    case NodeType::FontStrikeout: {
        auto editor = static_cast<QCheckBox*>(ed);
        val = editor->isChecked();
        auto preVal = model->data(index, NodeRole::Data).toBool();

        if (val == preVal)
            return;

        model->setData(index, val, NodeRole::Data);
        m_propertiesPane->saveChanges("font." + property, val.toString(), val);
        break;
    }

    case NodeType::Bool: {
        auto editor = static_cast<QCheckBox*>(ed);
        val = editor->isChecked();
        auto preVal = model->data(index, NodeRole::Data).toBool();

        if (val == preVal)
            return;

        model->setData(index, val, NodeRole::Data);
        m_propertiesPane->saveChanges(property, val.toString(), val);
        break;
    }

    case NodeType::Id: {
        auto editor = static_cast<QLineEdit*>(ed);
        val = editor->text();
        auto preVal = model->data(index, Qt::EditRole).toString();

        if (val == preVal)
            return;

        model->setData(index, val, NodeRole::Data);
        model->setData(index, val, Qt::EditRole);
        m_propertiesPane->saveChanges(property, val.toString(), val);
        break;
    }

    case NodeType::String: {
        auto editor = static_cast<QLineEdit*>(ed);
        val = editor->text();
        auto preVal = model->data(index, Qt::EditRole).toString();

        if (val == preVal)
            return;

        model->setData(index, val, NodeRole::Data);
        model->setData(index, val, Qt::EditRole);
        m_propertiesPane->saveChanges(property, "\"" +val.toString() + "\"", val);
        break;
    }

    case NodeType::Url: {
        auto editor = static_cast<QLineEdit*>(ed);
        val = QUrl(editor->text());
        auto preVal = model->data(index, Qt::EditRole).toUrl();

        if (val == preVal)
            return;

        model->setData(index, val, NodeRole::Data);
        model->setData(index, val, Qt::EditRole);
        m_propertiesPane->saveChanges(property, "\"" + val.toString() + "\"", val);
        break;
    }

    case NodeType::Double: {
        auto editor = static_cast<QDoubleSpinBox*>(ed);
        val = editor->value();
        auto preVal = model->data(index, NodeRole::Data).toDouble();

        if (val == preVal)
            return;

        model->setData(index, val, NodeRole::Data);
        model->setData(index, val, Qt::EditRole);
        if (property == "z") {
            auto sc = m_propertiesPane->m_designerScene->selectedControls();
            if (sc.size() == 1)
                sc[0]->setZValue(val.toReal());
        }
        m_propertiesPane->saveChanges(property, val.toString(), val);
        break;
    }

    case NodeType::Int: {
        auto editor = static_cast<QSpinBox*>(ed);
        val = editor->value();
        auto preVal = model->data(index, NodeRole::Data).toInt();

        if (val == preVal)
            return;

        model->setData(index, val, NodeRole::Data);
        model->setData(index, val, Qt::EditRole);
        m_propertiesPane->saveChanges(property, val.toString(), val);
        break;
    }

    case NodeType::GeometryX:
    case NodeType::GeometryY:
    case NodeType::GeometryWidth:
    case NodeType::GeometryHeight: {
        auto editor = static_cast<QSpinBox*>(ed);
        val = editor->value();
        auto preVal = model->data(index, NodeRole::Data).toInt();

        if (val == preVal)
            return;

        model->setData(index, val, NodeRole::Data);
        model->setData(index, val, Qt::EditRole);

        // Update parent node
        auto pIndex = model->index(index.parent().row(), 1, index.parent().parent());
        auto x = model->data(pIndex.child(0, 1), Qt::DisplayRole).toInt();
        auto y = model->data(pIndex.child(1, 1), Qt::DisplayRole).toInt();
        auto w = model->data(pIndex.child(2, 1), Qt::DisplayRole).toInt();
        auto h = model->data(pIndex.child(3, 1), Qt::DisplayRole).toInt();

        const auto gt = QString::fromUtf8("[(%1, %2), %3 x %4]").
                arg(x).arg(y).arg(w).arg(h);

        model->setData(pIndex, gt, Qt::DisplayRole);
        m_propertiesPane->saveChanges(property, val.toString(), val);
        break;
    }

    case NodeType::GeometryFX:
    case NodeType::GeometryFY:
    case NodeType::GeometryFWidth:
    case NodeType::GeometryFHeight: {
        auto editor = static_cast<QDoubleSpinBox*>(ed);
        val = editor->value();
        auto preVal = model->data(index, NodeRole::Data).toReal();

        if (val == preVal)
            return;

        model->setData(index, val, NodeRole::Data);
        model->setData(index, val, Qt::EditRole);

        // Update parent node
        auto pIndex = model->index(index.parent().row(), 1, index.parent().parent());
        auto x = model->data(pIndex.child(0, 1), Qt::DisplayRole).toReal();
        auto y = model->data(pIndex.child(1, 1), Qt::DisplayRole).toReal();
        auto w = model->data(pIndex.child(2, 1), Qt::DisplayRole).toReal();
        auto h = model->data(pIndex.child(3, 1), Qt::DisplayRole).toReal();

        const auto gt = QString::fromUtf8("[(%1, %2), %3 x %4]").
                arg(x).arg(y).arg(w).arg(h);

        model->setData(pIndex, gt, Qt::DisplayRole);
        m_propertiesPane->saveChanges(property, val.toString(), val);
        break;
    }

    default:
        break;
    }
}

void PropertiesListDelegate::updateEditorGeometry(QWidget* ed, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QStyledItemDelegate::updateEditorGeometry(ed, option, index);

    auto type = index.data(NodeRole::Type).value<NodeType>();

    switch (type) {
    case 0:
    default:
        ed->setGeometry(ed->geometry().adjusted(0, 0, -1, -1));
        break;
    }
}

QSize PropertiesListDelegate::sizeHint(const QStyleOptionViewItem& opt, const QModelIndex& index) const
{
    const auto& size = QStyledItemDelegate::sizeHint(opt, index);
    return QSize(size.width(), 22);
}

void PropertiesListDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QRectF cellRect(option.rect);
    auto model = index.model();
    bool isClassRow = !model->parent(index).isValid() && index.row() > 2;
    bool isSelected = option.state & QStyle::State_Selected;

    // Draw background
    if (isClassRow)
        painter->setBrush(QColor("#9D7650"));
    else if (isSelected)
        painter->setBrush(QColor("#ebd5c0"));
    else if (index.row() % 2)
        painter->setBrush(QColor("#faf1e8"));
    else
        painter->setBrush(QColor("#fffefc"));

    painter->setRenderHint(QPainter::Antialiasing);
    painter->setPen(Qt::NoPen);
    painter->drawRect(cellRect);

    // Draw bottom & middle cell lines
    painter->setPen("#D6CEC6");
    painter->drawLine(cellRect.bottomLeft() + QPointF(0.5, -0.5), cellRect.bottomRight() + QPointF(-0.5, -0.5));

    if (index.column() == 0)
        if (!isClassRow)
            painter->drawLine(cellRect.topRight() + QPointF(-0.5, 0.5), cellRect.bottomRight() + QPointF(-0.5, -0.5));


    // Draw data
    if (index.column() == 0) {
        if (isClassRow)
            painter->setPen(Qt::white);
        else
            painter->setPen(Qt::black);

        painter->drawText(
                    cellRect.adjusted(5, 0, 0, 0),
                    index.data(Qt::EditRole).toString(), QTextOption(Qt::AlignLeft | Qt::AlignVCenter)
                    );
    } else {
        if (!isClassRow) {
            painter->setPen(Qt::black);

            painter->drawText(
                        cellRect.adjusted(5, 0, 0, 0),
                        index.data(Qt::EditRole).toString(), QTextOption(Qt::AlignLeft | Qt::AlignVCenter)
                        );
        }

        auto type = index.data(NodeRole::Type).value<NodeType>();
        switch (type) {
        case NodeType::FontBold:
        case NodeType::FontItalic:
        case NodeType::FontUnderline:
        case NodeType::FontOverline:
        case NodeType::FontStrikeout:
        case NodeType::Bool: {
            bool value = index.data(NodeRole::Data).value<bool>();
            QStyleOptionButton opt;
            opt.initFrom(m_propertiesPane);
            opt.rect = option.rect;
            opt.state = QStyle::State_Active | QStyle::State_Enabled;
            opt.state |= value ? QStyle::State_On : QStyle::State_Off;
            qApp->style()->drawControl(QStyle::CE_CheckBox, &opt, painter, m_propertiesPane);
            break;
        }

        default:
            break;
        }
    }
}

PropertiesPane::PropertiesPane(DesignerScene* designerScene, QWidget* parent) : QTreeWidget(parent)
  , m_designerScene(designerScene)
  , m_searchEdit(new FocuslessLineEdit(this))
{

    QPalette p(palette());
    p.setColor(backgroundRole(), "#ececec");
    setAutoFillBackground(true);
    setPalette(p);

    QPalette p2(palette());
    p2.setColor(QPalette::All, QPalette::Base, QColor("#fffefc"));
    p2.setColor(QPalette::All, QPalette::Highlight, QColor("#ebd5c0"));
    p2.setColor(QPalette::All, QPalette::Text, Qt::black);
    p2.setColor(QPalette::All, QPalette::HighlightedText, Qt::black);
    setPalette(p2);

    setColumnCount(2);
    setIndentation(16);
    setDragEnabled(false);
    setUniformRowHeights(true);
    setDropIndicatorShown(false);
    setExpandsOnDoubleClick(true);
    setItemDelegate(new PropertiesListDelegate(this));
    setAttribute(Qt::WA_MacShowFocusRect, false);
    setSelectionBehavior(QTreeWidget::SelectRows);
    setSelectionMode(QTreeWidget::SingleSelection);
    setEditTriggers(QAbstractItemView::AllEditTriggers);
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
    connect(m_searchEdit, SIGNAL(textChanged(QString)), SLOT(filterList(QString)));
    setViewportMargins(0, 0, 0, 30);

    /* Prepare Properties Widget */
    connect(m_designerScene, SIGNAL(selectionChanged()), SLOT(handleSelectionChange()));
    // BUG   connect(ControlMonitoringManager::instance(), SIGNAL(geometryChanged(Control*)), SLOT(handleSelectionChange()));
    // BUG   connect(ControlMonitoringManager::instance(), SIGNAL(zValueChanged(Control*)), SLOT(handleSelectionChange()));
}

void PropertiesPane::sweep()
{
    clearList();
    m_searchEdit->clear();
}

void PropertiesPane::clearList()
{
    for (int i = 0; i < topLevelItemCount(); ++i)
        qDeleteAll(topLevelItem(i)->takeChildren());

    clear();
}

// FIXME: x, y, width, height, z properties are not shown (because we removed them within previewer)
void PropertiesPane::refreshList()
{
    int vsp = verticalScrollBar()->sliderPosition();
    int hsp = horizontalScrollBar()->sliderPosition();

    clearList();

    auto selectedControls = m_designerScene->selectedControls();

    if (selectedControls.size() != 1)
        return;

    setDisabled(selectedControls[0]->hasErrors());

    auto propertyNodes = selectedControls[0]->properties();
    if (propertyNodes.isEmpty())
        return;

    {
        QTreeWidgetItem* item_1 = new QTreeWidgetItem;
        item_1->setText(0, "Type");
        item_1->setText(1, propertyNodes.first().cleanClassName);

        QTreeWidgetItem* item_2 = new QTreeWidgetItem;
        item_2->setText(0, "uid");
        item_2->setText(1, selectedControls[0]->uid());

        QTreeWidgetItem* item_3 = new QTreeWidgetItem;
        item_3->setText(0, "id");
        item_3->setData(1, Qt::EditRole, selectedControls[0]->id());
        item_3->setData(1, NodeRole::Data, selectedControls[0]->id());
        item_3->setData(1, NodeRole::Type, NodeType::Id);
        item_3->setFlags(item_3->flags() | Qt::ItemIsEditable);

        addTopLevelItem(item_1);
        addTopLevelItem(item_2);
        addTopLevelItem(item_3);
    }

    for (const auto& propertyNode : propertyNodes) {
        auto map = propertyNode.properties;
        auto enums = propertyNode.enums;

        if (map.isEmpty() && enums.isEmpty())
            continue;

        auto item = new QTreeWidgetItem;
        item->setText(0, propertyNode.cleanClassName);

        for (const auto& propertyName : map.keys()) {
            switch (map.value(propertyName).type()) {
            case QVariant::Font: {
                processFont(item, propertyName, map);
                break;
            }

            case QVariant::Color: {
                processColor(item, propertyName, map);
                break;
            }

            case QVariant::Bool: {
                processBool(item, propertyName, map);
                break;
            }

            case QVariant::String: {
                processString(item, propertyName, map);
                break;
            }

            case QVariant::Url: {
                processUrl(item, propertyName, m_designerScene->selectedControls().first()->dir(), map);
                break;
            }

            case QVariant::Double: {
                if (propertyName == "x" || propertyName == "y" ||
                        propertyName == "width" || propertyName == "height") {
                    if (propertyName == "x") //To make it called only once
                        processGeometryF(item, "geometry", selectedControls[0]);
                } else {
                    if (propertyName == "z")
                        map[propertyName] = selectedControls[0]->zValue();
                    processDouble(item, propertyName, map);
                }
                break;
            }

            case QVariant::Int: {
                if (propertyName == "x" || propertyName == "y" ||
                        propertyName == "width" || propertyName == "height") {
                    if (propertyName == "x")
                        processGeometry(item, "geometry", selectedControls[0]);
                } else {
                    processInt(item, propertyName, map);
                }
                break;
            }

            default: {
                continue;
                // QTreeWidgetItem* iitem = new QTreeWidgetItem;
                // iitem->setText(0, propertyName);
                // iitem->setText(1, map.value(propertyName).typeName());
                // item->addChild(iitem);
                // break;
            }
            }
        }

        for (auto e : enums) {
            auto item1 = new QTreeWidgetItem;
            item1->setFlags(item1->flags() | Qt::ItemIsEditable);
            item1->setText(0, e.name);
            item1->setData(1, Qt::EditRole, e.value);
            item1->setData(1, NodeRole::Type, NodeType::EnumType);
            item1->setData(1, NodeRole::Data, QVariant::fromValue(e));
            item->addChild(item1);
        }

        addTopLevelItem(item);
        expandItem(item);
    }

    filterList(m_searchEdit->text());

    verticalScrollBar()->setSliderPosition(vsp);
    horizontalScrollBar()->setSliderPosition(hsp);
}

void PropertiesPane::handleSelectionChange()
{
    auto selectedControls = m_designerScene->selectedControls();

    if (selectedControls.size() != 1) {
        clearList();
        return;
    }

    refreshList();
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

void PropertiesPane::saveChanges(const QString& property, const QString& parserValue, const QVariant& value)
{
    auto scs = m_designerScene->selectedControls();

    if (scs.isEmpty())
        return;

    auto sc = scs.at(0);

    const QString& previousId = sc->id();

    // FIXME   SaveManager::setProperty(sc, property, parserValue);

    qDebug() << property << value;

    if (property == "id") {
        ControlPreviewingManager::scheduleIdUpdate(sc->uid(), value.toString());
        WindowManager::mainWindow()->inspectorPane()->handleControlIdChange(sc, previousId);
    } else {
        ControlPreviewingManager::schedulePropertyUpdate(sc->uid(), property, value);
    }
}

void PropertiesPane::drawBranches(QPainter* painter, const QRect& rect, const QModelIndex& index) const
{
    qreal width = 10;
    auto model = index.model();
    bool hasChild = itemFromIndex(index)->childCount();
    bool isClassRow = !model->parent(index).isValid() && index.row() > 2;
    bool isSelected = itemFromIndex(index)->isSelected();

    QRectF branchRect(rect);
    QRectF handleRect(0, 0, width, width);

    handleRect.moveCenter(branchRect.center());
    handleRect.moveRight(branchRect.right() - 0.5);

    // Draw background
    if (isClassRow)
        painter->setBrush(QColor("#9D7650"));
    else if (isSelected)
        painter->setBrush(QColor("#ebd5c0"));
    else if (index.row() % 2)
        painter->setBrush(QColor("#faf1e8"));
    else
        painter->setBrush(QColor("#fffefc"));

    painter->setPen(Qt::NoPen);
    painter->drawRect(branchRect);

    // Draw bottom & middle cell lines
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setPen("#D6CEC6");
    painter->drawLine(branchRect.bottomLeft() + QPointF(0.5, -0.5), branchRect.bottomRight() + QPointF(-0.5, -0.5));

    // Draw handle
    if (hasChild) {
        painter->setPen(isClassRow ? Qt::white : Qt::black);
        painter->setBrush(Qt::NoBrush);
        painter->drawRoundedRect(handleRect, 1.5, 1.5);

        painter->drawLine(
                    QPointF(handleRect.left() + 2.5, handleRect.center().y()),
                    QPointF(handleRect.right() - 2.5, handleRect.center().y())
                    );

        if (!isExpanded(index)) {
            painter->drawLine(
                        QPointF(handleRect.center().x(), handleRect.top() + 2.5),
                        QPointF(handleRect.center().x(), handleRect.bottom() - 2.5)
                        );
        }
    }
}

void PropertiesPane::paintEvent(QPaintEvent* e)
{
    QPainter painter(viewport());
    painter.setRenderHint(QPainter::Antialiasing);
    if (topLevelItemCount() == 0) {
        auto sc = m_designerScene->selectedControls();
        bool drawn = false;
        const qreal ic = viewport()->height() / 20; // WARNING: Constant 20?
        for (int i = 0; i < ic; i++) {
            if (i % 2) {
                painter.fillRect(0, i * 20, viewport()->width(),
                                 20, QColor("#faf1e8"));
            } else if (!drawn && (i == int(ic) / 2 ||
                                  i - 1 == int(ic) / 2 || i + 1 == int(ic) / 2)) {
                drawn = true;
                painter.setPen(QColor(sc.size() == 1 ?
                                          "#d98083" : "#b5aea7"));
                painter.drawText(0, i * 20, viewport()->width(),
                                 20, Qt::AlignCenter, sc.size() == 1 ?
                                     "Control has errors" : "No controls selected");
            }
        }

    } else {
        const auto tli = topLevelItem(topLevelItemCount() - 1);
        const auto lci = tli->child(tli->childCount() - 1);
        auto lcir = visualItemRect(lci);

        if (lcir.isNull())
            lcir = visualItemRect(tli);

        const qreal ic = (viewport()->height() + qAbs(lcir.y())) / (qreal) lcir.height();

        for (int i = 0; i < ic; i++) {
            if (i % 2) {
                painter.fillRect(
                            0,
                            lcir.y() + i * lcir.height(),
                            viewport()->width(),
                            lcir.height(),
                            QColor("#faf1e8")
                            );
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
