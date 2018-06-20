#include <propertiespane.h>
#include <centralwidget.h>
#include <css.h>
#include <saveutils.h>
#include <savemanager.h>
#include <delayer.h>
#include <filemanager.h>
#include <controlmonitoringmanager.h>
#include <focuslesslineedit.h>
#include <previewresult.h>
#include <control.h>
#include <designerscene.h>
#include <controlpreviewingmanager.h>

#include <QtWidgets>

class PropertiesDelegate : public QStyledItemDelegate
{
        Q_OBJECT

    public:
        explicit PropertiesDelegate(QTreeWidget* view, PropertiesPane* pane, QObject* parent = 0);
        QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
        void setEditorData(QWidget* ed, const QModelIndex &index) const override;
        void setModelData(QWidget* ed, QAbstractItemModel* model, const QModelIndex &index) const override;
        void updateEditorGeometry(QWidget* ed, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
        QSize sizeHint(const QStyleOptionViewItem& opt, const QModelIndex& index) const override;
        void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    private:
        QTreeWidget* m_view;
        PropertiesPane* m_pane;
};

PropertiesDelegate::PropertiesDelegate(QTreeWidget* view, PropertiesPane* pane, QObject* parent) : QStyledItemDelegate(parent)
  , m_view(view)
  , m_pane(pane)
{
}

QWidget* PropertiesDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem &, const QModelIndex &index) const
{
    QWidget* ed = 0;

    if (index.column() == 0)
        return ed;

    auto type = index.data(PropertiesPane::Type).value<PropertiesPane::NodeType>();
    auto pIndex = m_view->model()->index(index.row(), 0, index.parent());
    auto property = m_view->model()->data(pIndex, Qt::DisplayRole).toString();

    switch (type) {
        case PropertiesPane::EnumType: {
            auto editor = new QComboBox(parent);
            auto e = index.data(PropertiesPane::NodeRole::Data).value<Enum>();
            editor->addItems(e.keys.keys());
            connect(editor, static_cast<void(QComboBox::*)(int)>(&QComboBox::activated),
                    [this, editor] () { ((PropertiesDelegate*)this)->commitData(editor); });
            editor->setFocusPolicy(Qt::StrongFocus);
            ed = editor;
            break;
        }

        case PropertiesPane::FontFamily: {
            auto editor = new QComboBox(parent);
            editor->addItems(QFontDatabase().families());
            connect(editor, static_cast<void(QComboBox::*)(int)>(&QComboBox::activated),
                    [this, editor] () { ((PropertiesDelegate*)this)->commitData(editor); });
            editor->setFocusPolicy(Qt::StrongFocus);
            ed = editor;
            break;
        }

        case PropertiesPane::FontWeight: {
            auto editor = new QComboBox(parent);
            auto fenum = QMetaEnum::fromType<QFont::Weight>();
            for (int i = fenum.keyCount(); i--;)
                if (QString(fenum.key(i)).contains(QRegularExpression("[^\\r\\n\\t\\f\\v ]")))
                    editor->addItem(fenum.key(i));
            connect(editor, static_cast<void(QComboBox::*)(int)>(&QComboBox::activated),
                    [this, editor] () { ((PropertiesDelegate*)this)->commitData(editor); });
            editor->setFocusPolicy(Qt::StrongFocus);
            ed = editor;
            break;
        }

        case PropertiesPane::FontPtSize:
        case PropertiesPane::FontPxSize: {
            auto editor = new QSpinBox(parent);
            connect(editor, &QSpinBox::editingFinished,
                    [this, editor] () { ((PropertiesDelegate*)this)->commitData(editor); });
            editor->setFocusPolicy(Qt::StrongFocus);
            editor->setMaximum(72);
            editor->setMinimum(0);
            ed = editor;
            break;
        }

        case PropertiesPane::FontBold:
        case PropertiesPane::FontItalic:
        case PropertiesPane::FontUnderline:
        case PropertiesPane::FontOverline:
        case PropertiesPane::FontStrikeout:
        case PropertiesPane::Bool: {
            auto editor = new QCheckBox(parent);
            connect(editor, &QCheckBox::toggled,
                    [this, editor] () { ((PropertiesDelegate*)this)->commitData(editor); });
            editor->setFocusPolicy(Qt::StrongFocus);
            ed = editor;
            break;
        }

        case PropertiesPane::Color: {
            auto editor = new QToolButton(parent);
            editor->setFocusPolicy(Qt::StrongFocus);
            editor->setText("Change Color");
            ed = editor;

            connect(editor, &QCheckBox::clicked, [this, property, index] ()
            {
                auto color = index.data(PropertiesPane::NodeRole::Data).value<QColor>();
                color = QColorDialog::getColor(
                    Qt::white,
                    m_view,
                    "Select Color",
                    QColorDialog::ShowAlphaChannel | QColorDialog::DontUseNativeDialog
                );

                if (color.isValid()) {
                    m_view->model()->setData(index, color, PropertiesPane::NodeRole::Data);
                    m_view->model()->setData(index, color.name(QColor::HexArgb), Qt::EditRole);
                    m_pane->saveChanges(property, "\"" + color.name(QColor::HexArgb) + "\"", color);
                }
            });
            break;
        }

        case PropertiesPane::String: {
            auto editor = new QLineEdit(parent);
            connect(editor, &QLineEdit::editingFinished,
                    [this, editor] () { ((PropertiesDelegate*)this)->commitData(editor); });
            editor->setFocusPolicy(Qt::StrongFocus);
            ed = editor;
            break;
        }

        case PropertiesPane::Id: {
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

        case PropertiesPane::Url: {
            auto editor = new QLineEdit(parent);
            connect(editor, &QLineEdit::editingFinished,
                    [this, editor] () { ((PropertiesDelegate*)this)->commitData(editor); });
            editor->setFocusPolicy(Qt::StrongFocus);
            ed = editor;
            break;
        }

        case PropertiesPane::Double: {
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

        case PropertiesPane::Int: {
            auto editor = new QSpinBox(parent);
            connect(editor, &QSpinBox::editingFinished,
                    [this, editor] () { ((PropertiesDelegate*)this)->commitData(editor); });
            editor->setFocusPolicy(Qt::StrongFocus);
            editor->setMaximum(std::numeric_limits<int>::max());
            editor->setMinimum(std::numeric_limits<int>::min());
            ed = editor;
            break;
        }

        case PropertiesPane::GeometryX:
        case PropertiesPane::GeometryY:
        case PropertiesPane::GeometryWidth:
        case PropertiesPane::GeometryHeight: {
            auto editor = new QSpinBox(parent);
            connect(editor, &QSpinBox::editingFinished,
                    [this, editor] () { ((PropertiesDelegate*)this)->commitData(editor); });
            editor->setFocusPolicy(Qt::StrongFocus);
            editor->setMaximum(std::numeric_limits<int>::max());
            editor->setMinimum(std::numeric_limits<int>::min());
            ed = editor;
            break;
        }

        case PropertiesPane::GeometryFX:
        case PropertiesPane::GeometryFY:
        case PropertiesPane::GeometryFWidth:
        case PropertiesPane::GeometryFHeight: {
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

    auto type = index.data(PropertiesPane::NodeRole::Type).value<PropertiesPane::NodeType>();

    switch (type) {
        case PropertiesPane::EnumType: {
            auto val = index.model()->data(index, PropertiesPane::NodeRole::Data).value<Enum>();
            auto editor = static_cast<QComboBox*>(ed);
            editor->setCurrentText(val.value);
            break;
        }

        case PropertiesPane::FontFamily: {
            auto val = index.model()->data(index, PropertiesPane::NodeRole::Data).value<QString>();
            auto editor = static_cast<QComboBox*>(ed);
            editor->setCurrentText(val);
            break;
        }

        case PropertiesPane::FontWeight: {
            auto val = index.model()->data(index, PropertiesPane::NodeRole::Data).value<QFont::Weight>();
            auto editor = static_cast<QComboBox*>(ed);
            editor->setCurrentText(QMetaEnum::fromType<QFont::Weight>().valueToKey(val));
            break;
        }

        case PropertiesPane::FontPtSize:
        case PropertiesPane::FontPxSize: {
            auto val = index.model()->data(index, PropertiesPane::NodeRole::Data).value<int>();
            auto editor = static_cast<QSpinBox*>(ed);
            editor->setValue(val);
            break;
        }

        case PropertiesPane::FontBold:
        case PropertiesPane::FontItalic:
        case PropertiesPane::FontUnderline:
        case PropertiesPane::FontOverline:
        case PropertiesPane::FontStrikeout:
        case PropertiesPane::Bool: {
            auto val = index.model()->data(index, PropertiesPane::NodeRole::Data).value<bool>();
            auto editor = static_cast<QCheckBox*>(ed);
            editor->setChecked(val);
            break;
        }

        case PropertiesPane::Id:
        case PropertiesPane::String: {
            auto val = index.model()->data(index, PropertiesPane::NodeRole::Data).value<QString>();
            auto editor = static_cast<QLineEdit*>(ed);
            editor->setText(val);
            break;
        }

        case PropertiesPane::Url: {
            auto selectedControl = m_pane->m_designerScene->selectedControls().at(0);
            auto val = index.model()->data(index, PropertiesPane::NodeRole::Data).value<QUrl>();
            auto editor = static_cast<QLineEdit*>(ed);
            auto dispText = val.toDisplayString();
            if (val.isLocalFile()) {
                dispText = val.toLocalFile().
                           remove(selectedControl->dir() + separator() + DIR_THIS + separator());
            }
            editor->setText(dispText);
            break;
        }

        case PropertiesPane::Double: {
            auto val = index.model()->data(index, PropertiesPane::NodeRole::Data).value<double>();
            auto editor = static_cast<QDoubleSpinBox*>(ed);
            editor->setValue(val);
            break;
        }

        case PropertiesPane::Int: {
            auto val = index.model()->data(index, PropertiesPane::NodeRole::Data).value<int>();
            auto editor = static_cast<QSpinBox*>(ed);
            editor->setValue(val);
            break;
        }

        case PropertiesPane::GeometryX:
        case PropertiesPane::GeometryY:
        case PropertiesPane::GeometryWidth:
        case PropertiesPane::GeometryHeight: {
            auto val = index.model()->data(index, PropertiesPane::NodeRole::Data).value<int>();
            auto editor = static_cast<QSpinBox*>(ed);
            editor->setValue(val);
            break;
        }

        case PropertiesPane::GeometryFX:
        case PropertiesPane::GeometryFY:
        case PropertiesPane::GeometryFWidth:
        case PropertiesPane::GeometryFHeight: {
            auto val = index.model()->data(index, PropertiesPane::NodeRole::Data).value<double>();
            auto editor = static_cast<QDoubleSpinBox*>(ed);
            editor->setValue(val);
            break;
        }

        default:
            break;
    }
}

void PropertiesDelegate::setModelData(QWidget* ed, QAbstractItemModel* model, const QModelIndex &index) const
{
    if (index.column() == 0)
        return;

    QVariant val;
    auto type = index.data(PropertiesPane::NodeRole::Type).value<PropertiesPane::NodeType>();
    auto pIndex = model->index(index.row(), 0, index.parent());
    auto property = model->data(pIndex, Qt::DisplayRole).toString();

    switch (type) {
        case PropertiesPane::EnumType: {
            auto editor = static_cast<QComboBox*>(ed);
            val = editor->currentText();
            auto preVal = model->data(index, Qt::EditRole).toString();

            if (val == preVal)
                return;

            auto e = index.data(PropertiesPane::NodeRole::Data).value<Enum>();
            e.value = val.toString();

            model->setData(index, QVariant::fromValue(e), PropertiesPane::NodeRole::Data);
            model->setData(index, e.value, Qt::EditRole);

            m_pane->saveChanges(property, e.scope + "." + e.value, e.keys.value(e.value));
            break;
        }

        case PropertiesPane::FontFamily: {
            auto editor = static_cast<QComboBox*>(ed);
            val = editor->currentText();
            auto preVal = model->data(index, Qt::EditRole).toString();

            if (val == preVal)
                return;

            model->setData(index, val, PropertiesPane::NodeRole::Data);
            model->setData(index, val, Qt::EditRole);

            // Update parent node
            auto pIndex = model->index(index.parent().row(), 1, index.parent().parent());
            auto pVal = model->data(pIndex, Qt::DisplayRole).toString();
            pVal.replace(preVal, val.toString());
            model->setData(pIndex, pVal, Qt::DisplayRole);

            m_pane->saveChanges(type, "\"" + val.toString() + "\"", val);
            break;
        }

        case PropertiesPane::FontWeight: {
            auto editor = static_cast<QComboBox*>(ed);
            val = editor->currentText();
            auto preVal = model->data(index, Qt::EditRole).toString();

            if (val == preVal)
                return;

            auto fenum = QMetaEnum::fromType<QFont::Weight>();

            model->setData(index, fenum.keyToValue(val.toString().toUtf8().data()), PropertiesPane::NodeRole::Data);
            model->setData(index, val, Qt::EditRole);

            m_pane->saveChanges(type, "Font." + val.toString(), val);
            break;
        }

        case PropertiesPane::FontPtSize:
        case PropertiesPane::FontPxSize: {
            auto editor = static_cast<QSpinBox*>(ed);
            val = editor->value();
            auto preVal = model->data(index, Qt::EditRole).toString();

            if (val == preVal)
                return;

            model->setData(index, val, PropertiesPane::NodeRole::Data);
            model->setData(index, val, Qt::EditRole);

            // Update parent node
            int pxSize, ptSize;
            if (type == PropertiesPane::FontPtSize) {
                auto bIndex = model->index(index.row() + 1, 1, index.parent());
                pxSize = model->data(bIndex, PropertiesPane::NodeRole::Data).toInt();
                ptSize = val.toInt();
            } else {
                auto bIndex = model->index(index.row() - 1, 1, index.parent());
                ptSize = model->data(bIndex, PropertiesPane::NodeRole::Data).toInt();
                pxSize = val.toInt();
            }
            bool px = pxSize > 0 ? true : false;
            auto pIndex = model->index(index.parent().row(), 1, index.parent().parent());
            auto pVal = model->data(pIndex, Qt::DisplayRole).toString();
            pVal.replace(QRegExp(",.*"), ", " + QString::number(px ? pxSize : ptSize) + (px ? "px]" : "pt]"));
            model->setData(pIndex, pVal, Qt::DisplayRole);
            m_pane->saveChanges(type, val.toString(), val);
            break;
        }

        case PropertiesPane::FontBold:
        case PropertiesPane::FontItalic:
        case PropertiesPane::FontUnderline:
        case PropertiesPane::FontOverline:
        case PropertiesPane::FontStrikeout: {
            auto editor = static_cast<QCheckBox*>(ed);
            val = editor->isChecked();
            auto preVal = model->data(index, PropertiesPane::NodeRole::Data).toBool();

            if (val == preVal)
                return;

            model->setData(index, val, PropertiesPane::NodeRole::Data);
            m_pane->saveChanges(type, val.toString(), val);
            break;
        }

        case PropertiesPane::Bool: {
            auto editor = static_cast<QCheckBox*>(ed);
            val = editor->isChecked();
            auto preVal = model->data(index, PropertiesPane::NodeRole::Data).toBool();

            if (val == preVal)
                return;

            model->setData(index, val, PropertiesPane::NodeRole::Data);
            m_pane->saveChanges(property, val.toString(), val);
            break;
        }

        case PropertiesPane::Id: {
            auto editor = static_cast<QLineEdit*>(ed);
            val = editor->text();
            auto preVal = model->data(index, Qt::EditRole).toString();

            if (val == preVal)
                return;

            model->setData(index, val, PropertiesPane::NodeRole::Data);
            model->setData(index, val, Qt::EditRole);
            m_pane->saveChanges(property, val.toString(), val);
            break;
        }

        case PropertiesPane::String: {
            auto editor = static_cast<QLineEdit*>(ed);
            val = editor->text();
            auto preVal = model->data(index, Qt::EditRole).toString();

            if (val == preVal)
                return;

            model->setData(index, val, PropertiesPane::NodeRole::Data);
            model->setData(index, val, Qt::EditRole);
            m_pane->saveChanges(property, "\"" +val.toString() + "\"", val);
            break;
        }

        case PropertiesPane::Url: {
            auto editor = static_cast<QLineEdit*>(ed);
            val = QUrl(editor->text());
            auto preVal = model->data(index, Qt::EditRole).toUrl();

            if (val == preVal)
                return;

            model->setData(index, val, PropertiesPane::NodeRole::Data);
            model->setData(index, val, Qt::EditRole);
            m_pane->saveChanges(property, "\"" + val.toString() + "\"", val);
            break;
        }

        case PropertiesPane::Double: {
            auto editor = static_cast<QDoubleSpinBox*>(ed);
            val = editor->value();
            auto preVal = model->data(index, PropertiesPane::NodeRole::Data).toDouble();

            if (val == preVal)
                return;

            model->setData(index, val, PropertiesPane::NodeRole::Data);
            model->setData(index, val, Qt::EditRole);
            if (property == "z") {
                auto sc = m_pane->m_designerScene->selectedControls();
                if (sc.size() == 1)
                    sc[0]->setZValue(val.toReal());
            }
            m_pane->saveChanges(property, val.toString(), val);
            break;
        }

        case PropertiesPane::Int: {
            auto editor = static_cast<QSpinBox*>(ed);
            val = editor->value();
            auto preVal = model->data(index, PropertiesPane::NodeRole::Data).toInt();

            if (val == preVal)
                return;

            model->setData(index, val, PropertiesPane::NodeRole::Data);
            model->setData(index, val, Qt::EditRole);
            m_pane->saveChanges(property, val.toString(), val);
            break;
        }

        case PropertiesPane::GeometryX:
        case PropertiesPane::GeometryY:
        case PropertiesPane::GeometryWidth:
        case PropertiesPane::GeometryHeight: {
            auto editor = static_cast<QSpinBox*>(ed);
            val = editor->value();
            auto preVal = model->data(index, PropertiesPane::NodeRole::Data).toInt();

            if (val == preVal)
                return;

            model->setData(index, val, PropertiesPane::NodeRole::Data);
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
            m_pane->saveChanges(type, val.toString(), val);
            break;
        }

        case PropertiesPane::GeometryFX:
        case PropertiesPane::GeometryFY:
        case PropertiesPane::GeometryFWidth:
        case PropertiesPane::GeometryFHeight: {
            auto editor = static_cast<QDoubleSpinBox*>(ed);
            val = editor->value();
            auto preVal = model->data(index, PropertiesPane::NodeRole::Data).toReal();

            if (val == preVal)
                return;

            model->setData(index, val, PropertiesPane::NodeRole::Data);
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
            m_pane->saveChanges(type, val.toString(), val);
            break;
        }

        default:
            break;
    }
}

void PropertiesDelegate::updateEditorGeometry(QWidget* ed, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyledItemDelegate::updateEditorGeometry(ed, option, index);

    auto type = index.data(PropertiesPane::NodeRole::Type).value<PropertiesPane::NodeType>();

    switch (type) {
        case 0:
        default:
            ed->setGeometry(ed->geometry().adjusted(0, 0, -1, -1));
            break;
    }
}

QSize PropertiesDelegate::sizeHint(const QStyleOptionViewItem &opt, const QModelIndex &index) const
{
    const auto& size = QStyledItemDelegate::sizeHint(opt, index);
    return QSize(size.width(), 22);
}

void PropertiesDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
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

        auto type = index.data(PropertiesPane::NodeRole::Type).value<PropertiesPane::NodeType>();
        switch (type) {
            case PropertiesPane::FontBold:
            case PropertiesPane::FontItalic:
            case PropertiesPane::FontUnderline:
            case PropertiesPane::FontOverline:
            case PropertiesPane::FontStrikeout:
            case PropertiesPane::Bool: {
                bool value = index.data(PropertiesPane::NodeRole::Data).value<bool>();
                QStyleOptionButton opt;
                opt.initFrom(m_view);
                opt.rect = option.rect;
                opt.state = QStyle::State_Active | QStyle::State_Enabled;
                opt.state |= value ? QStyle::State_On : QStyle::State_Off;
                qApp->style()->drawControl(QStyle::CE_CheckBox, &opt, painter, m_view);
                break;
            }

            default:
                break;
        }
    }
}

class PropertiesTree : public QTreeWidget
{
        Q_OBJECT
    protected:
        void drawBranches(QPainter* painter, const QRect& rect, const QModelIndex& index) const override;
};

void PropertiesTree::drawBranches(QPainter* painter, const QRect& rect, const QModelIndex& index) const
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

PropertiesPane::PropertiesPane(DesignerScene* designerScene, QWidget* parent) : QWidget(parent)
  , m_designerScene(designerScene)
{
    _layout = new QVBoxLayout(this);
    _treeWidget = new PropertiesTree;
    _searchEdit = new FocuslessLineEdit;

    QPalette p(palette());
    p.setColor(backgroundRole(), "#ececec");
    setAutoFillBackground(true);
    setPalette(p);

    QPalette p2(_treeWidget->palette());
    p2.setColor(QPalette::All, QPalette::Base, QColor("#fffefc"));
    p2.setColor(QPalette::All, QPalette::Highlight, QColor("#ebd5c0"));
    p2.setColor(QPalette::All, QPalette::Text, Qt::black);
    p2.setColor(QPalette::All, QPalette::HighlightedText, Qt::black);
    _treeWidget->setPalette(p2);

    _treeWidget->setColumnCount(2);
    _treeWidget->setIndentation(16);
    _treeWidget->setDragEnabled(false);
    _treeWidget->setFocusPolicy(Qt::NoFocus);
    _treeWidget->setUniformRowHeights(true);
    _treeWidget->setDropIndicatorShown(false);
    _treeWidget->headerItem()->setText(1, "Value");
    _treeWidget->headerItem()->setText(0, "Property");
    _treeWidget->viewport()->installEventFilter(this);
    _treeWidget->header()->resizeSection(0, 170);
    _treeWidget->setSelectionBehavior(QTreeWidget::SelectRows);
    _treeWidget->verticalScrollBar()->setStyleSheet(CSS::ScrollBar);
    _treeWidget->setVerticalScrollMode(QTreeWidget::ScrollPerPixel);
    _treeWidget->setEditTriggers(QAbstractItemView::AllEditTriggers);
    _treeWidget->setHorizontalScrollMode(QTreeWidget::ScrollPerPixel);
    _treeWidget->horizontalScrollBar()->setStyleSheet(CSS::ScrollBarH);
    _treeWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    _treeWidget->setItemDelegate(new PropertiesDelegate(_treeWidget, this, _treeWidget));
    _treeWidget->header()->setFixedHeight(23);
    _treeWidget->setStyleSheet("QTreeWidget { border: 1px solid #8c6a48; }"
                               "QHeaderView::section {"
                               "padding-left: 5px; color: white; border:none; border-bottom: 1px solid #8c6a48;"
                               "background: qlineargradient(spread:pad, x1:0.5, y1:0, x2:0.5, y2:1, stop:0 #AB8157, stop:1 #9C7650);}");

    QFont f; f.setWeight(QFont::Medium);
    _treeWidget->header()->setFont(f);

    _layout->setSpacing(2);
    _layout->setContentsMargins(3, 3, 3, 3);

    _searchEdit->setPlaceholderText("Filter");
    _searchEdit->setClearButtonEnabled(true);
    _searchEdit->setFixedHeight(22);
    connect(_searchEdit, SIGNAL(textChanged(QString)), SLOT(filterList(QString)));

    _layout->addWidget(_searchEdit);
    _layout->addWidget(_treeWidget);

    /* Prepare Properties Widget */
    connect(m_designerScene, SIGNAL(selectionChanged()), SLOT(handleSelectionChange()));
    connect(ControlMonitoringManager::instance(), SIGNAL(geometryChanged(Control*)), SLOT(handleSelectionChange()));
    connect(ControlMonitoringManager::instance(), SIGNAL(zValueChanged(Control*)), SLOT(handleSelectionChange()));
}

void PropertiesPane::sweep()
{
    clearList();
    _searchEdit->clear();
}

void PropertiesPane::clearList()
{
    for (int i = 0; i < _treeWidget->topLevelItemCount(); ++i)
        qDeleteAll(_treeWidget->topLevelItem(i)->takeChildren());

    _treeWidget->clear();
}

// FIXME: x, y, width, height, z properties are not shown (because we removed them within previewer)
void PropertiesPane::refreshList()
{
    int vsp = _treeWidget->verticalScrollBar()->sliderPosition();
    int hsp = _treeWidget->horizontalScrollBar()->sliderPosition();

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
        item_3->setData(1, NodeRole::Type, PropertiesPane::NodeType::Id);
        item_3->setFlags(item_3->flags() | Qt::ItemIsEditable);

        _treeWidget->addTopLevelItem(item_1);
        _treeWidget->addTopLevelItem(item_2);
        _treeWidget->addTopLevelItem(item_3);
    }

    for (const auto& propertyNode : propertyNodes) {
        auto map = propertyNode.properties;
        auto enums = propertyNode.enums;

        if (map.isEmpty() && enums.isEmpty())
            continue;

        auto item = new QTreeWidgetItem;
        item->setText(0, propertyNode.cleanClassName);

        for (const auto& propertyName : map.keys()) {
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
             item1->setData(1, NodeRole::Type, PropertiesPane::NodeType::EnumType);
             item1->setData(1, NodeRole::Data, QVariant::fromValue(e));
             item->addChild(item1);
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
    auto selectedControls = m_designerScene->selectedControls();

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

void PropertiesPane::processFont(QTreeWidgetItem* item, const QString& propertyName, const QMap<QString, QVariant>& map)
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
    item0->setData(1, NodeRole::Type, PropertiesPane::NodeType::FontFamily);
    item0->setData(1, NodeRole::Data, value.family());
    iitem->addChild(item0);

    auto item1 = new QTreeWidgetItem;
    item1->setFlags(item1->flags() | Qt::ItemIsEditable);
    item1->setText(0, "weight");
    item1->setData(1, Qt::EditRole, QMetaEnum::fromType<QFont::Weight>().valueToKey(value.weight()));
    item1->setData(1, NodeRole::Type, PropertiesPane::NodeType::FontWeight);
    item1->setData(1, NodeRole::Data, value.weight());
    iitem->addChild(item1);

    auto item2 = new QTreeWidgetItem;
    item2->setFlags(item2->flags() | Qt::ItemIsEditable);
    item2->setText(0, "pointSize");
    item2->setData(1, Qt::EditRole, value.pointSize() < 0 ? 0 : value.pointSize());
    item2->setData(1, NodeRole::Type, PropertiesPane::NodeType::FontPtSize);
    item2->setData(1, NodeRole::Data, value.pointSize() < 0 ? 0 : value.pointSize());
    iitem->addChild(item2);

    auto item3 = new QTreeWidgetItem;
    item3->setFlags(item3->flags() | Qt::ItemIsEditable);
    item3->setText(0, "pixelSize");
    item3->setData(1, Qt::EditRole, value.pixelSize() < 0 ? 0 : value.pixelSize());
    item3->setData(1, NodeRole::Type, PropertiesPane::NodeType::FontPxSize);
    item3->setData(1, NodeRole::Data, value.pixelSize() < 0 ? 0 : value.pixelSize());
    iitem->addChild(item3);

    auto item4 = new QTreeWidgetItem;
    item4->setFlags(item4->flags() | Qt::ItemIsEditable);
    item4->setText(0, "bold");
    item4->setData(1, NodeRole::Type, PropertiesPane::NodeType::FontBold);
    item4->setData(1, NodeRole::Data, value.bold());
    iitem->addChild(item4);

    auto item5 = new QTreeWidgetItem;
    item5->setFlags(item5->flags() | Qt::ItemIsEditable);
    item5->setText(0, "italic");
    item5->setData(1, NodeRole::Type, PropertiesPane::NodeType::FontItalic);
    item5->setData(1, NodeRole::Data, value.italic());
    iitem->addChild(item5);

    auto item6 = new QTreeWidgetItem;
    item6->setFlags(item6->flags() | Qt::ItemIsEditable);
    item6->setText(0, "underline");
    item6->setData(1, NodeRole::Type, PropertiesPane::NodeType::FontUnderline);
    item6->setData(1, NodeRole::Data, value.underline());
    iitem->addChild(item6);

    auto item7 = new QTreeWidgetItem;
    item7->setFlags(item7->flags() | Qt::ItemIsEditable);
    item7->setText(0, "overline");
    item7->setData(1, NodeRole::Type, PropertiesPane::NodeType::FontOverline);
    item7->setData(1, NodeRole::Data, value.overline());
    iitem->addChild(item7);

    auto item8 = new QTreeWidgetItem;
    item8->setFlags(item8->flags() | Qt::ItemIsEditable);
    item8->setText(0, "strikeout");
    item8->setData(1, NodeRole::Type, PropertiesPane::NodeType::FontStrikeout);
    item8->setData(1, NodeRole::Data, value.strikeOut());
    iitem->addChild(item8);

    item->addChild(iitem);
}

void PropertiesPane::processGeometry(QTreeWidgetItem* item, const QString& propertyName, Control* control)
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
    item1->setData(1, NodeRole::Type, PropertiesPane::NodeType::GeometryX);
    item1->setData(1, NodeRole::Data, value.x());
    iitem->addChild(item1);

    auto item2 = new QTreeWidgetItem;
    item2->setFlags(item2->flags() | Qt::ItemIsEditable);
    item2->setText(0, "y");
    item2->setData(1, Qt::EditRole, value.y());
    item2->setData(1, NodeRole::Type, PropertiesPane::NodeType::GeometryY);
    item2->setData(1, NodeRole::Data, value.y());
    iitem->addChild(item2);

    auto item3 = new QTreeWidgetItem;
    item3->setFlags(item3->flags() | Qt::ItemIsEditable);
    item3->setText(0, "width");
    item3->setData(1, Qt::EditRole, value.width());
    item3->setData(1, NodeRole::Type, PropertiesPane::NodeType::GeometryWidth);
    item3->setData(1, NodeRole::Data, value.width());
    iitem->addChild(item3);

    auto item4 = new QTreeWidgetItem;
    item4->setFlags(item4->flags() | Qt::ItemIsEditable);
    item4->setText(0, "height");
    item4->setData(1, Qt::EditRole, value.height());
    item4->setData(1, NodeRole::Type, PropertiesPane::NodeType::GeometryHeight);
    item4->setData(1, NodeRole::Data, value.height());
    iitem->addChild(item4);

    item->addChild(iitem);
}

void PropertiesPane::processGeometryF(QTreeWidgetItem* item, const QString& propertyName, Control* control)
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
    item1->setData(1, NodeRole::Type, PropertiesPane::NodeType::GeometryFX);
    item1->setData(1, NodeRole::Data, value.x());
    iitem->addChild(item1);

    auto item2 = new QTreeWidgetItem;
    item2->setFlags(item2->flags() | Qt::ItemIsEditable);
    item2->setText(0, "y");
    item2->setData(1, Qt::EditRole, value.y());
    item2->setData(1, NodeRole::Type, PropertiesPane::NodeType::GeometryFY);
    item2->setData(1, NodeRole::Data, value.y());
    iitem->addChild(item2);

    auto item3 = new QTreeWidgetItem;
    item3->setFlags(item3->flags() | Qt::ItemIsEditable);
    item3->setText(0, "width");
    item3->setData(1, Qt::EditRole, value.width());
    item3->setData(1, NodeRole::Type, PropertiesPane::NodeType::GeometryFWidth);
    item3->setData(1, NodeRole::Data, value.width());
    iitem->addChild(item3);

    auto item4 = new QTreeWidgetItem;
    item4->setFlags(item4->flags() | Qt::ItemIsEditable);
    item4->setText(0, "height");
    item4->setData(1, Qt::EditRole, value.height());
    item4->setData(1, NodeRole::Type, PropertiesPane::NodeType::GeometryFHeight);
    item4->setData(1, NodeRole::Data, value.height());
    iitem->addChild(item4);

    item->addChild(iitem);
}

void PropertiesPane::processColor(QTreeWidgetItem* item, const QString& propertyName, const QMap<QString, QVariant>& map)
{
    const auto value = map.value(propertyName).value<QColor>();
    const auto cc = value.name(QColor::HexArgb);

    auto iitem = new QTreeWidgetItem;
    iitem->setText(0, propertyName);
    iitem->setData(1, Qt::EditRole, cc);
    iitem->setData(1, NodeRole::Data, value);
    iitem->setData(1, NodeRole::Type, PropertiesPane::NodeType::Color);
    iitem->setFlags(iitem->flags() | Qt::ItemIsEditable);

    item->addChild(iitem);
}

void PropertiesPane::processBool(QTreeWidgetItem* item, const QString& propertyName, const QMap<QString, QVariant>& map)
{
    const auto value = map.value(propertyName).value<bool>();

    auto iitem = new QTreeWidgetItem;
    iitem->setText(0, propertyName);
    iitem->setData(1, NodeRole::Data, value);
    iitem->setData(1, NodeRole::Type, PropertiesPane::NodeType::Bool);
    iitem->setFlags(iitem->flags() | Qt::ItemIsEditable);

    item->addChild(iitem);
}

void PropertiesPane::processString(QTreeWidgetItem* item, const QString& propertyName, const QMap<QString, QVariant>& map)
{
    const auto value = map.value(propertyName).value<QString>();

    auto iitem = new QTreeWidgetItem;
    iitem->setText(0, propertyName);
    iitem->setData(1, Qt::EditRole, value);
    iitem->setData(1, NodeRole::Data, value);
    iitem->setData(1, NodeRole::Type, PropertiesPane::NodeType::String);
    iitem->setFlags(iitem->flags() | Qt::ItemIsEditable);

    item->addChild(iitem);
}

void PropertiesPane::processUrl(QTreeWidgetItem* item, const QString& propertyName, const QMap<QString, QVariant>& map)
{
    auto selectedControl = m_designerScene->selectedControls().at(0);
    const auto value = map.value(propertyName).value<QUrl>();
    auto dispText = value.toDisplayString();
    if (value.isLocalFile()) {
        dispText = value.toLocalFile().remove(selectedControl->dir() + separator() + DIR_THIS + separator());
    }

    auto iitem = new QTreeWidgetItem;
    iitem->setText(0, propertyName);
    iitem->setData(1, Qt::EditRole, dispText);
    iitem->setData(1, NodeRole::Data, value);
    iitem->setData(1, NodeRole::Type, PropertiesPane::NodeType::Url);
    iitem->setFlags(iitem->flags() | Qt::ItemIsEditable);

    item->addChild(iitem);
}

void PropertiesPane::processDouble(QTreeWidgetItem* item, const QString& propertyName, const QMap<QString, QVariant>& map)
{
    const auto value = map.value(propertyName).value<double>();

    auto iitem = new QTreeWidgetItem;
    iitem->setText(0, propertyName);
    iitem->setData(1, Qt::EditRole, value);
    iitem->setData(1, NodeRole::Data, value);
    iitem->setData(1, NodeRole::Type, PropertiesPane::NodeType::Double);
    iitem->setFlags(iitem->flags() | Qt::ItemIsEditable);

    item->addChild(iitem);
}

void PropertiesPane::processInt(QTreeWidgetItem* item, const QString& propertyName, const QMap<QString, QVariant>& map)
{
    const auto value = map.value(propertyName).value<int>();

    auto iitem = new QTreeWidgetItem;
    iitem->setText(0, propertyName);
    iitem->setData(1, Qt::EditRole, value);
    iitem->setData(1, NodeRole::Data, value);
    iitem->setData(1, NodeRole::Type, PropertiesPane::NodeType::Int);
    iitem->setFlags(iitem->flags() | Qt::ItemIsEditable);

    item->addChild(iitem);
}

void PropertiesPane::saveChanges(const QString& property, const QString& parserValue, const QVariant& value)
{
    auto scs = m_designerScene->selectedControls();

    if (scs.isEmpty())
        return;

    auto sc = scs.at(0);

    SaveManager::setProperty(sc, property, parserValue);
    ControlPreviewingManager::updateCache(sc->uid(), property, value);
}

void PropertiesPane::saveChanges(const PropertiesPane::NodeType& type, const QString& parserValue, const QVariant& value)
{
    QString property;
    switch (type) {
        case PropertiesPane::FontFamily:
            property = "font.family";
            break;

        case PropertiesPane::FontWeight:
            property = "font.weight";
            break;

        case PropertiesPane::FontPtSize:
            property = "font.pointSize";
            break;

        case PropertiesPane::FontPxSize:
            property = "font.pixelSize";
            break;

        case PropertiesPane::FontBold:
            property = "font.bold";
            break;

        case PropertiesPane::FontItalic:
            property = "font.italic";
            break;

        case PropertiesPane::FontUnderline:
            property = "font.underline";
            break;

        case PropertiesPane::FontOverline:
            property = "font.overline";
            break;

        case PropertiesPane::FontStrikeout:
            property = "font.strikeout";
            break;

        case PropertiesPane::GeometryX:
        case PropertiesPane::GeometryFX: {
            auto sc = m_designerScene->selectedControls();
            if (sc.size() != 1)
                return;
            if (sc[0]->form()) {
                property = "x";
                break;
            } else {
                sc[0]->setX(value.toReal());
                return;
            }
        }

        case PropertiesPane::GeometryY:
        case PropertiesPane::GeometryFY: {
            auto sc = m_designerScene->selectedControls();
            if (sc.size() != 1)
                return;
            if (sc[0]->form()) {
                property = "y";
                break;
            } else {
                sc[0]->setY(value.toReal());
                return;
            }
        }

        case PropertiesPane::GeometryWidth:
        case PropertiesPane::GeometryFWidth: {
            auto sc = m_designerScene->selectedControls();
            if (sc.size() != 1)
                return;
            if (sc[0]->gui())
                sc[0]->resize(value.toReal(), sc[0]->size().height());
            return;
        }

        case PropertiesPane::GeometryHeight:
        case PropertiesPane::GeometryFHeight: {
            auto sc = m_designerScene->selectedControls();
            if (sc.size() != 1)
                return;
            if (sc[0]->gui())
                sc[0]->resize(sc[0]->size().width(), value.toReal());
            return;
        }

        default:
            break;
    }
    saveChanges(property, parserValue, value);
}

// TODO: Visibility control->properties()'den alınmayacak, ParserWorker ile alınacak
bool PropertiesPane::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == _treeWidget->viewport()) {
        if (event->type() == QEvent::Paint) {
            auto w = (QWidget*)watched;
            QPainter painter(w);
            painter.setRenderHint(QPainter::Antialiasing);
            if (_treeWidget->topLevelItemCount() == 0) {
                auto sc = m_designerScene->selectedControls();
                bool drawn = false;
                const qreal ic = w->height() / 20; // WARNING: Constant 20?
                for (int i = 0; i < ic; i++) {
                    if (i % 2) {
                        painter.fillRect(0, i * 20, w->width(),
                                         20, QColor("#faf1e8"));
                    } else if (!drawn && (i == int(ic) / 2 ||
                                          i - 1 == int(ic) / 2 || i + 1 == int(ic) / 2)) {
                        drawn = true;
                        painter.setPen(QColor(sc.size() == 1 ?
                                                  "#d98083" : "#b5aea7"));
                        painter.drawText(0, i * 20, w->width(),
                                         20, Qt::AlignCenter, sc.size() == 1 ?
                                             "Control has errors" : "No controls selected");
                    }
                }

            } else {
                const auto tli = _treeWidget->topLevelItem(_treeWidget->topLevelItemCount() - 1);
                const auto lci = tli->child(tli->childCount() - 1);
                auto lcir = _treeWidget->visualItemRect(lci);

                if (lcir.isNull())
                    lcir = _treeWidget->visualItemRect(tli);

                const qreal ic = (_treeWidget->viewport()->height() + qAbs(lcir.y())) / (qreal) lcir.height();

                for (int i = 0; i < ic; i++) {
                    if (i % 2) {
                        painter.fillRect(
                                    0,
                                    lcir.y() + i * lcir.height(),
                                    _treeWidget->viewport()->width(),
                                    lcir.height(),
                                    QColor("#faf1e8")
                                    );
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
    return QSize{340, 700};
}

#include "propertiespane.moc"
