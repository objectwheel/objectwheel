#include <propertiespane.h>
#include <fit.h>
#include <designerwidget.h>
#include <css.h>
#include <savebackend.h>
#include <delayer.h>
#include <filemanager.h>
#include <controlwatcher.h>
#include <filterlineedit.h>
#include <previewbackend.h>
#include <control.h>
#include <frontend.h>
#include <controlscene.h>
#include <formscene.h>
#include <mainwindow.h>
#include <QtWidgets>

//!
//! *************************** [global] ***************************
//!

enum NodeType {
    FontFamily,
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
Q_DECLARE_METATYPE(NodeType)

enum NodeRole {
    Type = Qt::UserRole + 1,
    Data
};

static void processFont(QTreeWidgetItem* item, const QString& propertyName, const PropertyMap& map)
{
    const auto value = map.value(propertyName).value<QFont>();
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

static void processGeometry(QTreeWidgetItem* item, const QString& propertyName, Control* control)
{
    const auto value = QRect(control->pos().toPoint(), control->size().toSize());
    const auto gt = QString::fromUtf8("[(%1, %2), %3 x %4]").
                    arg(value.x()).arg(value.y()).arg(value.width()).arg(value.height());

    auto iitem = new QTreeWidgetItem;
    iitem->setText(0, propertyName);
    iitem->setText(1, gt);

    auto item1 = new QTreeWidgetItem;
    item1->setFlags(item1->flags() | Qt::ItemIsEditable);
    item1->setText(0, "X");
    item1->setData(1, Qt::EditRole, value.x());
    item1->setData(1, NodeRole::Type, NodeType::GeometryX);
    item1->setData(1, NodeRole::Data, value.x());
    iitem->addChild(item1);

    auto item2 = new QTreeWidgetItem;
    item2->setFlags(item2->flags() | Qt::ItemIsEditable);
    item2->setText(0, "Y");
    item2->setData(1, Qt::EditRole, value.y());
    item2->setData(1, NodeRole::Type, NodeType::GeometryY);
    item2->setData(1, NodeRole::Data, value.y());
    iitem->addChild(item2);

    auto item3 = new QTreeWidgetItem;
    item3->setFlags(item3->flags() | Qt::ItemIsEditable);
    item3->setText(0, "Width");
    item3->setData(1, Qt::EditRole, value.width());
    item3->setData(1, NodeRole::Type, NodeType::GeometryWidth);
    item3->setData(1, NodeRole::Data, value.width());
    iitem->addChild(item3);

    auto item4 = new QTreeWidgetItem;
    item4->setFlags(item4->flags() | Qt::ItemIsEditable);
    item4->setText(0, "Height");
    item4->setData(1, Qt::EditRole, value.height());
    item4->setData(1, NodeRole::Type, NodeType::GeometryHeight);
    item4->setData(1, NodeRole::Data, value.height());
    iitem->addChild(item4);

    item->addChild(iitem);
}

static void processGeometryF(QTreeWidgetItem* item, const QString& propertyName, Control* control)
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
    item1->setText(0, "X");
    item1->setData(1, Qt::EditRole, value.x());
    item1->setData(1, NodeRole::Type, NodeType::GeometryFX);
    item1->setData(1, NodeRole::Data, value.x());
    iitem->addChild(item1);

    auto item2 = new QTreeWidgetItem;
    item2->setFlags(item2->flags() | Qt::ItemIsEditable);
    item2->setText(0, "Y");
    item2->setData(1, Qt::EditRole, value.y());
    item2->setData(1, NodeRole::Type, NodeType::GeometryFY);
    item2->setData(1, NodeRole::Data, value.y());
    iitem->addChild(item2);

    auto item3 = new QTreeWidgetItem;
    item3->setFlags(item3->flags() | Qt::ItemIsEditable);
    item3->setText(0, "Width");
    item3->setData(1, Qt::EditRole, value.width());
    item3->setData(1, NodeRole::Type, NodeType::GeometryFWidth);
    item3->setData(1, NodeRole::Data, value.width());
    iitem->addChild(item3);

    auto item4 = new QTreeWidgetItem;
    item4->setFlags(item4->flags() | Qt::ItemIsEditable);
    item4->setText(0, "Height");
    item4->setData(1, Qt::EditRole, value.height());
    item4->setData(1, NodeRole::Type, NodeType::GeometryFHeight);
    item4->setData(1, NodeRole::Data, value.height());
    iitem->addChild(item4);

    item->addChild(iitem);
}

static void processColor(QTreeWidgetItem* item, const QString& propertyName, const PropertyMap& map)
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

static void processBool(QTreeWidgetItem* item, const QString& propertyName, const PropertyMap& map)
{
    const auto value = map.value(propertyName).value<bool>();

    auto iitem = new QTreeWidgetItem;
    iitem->setText(0, propertyName);
    iitem->setData(1, NodeRole::Data, value);
    iitem->setData(1, NodeRole::Type, NodeType::Bool);
    iitem->setFlags(iitem->flags() | Qt::ItemIsEditable);

    item->addChild(iitem);
}

static void processString(QTreeWidgetItem* item, const QString& propertyName, const PropertyMap& map)
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

static void processUrl(QTreeWidgetItem* item, const QString& propertyName, const PropertyMap& map)
{
    auto selectedControl = dW->currentScene()->selectedControls().at(0);
    const auto value = map.value(propertyName).value<QUrl>();
    auto dispText = value.toDisplayString();
    if (value.isLocalFile()) {
        dispText = value.toLocalFile().
                   remove(selectedControl->dir() + separator() + DIR_THIS + separator());
    }

    auto iitem = new QTreeWidgetItem;
    iitem->setText(0, propertyName);
    iitem->setData(1, Qt::EditRole, dispText);
    iitem->setData(1, NodeRole::Data, value);
    iitem->setData(1, NodeRole::Type, NodeType::Url);
    iitem->setFlags(iitem->flags() | Qt::ItemIsEditable);

    item->addChild(iitem);
}

static void processDouble(QTreeWidgetItem* item, const QString& propertyName, const PropertyMap& map)
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

static void processInt(QTreeWidgetItem* item, const QString& propertyName, const PropertyMap& map)
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

static void saveChanges(const QString& property, const QVariant& value)
{
    auto scs = dW->currentScene()->selectedControls();

    if (scs.isEmpty())
        return;

    QPointer<Control> sc = scs.at(0);

    if (dW->mode() == ControlGui && property == TAG_ID)
        SaveBackend::setProperty(sc, property, value,
                                 dW->controlScene()->mainControl()->dir());
    else
        SaveBackend::setProperty(sc, property, value);

    QMetaObject::Connection con;
    con = QObject::connect(SaveBackend::instance(),
      &SaveBackend::parserRunningChanged, [sc, con] {
        if (sc.isNull()) {
            QObject::disconnect(con);
            return;
        }
        if (SaveBackend::parserWorking() == false) {
            sc->refresh();
            QObject::disconnect(con);
        }
    });
}

static void saveChanges(const NodeType& type, const QVariant& value)
{
    QString property;
    switch (type) {
        case FontFamily:
            property = "font.family";
            break;

        case FontPtSize:
            property = "font.pointSize";
            break;

        case FontPxSize:
            property = "font.pixelSize";
            break;

        case FontBold:
            property = "font.bold";
            break;

        case FontItalic:
            property = "font.italic";
            break;

        case FontUnderline:
            property = "font.underline";
            break;

        case FontOverline:
            property = "font.overline";
            break;

        case FontStrikeout:
            property = "font.strikeout";
            break;

        case GeometryX:
        case GeometryFX: {
            auto sc = dW->currentScene()->selectedControls();
            if (sc.size() != 1)
                return;
            sc[0]->setX(value.toReal());
            return;
        }

        case GeometryY:
        case GeometryFY: {
            auto sc = dW->currentScene()->selectedControls();
            if (sc.size() != 1)
                return;
            sc[0]->setY(value.toReal());
            return;
        }

        case GeometryWidth:
        case GeometryFWidth: {
            auto sc = dW->currentScene()->selectedControls();
            if (sc.size() != 1)
                return;
            sc[0]->resize(value.toReal(), sc[0]->size().height());
            return;
        }

        case GeometryHeight:
        case GeometryFHeight: {
            auto sc = dW->currentScene()->selectedControls();
            if (sc.size() != 1)
                return;
            sc[0]->resize(sc[0]->size().width(), value.toReal());
            return;
        }

        default:
            break;
    }
    saveChanges(property, value);
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

//!
//! ********************** [PropertiesDelegate] **********************
//!

class PropertiesDelegate : public QStyledItemDelegate
{
        Q_OBJECT

    public:
        explicit PropertiesDelegate(QTreeWidget* view, QObject* parent = 0);

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

    private:
        QTreeWidget* m_view;
};

PropertiesDelegate::PropertiesDelegate(QTreeWidget* view, QObject* parent)
    : QStyledItemDelegate(parent)
    , m_view(view)
{
}

QWidget* PropertiesDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem &,
                                          const QModelIndex &index) const
{
    QWidget* ed = 0;

    if (index.column() == 0)
        return ed;

    auto type = index.data(NodeRole::Type).value<NodeType>();
    auto pIndex = m_view->model()->index(index.row(), 0, index.parent());
    auto property = m_view->model()->data(pIndex, Qt::DisplayRole).toString();

    switch (type) {
        case FontFamily: {
            auto editor = new QComboBox(parent);
            editor->addItems(QFontDatabase().families());
            connect(editor, static_cast<void(QComboBox::*)(int)>(&QComboBox::activated),
                    [this, editor] () { ((PropertiesDelegate*)this)->commitData(editor); });
            editor->setFocusPolicy(Qt::StrongFocus);
            ed = editor;
            break;
        }

        case FontPtSize:
        case FontPxSize: {
            auto editor = new QSpinBox(parent);
            connect(editor, &QSpinBox::editingFinished,
                    [this, editor] () { ((PropertiesDelegate*)this)->commitData(editor); });
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
        case FontStrikeout:
        case Bool: {
            auto editor = new QCheckBox(parent);
            connect(editor, &QCheckBox::toggled,
                    [this, editor] () { ((PropertiesDelegate*)this)->commitData(editor); });
            editor->setFocusPolicy(Qt::StrongFocus);
            ed = editor;
            break;
        }

        case Color: {
            auto editor = new QToolButton(parent);
            editor->setFocusPolicy(Qt::StrongFocus);
            editor->setText("Change Color");
            ed = editor;

            connect(editor, &QCheckBox::clicked, [this, property, index] ()
            {
                auto color = index.data(NodeRole::Data).value<QColor>();
                color = QColorDialog::getColor(color, m_view, "Choose Color",
                                               QColorDialog::ShowAlphaChannel | QColorDialog::DontUseNativeDialog);
                if (color.isValid()) {
                    m_view->model()->setData(index, color, NodeRole::Data);
                    m_view->model()->setData(index, color.name(QColor::HexArgb), Qt::EditRole);
                    saveChanges(property, color);
                }
            });
            break;
        }

        case String: {
            auto editor = new QLineEdit(parent);
            connect(editor, &QLineEdit::editingFinished,
                    [this, editor] () { ((PropertiesDelegate*)this)->commitData(editor); });
            editor->setFocusPolicy(Qt::StrongFocus);
            ed = editor;
            break;
        }

        case Id: {
            auto editor = new QLineEdit(parent);
            QRegExp rx("[a-z_][a-zA-Z0-9_]+");
            QValidator* validator = new QRegExpValidator(rx, editor);
            connect(editor, &QLineEdit::editingFinished,
                    [this, editor] () { ((PropertiesDelegate*)this)->commitData(editor); });
            editor->setValidator(validator);
            editor->setFocusPolicy(Qt::StrongFocus);
            ed = editor;
            break;
        }

        case Url: {
            auto editor = new QLineEdit(parent);
            connect(editor, &QLineEdit::editingFinished,
                    [this, editor] () { ((PropertiesDelegate*)this)->commitData(editor); });
            editor->setFocusPolicy(Qt::StrongFocus);
            ed = editor;
            break;
        }

        case Double: {
            auto editor = new QDoubleSpinBox(parent);
            connect(editor, &QDoubleSpinBox::editingFinished,
                    [this, editor] () { ((PropertiesDelegate*)this)->commitData(editor); });
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

        case Int: {
            auto editor = new QSpinBox(parent);
            connect(editor, &QSpinBox::editingFinished,
                    [this, editor] () { ((PropertiesDelegate*)this)->commitData(editor); });
            editor->setFocusPolicy(Qt::StrongFocus);
            editor->setMaximum(std::numeric_limits<int>::max());
            editor->setMinimum(std::numeric_limits<int>::min());
            ed = editor;
            break;
        }

        case GeometryX:
        case GeometryY:
        case GeometryWidth:
        case GeometryHeight: {
            auto editor = new QSpinBox(parent);
            connect(editor, &QSpinBox::editingFinished,
                    [this, editor] () { ((PropertiesDelegate*)this)->commitData(editor); });
            editor->setFocusPolicy(Qt::StrongFocus);
            editor->setMaximum(std::numeric_limits<int>::max());
            editor->setMinimum(std::numeric_limits<int>::min());
            ed = editor;
            break;
        }

        case GeometryFX:
        case GeometryFY:
        case GeometryFWidth:
        case GeometryFHeight: {
            auto editor = new QDoubleSpinBox(parent);
            connect(editor, &QDoubleSpinBox::editingFinished,
                    [this, editor] () { ((PropertiesDelegate*)this)->commitData(editor); });
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

void PropertiesDelegate::setEditorData(QWidget* ed, const QModelIndex &index) const
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
        case FontStrikeout:
        case Bool: {
            auto val = index.model()->data(index, NodeRole::Data).value<bool>();
            auto editor = static_cast<QCheckBox*>(ed);
            editor->setChecked(val);
            break;
        }

        case Id:
        case String: {
            auto val = index.model()->data(index, NodeRole::Data).value<QString>();
            auto editor = static_cast<QLineEdit*>(ed);
            editor->setText(val);
            break;
        }

        case Url: {
            auto selectedControl = dW->currentScene()->selectedControls().at(0);
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

        case Double: {
            auto val = index.model()->data(index, NodeRole::Data).value<double>();
            auto editor = static_cast<QDoubleSpinBox*>(ed);
            editor->setValue(val);
            break;
        }

        case Int: {
            auto val = index.model()->data(index, NodeRole::Data).value<int>();
            auto editor = static_cast<QSpinBox*>(ed);
            editor->setValue(val);
            break;
        }

        case GeometryX:
        case GeometryY:
        case GeometryWidth:
        case GeometryHeight: {
            auto val = index.model()->data(index, NodeRole::Data).value<int>();
            auto editor = static_cast<QSpinBox*>(ed);
            editor->setValue(val);
            break;
        }

        case GeometryFX:
        case GeometryFY:
        case GeometryFWidth:
        case GeometryFHeight: {
            auto val = index.model()->data(index, NodeRole::Data).value<double>();
            auto editor = static_cast<QDoubleSpinBox*>(ed);
            editor->setValue(val);
            break;
        }

        default:
            break;
    }
}

void PropertiesDelegate::setModelData(QWidget* ed, QAbstractItemModel* model,
                                      const QModelIndex &index) const
{
    if (index.column() == 0)
        return;

    QVariant val;
    auto type = index.data(NodeRole::Type).value<NodeType>();
    auto pIndex = model->index(index.row(), 0, index.parent());
    auto property = model->data(pIndex, Qt::DisplayRole).toString();

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
            saveChanges(type, val);
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
            saveChanges(type, val);
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
            saveChanges(type, val);
            break;
        }

        case Bool: {
            auto editor = static_cast<QCheckBox*>(ed);
            val = editor->isChecked();
            model->setData(index, val, NodeRole::Data);
            saveChanges(property, val);
            break;
        }

        case Id:
        case String: {
            auto editor = static_cast<QLineEdit*>(ed);
            val = editor->text();
            model->setData(index, val, NodeRole::Data);
            model->setData(index, val, Qt::EditRole);
            saveChanges(property, val);
            break;
        }

        case Url: {
            auto editor = static_cast<QLineEdit*>(ed);
            val = QUrl(editor->text());
            model->setData(index, val, NodeRole::Data);
            model->setData(index, val, Qt::EditRole);
            saveChanges(property, val);
            break;
        }

        case Double: {
            auto editor = static_cast<QDoubleSpinBox*>(ed);
            val = editor->value();
            model->setData(index, val, NodeRole::Data);
            model->setData(index, val, Qt::EditRole);
            if (property == "z") {
                auto sc = dW->currentScene()->selectedControls();
                if (sc.size() == 1)
                    sc[0]->setZValue(val.toReal());
            }
            saveChanges(property, val);
            break;
        }

        case Int: {
            auto editor = static_cast<QSpinBox*>(ed);
            val = editor->value();
            model->setData(index, val, NodeRole::Data);
            model->setData(index, val, Qt::EditRole);
            saveChanges(property, val);
            break;
        }

        case GeometryX:
        case GeometryY:
        case GeometryWidth:
        case GeometryHeight: {
            auto editor = static_cast<QSpinBox*>(ed);
            val = editor->value();
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
            saveChanges(type, val);
            break;
        }

        case GeometryFX:
        case GeometryFY:
        case GeometryFWidth:
        case GeometryFHeight: {
            auto editor = static_cast<QDoubleSpinBox*>(ed);
            val = editor->value();
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
            saveChanges(type, val);
            break;
        }

        default:
            break;
    }
}

void PropertiesDelegate::updateEditorGeometry(QWidget* ed,
  const QStyleOptionViewItem &option, const QModelIndex &index) const
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

void PropertiesDelegate::paint(QPainter* painter, const QStyleOptionViewItem &opt,
  const QModelIndex &index) const
{
    QStyleOptionViewItem option = opt;
    const QAbstractItemModel* model = index.model();
    Q_ASSERT(model);
    painter->setRenderHint(QPainter::Antialiasing);

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
        painter->fillRect(option.rect, QColor("#d5d9dC"));
        option.palette.setColor(QPalette::Highlight, QColor("#d5d9dC"));
    }

    if (index.column() == 0) {
        QRectF branchRect = QRectF(0, option.rect.top(),
                                   option.rect.x(), option.rect.height());

        QBrush branchColor = option.palette.base();
        if(option.state & QStyle::State_Selected) {
            branchColor = option.palette.highlight();
        } else {
            if (!model->parent(index).isValid())
                branchColor = QColor("#d5d9dC");
            else if (index.row() % 2)
                branchColor = QColor("#EDF3FE");
        }
        painter->fillRect(branchRect, branchColor);

        if (model->rowCount(index)) {
            static const int i = fit::fx(9); // ### hardcoded in qcommonstyle.cpp
            QRect r = option.rect;
            QStyleOption branchOption;
            branchOption.rect = QRect(r.left() - i,
                                      r.top() + (r.height() - i)/2, i, i);
            branchOption.state = QStyle::State_Children;

            if (m_view->isExpanded(index))
                branchOption.state |= QStyle::State_Open;

            qApp->style()->drawPrimitive(QStyle::PE_IndicatorBranch,
              &branchOption, painter, m_view);
        }
    }

    const bool mask = qvariant_cast<bool>
      (index.model()->data(index, Qt::EditRole));
    if (!model->parent(index).isValid() && mask) {
        option.font.setWeight(QFont::Medium);
    }

    QStyledItemDelegate::paint(painter, option, index);

    auto type = index.data(NodeRole::Type).value<NodeType>();
    switch (type) {
        case FontBold:
        case FontItalic:
        case FontUnderline:
        case FontOverline:
        case FontStrikeout:
        case Bool: {
            bool value = index.data(NodeRole::Data).value<bool>();
            QStyleOptionButton opt;
            opt.initFrom(m_view);
            opt.rect = option.rect;
            opt.state = QStyle::State_Active | QStyle::State_Enabled;
            opt.state |= value ? QStyle::State_On : QStyle::State_Off;
            m_view->style()->drawControl(QStyle::CE_CheckBox, &opt, painter, m_view);
            break;
        }

        default:
            break;
    }

    const QColor color = static_cast<QRgb>(qApp->style()->styleHint(QStyle::SH_Table_GridLineColor, &option));
    const QPen oldPen = painter->pen();
    QPen pen(color);
    pen.setWidthF(fit::fx(1));
    painter->setPen(pen);

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

    if (!m_view->isEnabled()) {
        painter->fillRect(option.rect, QColor("04000000"));
    }
}

QSize PropertiesDelegate::sizeHint(const QStyleOptionViewItem &opt, const QModelIndex &index) const
{
    return QStyledItemDelegate::sizeHint(opt, index) + QSize(4, 4);
}

//!
//! *********************** [PropertiesPane] ***********************
//!

PropertiesPane::PropertiesPane(MainWindow* parent) : QWidget(parent)
{
    _layout = new QVBoxLayout(this);
    _treeWidget = new QTreeWidget;
    _searchEdit = new FilterLineEdit;

    QPalette p(palette());
    p.setColor(backgroundRole(), "#E0E4E7");
    setAutoFillBackground(true);
    setPalette(p);

    QPalette p2(_treeWidget->palette());
    p2.setColor(QPalette::Base, "#F3F7FA");
    p2.setColor(QPalette::Highlight, "#E0E4E7");
    p2.setColor(QPalette::Text, "#202427");
    p2.setColor(QPalette::HighlightedText, "#202427");
    _treeWidget->setPalette(p2);

    _treeWidget->setColumnCount(2);
    _treeWidget->setDragEnabled(false);
    _treeWidget->setFocusPolicy(Qt::NoFocus);
    _treeWidget->setIndentation(fit::fx(10));
    _treeWidget->setDropIndicatorShown(false);
    _treeWidget->headerItem()->setText(1, "Value");
    _treeWidget->headerItem()->setText(0, "Property");
    _treeWidget->viewport()->installEventFilter(this);
    _treeWidget->header()->resizeSection(0, fit::fx(170));
    _treeWidget->setSelectionBehavior(QTreeWidget::SelectRows);
    _treeWidget->verticalScrollBar()->setStyleSheet(CSS::ScrollBar);
    _treeWidget->setVerticalScrollMode(QTreeWidget::ScrollPerPixel);
    _treeWidget->setEditTriggers(QAbstractItemView::AllEditTriggers);
    _treeWidget->setHorizontalScrollMode(QTreeWidget::ScrollPerPixel);
    _treeWidget->horizontalScrollBar()->setStyleSheet(CSS::ScrollBarH);
    _treeWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    _treeWidget->setItemDelegate(new PropertiesDelegate(_treeWidget, _treeWidget));

    _layout->setSpacing(fit::fx(2));
    _layout->setContentsMargins(fit::fx(3), fit::fx(3), fit::fx(3), fit::fx(3));

    _searchEdit->setPlaceholderText("Filter");
    _searchEdit->setClearButtonEnabled(true);
    _searchEdit->setFixedHeight(fit::fx(22));
    connect(_searchEdit, SIGNAL(textChanged(QString)), SLOT(filterList(QString)));

    _layout->addWidget(_searchEdit);
    _layout->addWidget(_treeWidget);

    /* Prepare Properties Widget */
    connect(parent->designerWidget()->formScene(), SIGNAL(selectionChanged()),
            SLOT(handleSelectionChange()));
    connect(parent->designerWidget()->controlScene(), SIGNAL(selectionChanged()),
            SLOT(handleSelectionChange()));
    connect(parent->designerWidget(), SIGNAL(modeChanged()),
            SLOT(handleSelectionChange()));
    connect(ControlWatcher::instance(), SIGNAL(geometryChanged(Control*)),
            SLOT(handleSelectionChange()));
    connect(ControlWatcher::instance(), SIGNAL(zValueChanged(Control*)),
            SLOT(handleSelectionChange()));
}

void PropertiesPane::clearList()
{
    for (int i = 0; i < _treeWidget->topLevelItemCount(); ++i)
        qDeleteAll(_treeWidget->topLevelItem(i)->takeChildren());

    _treeWidget->clear();
}

void PropertiesPane::refreshList()
{
    int vsp = _treeWidget->verticalScrollBar()->sliderPosition();
    int hsp = _treeWidget->horizontalScrollBar()->sliderPosition();

    clearList();

    auto selectedControls = dW->currentScene()->selectedControls();

    if (selectedControls.size() != 1)
        return;

    setDisabled(selectedControls[0]->hasErrors());

    auto propertyNodes = selectedControls[0]->properties();
    if (propertyNodes.isEmpty())
        return;

    {
        QTreeWidgetItem* item = new QTreeWidgetItem;
        item->setText(0, "Type");
        {
            QTreeWidgetItem* iitem = new QTreeWidgetItem;
            iitem->setText(0, "Type");
            iitem->setText(1, propertyNodes.first().cleanClassName);
            item->addChild(iitem);

            QTreeWidgetItem* jitem = new QTreeWidgetItem;
            jitem->setText(0, "id");
            jitem->setData(1, Qt::EditRole, selectedControls[0]->id());
            jitem->setData(1, NodeRole::Data, selectedControls[0]->id());
            jitem->setData(1, NodeRole::Type, NodeType::Id);
            jitem->setFlags(jitem->flags() | Qt::ItemIsEditable);
            item->addChild(jitem);
        }
        _treeWidget->addTopLevelItem(item);
        _treeWidget->expandItem(item);
    }

    for (const auto& propertyNode : propertyNodes) {
        auto metaObject = propertyNode.metaObject;
        auto map = propertyNode.propertyMap;
        cleanProperties(map);

        if (map.isEmpty())
            continue;

        auto item = new QTreeWidgetItem;
        item->setText(0, propertyNode.cleanClassName);

        for (auto propertyName : map.keys()) {
            switch (map.value(propertyName).type())
            {
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
                    processUrl(item, propertyName, map);
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
                    QMetaProperty mp;
                    for (int i = 0; i < metaObject.propertyCount(); i++)
                        if (metaObject.property(i).name() == propertyName)
                            mp = metaObject.property(i);

                    if ((mp.isValid() && (mp.isEnumType() || mp.isFlagType())) ||
                        propertyName == "inputMethodHints" ||
                        propertyName == "horizontalAlignment" ||
                        propertyName == "verticalAlignment" ||
                        propertyName == "horizontalScrollBarPolicy" ||
                        propertyName == "verticalScrollBarPolicy" ||
                        propertyName == "wrapMode" ||
                        propertyName == "orientation" ||
                        propertyName == "tickmarkAlignment" ||
                        propertyName == "echoMode") {
                        continue;
                    }

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
        _treeWidget->addTopLevelItem(item);
        _treeWidget->expandItem(item);
    }

    filterList(_searchEdit->text());

    _treeWidget->verticalScrollBar()->setSliderPosition(vsp);
    _treeWidget->horizontalScrollBar()->setSliderPosition(hsp);
}

void PropertiesPane::handleSelectionChange()
{
    auto selectedControls = dW->currentScene()->selectedControls();

    if (selectedControls.size() != 1) {
        clearList();
        return;
    }

    refreshList();
}

void PropertiesPane::filterList(const QString& filter)
{
    for (int i = 0; i < _treeWidget->topLevelItemCount(); i++) {
        auto tli = _treeWidget->topLevelItem(i);
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

bool PropertiesPane::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == _treeWidget->viewport()) {
        if (event->type() == QEvent::Paint) {
            auto w = (QWidget*)watched;
            QPainter painter(w);
            painter.setRenderHint(QPainter::Antialiasing);
            if (_treeWidget->topLevelItemCount() == 0) {
                auto sc = dW->currentScene()->selectedControls();
                bool drawn = false;
                const qreal ic = w->height() / fit::fx(20);
                for (int i = 0; i < ic; i++) {
                    if (i % 2) {
                        painter.fillRect(0, i * fit::fx(20), w->width(),
                          fit::fx(20), QColor("#E5E9EC"));
                    } else if (!drawn && (i == int(ic) / 2 ||
                      i - 1 == int(ic) / 2 || i + 1 == int(ic) / 2)) {
                        drawn = true;
                        painter.setPen(QColor(sc.size() == 1 ?
                          "#d98083" : "#a0a4a7"));
                        painter.drawText(0, i * fit::fx(20), w->width(),
                          fit::fx(20), Qt::AlignCenter, sc.size() == 1 ?
                            "Control has errors" : "No controls selected");
                    }
                }

            }
        }
        return false;
    } else {
        return QWidget::eventFilter(watched, event);
    }
}

QSize PropertiesPane::sizeHint() const
{
    return fit::fx(QSizeF{340, 700}).toSize();
}

#include "propertiespane.moc"
