#include <propertieswidget.h>
#include <propertyitem.h>
#include <fit.h>
#include <toolboxtree.h>
#include <designmanager.h>
#include <css.h>

#include <QStyleOption>
#include <QPainter>
#include <QQuickItem>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QQmlContext>
#include <QIcon>
#include <QHeaderView>
#include <QItemDelegate>
#include <QApplication>

using namespace Fit;

class ColorDelegate : public QItemDelegate
{
    Q_OBJECT
        enum { BrushRole = 33 };
public:
    explicit ColorDelegate(QTreeView *view, QObject *parent = 0);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const Q_DECL_OVERRIDE;

    void setEditorData(QWidget *ed, const QModelIndex &index) const Q_DECL_OVERRIDE;
    void setModelData(QWidget *ed, QAbstractItemModel *model,
                      const QModelIndex &index) const Q_DECL_OVERRIDE;

    void updateEditorGeometry(QWidget *ed, const QStyleOptionViewItem &option,
                              const QModelIndex &index) const Q_DECL_OVERRIDE;

    void paint(QPainter *painter, const QStyleOptionViewItem &opt,
               const QModelIndex &index) const Q_DECL_OVERRIDE;
    QSize sizeHint(const QStyleOptionViewItem &opt, const QModelIndex &index) const Q_DECL_OVERRIDE;
private:
    QTreeView *m_view;
};

ColorDelegate::ColorDelegate(QTreeView *view, QObject *parent) :
    QItemDelegate(parent),
    m_view(view)
{
}

QWidget *ColorDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &,
                const QModelIndex &index) const
{
    QWidget *ed = 0;
    if (index.column() == 0) {
//        RoleEditor *editor = new RoleEditor(parent);
//        connect(editor, &RoleEditor::changed, this, &ColorDelegate::commitData);
        //editor->setFocusPolicy(Qt::NoFocus);
        //editor->installEventFilter(const_cast<ColorDelegate *>(this));
//        ed = editor;
    } else {
//        typedef void (BrushEditor::*BrushEditorWidgetSignal)(QWidget *);

//        BrushEditor *editor = new BrushEditor(m_view, parent);
//        connect(editor, static_cast<BrushEditorWidgetSignal>(&BrushEditor::changed),
//                this, &ColorDelegate::commitData);
//        editor->setFocusPolicy(Qt::NoFocus);
//        editor->installEventFilter(const_cast<ColorDelegate *>(this));
//        ed = editor;
    }
    return ed;
}

void ColorDelegate::setEditorData(QWidget *ed, const QModelIndex &index) const
{
    if (index.column() == 0) {
//        const bool mask = qvariant_cast<bool>(index.model()->data(index, Qt::EditRole));
//        RoleEditor *editor = static_cast<RoleEditor *>(ed);
//        editor->setEdited(mask);
//        const QString colorName = qvariant_cast<QString>(index.model()->data(index, Qt::DisplayRole));
//        editor->setLabel(colorName);
    } else {
//        const QBrush br = qvariant_cast<QBrush>(index.model()->data(index, BrushRole));
//        BrushEditor *editor = static_cast<BrushEditor *>(ed);
//        editor->setBrush(br);
    }
}

void ColorDelegate::setModelData(QWidget *ed, QAbstractItemModel *model,
                const QModelIndex &index) const
{
//    if (index.column() == 0) {
//        RoleEditor *editor = static_cast<RoleEditor *>(ed);
//        const bool mask = editor->edited();
//        model->setData(index, mask, Qt::EditRole);
//    } else {
//        BrushEditor *editor = static_cast<BrushEditor *>(ed);
//        if (editor->changed()) {
//            QBrush br = editor->brush();
//            model->setData(index, br, BrushRole);
//        }
//    }
}

void ColorDelegate::updateEditorGeometry(QWidget *ed,
                const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QItemDelegate::updateEditorGeometry(ed, option, index);
    ed->setGeometry(ed->geometry().adjusted(0, 0, -1, -1));
}

void ColorDelegate::paint(QPainter *painter, const QStyleOptionViewItem &opt,
            const QModelIndex &index) const
{
    const QAbstractItemModel *model = index.model();
    Q_ASSERT(model);

    QStyleOptionViewItem option = opt;
    const bool mask = qvariant_cast<bool>(index.model()->data(index, Qt::EditRole));
    if (!model->parent(index).isValid() && mask) {
        option.font.setBold(true);
    }
    QBrush br = qvariant_cast<QBrush>(index.model()->data(index, BrushRole));
    if (br.style() == Qt::LinearGradientPattern ||
            br.style() == Qt::RadialGradientPattern ||
            br.style() == Qt::ConicalGradientPattern) {
        painter->save();
        painter->translate(option.rect.x(), option.rect.y());
        painter->scale(option.rect.width(), option.rect.height());
        QGradient gr = *(br.gradient());
        gr.setCoordinateMode(QGradient::LogicalMode);
        br = QBrush(gr);
        painter->fillRect(0, 0, 1, 1, br);
        painter->restore();
    } else {
        painter->save();
        painter->setBrushOrigin(option.rect.x(), option.rect.y());
        painter->fillRect(option.rect, br);
        painter->restore();
    }
    QItemDelegate::paint(painter, option, index);


    const QColor color = static_cast<QRgb>(qApp->style()->styleHint(QStyle::SH_Table_GridLineColor, &option));
    const QPen oldPen = painter->pen();
    painter->setPen(QPen(color));

    if (index.column() == 0) {
        painter->drawLine(QPointF(0.5, option.rect.y()),
                          QPointF(0.5, option.rect.bottom()));
        if (model->parent(index).isValid()) {
            painter->drawLine(option.rect.right(), option.rect.y(),
                              option.rect.right(), option.rect.bottom());
        }
    } else {
        painter->drawLine(option.rect.right(), option.rect.y(),
                          option.rect.right(), option.rect.bottom());
    }

    painter->drawLine(QPointF(0.5, option.rect.bottom()),
                      QPointF(option.rect.right(), option.rect.bottom()));
    painter->setPen(oldPen);
}

QSize ColorDelegate::sizeHint(const QStyleOptionViewItem &opt, const QModelIndex &index) const
{
    return QItemDelegate::sizeHint(opt, index) + QSize(4, 4);
}

PropertiesWidget::PropertiesWidget(QWidget *parent) : QWidget(parent)
{
	setAutoFillBackground(true);
    QPalette p(palette());
    p.setColor(QPalette::Window, QColor("#E0E4E7"));
    setPalette(p);

    QPalette p2(_treeWidget.palette());
    p2.setColor(QPalette::Base, QColor("#F3F7FA"));
    p2.setColor(QPalette::Highlight, QColor("#D0D4D7"));
    p2.setColor(QPalette::Text, QColor("#202427"));
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
        if (property.second.isEmpty())
            continue;

        QTreeWidgetItem* item = new QTreeWidgetItem;
        item->setText(0, property.first);

        for (auto propertyName : property.second.keys()) {
            //            if (superClass->property(i).isWritable() &&
            //                !QString(superClass->property(i).name()).startsWith("__") &&
            //                !QString(superClass->property(i).read(object).typeName())
            //                .contains(QRegExp("Q([A-Za-z_][A-Za-z0-9_]*)\\*")))

            QTreeWidgetItem* iitem = new QTreeWidgetItem;
            iitem->setText(0, propertyName);
            iitem->setText(1, property.second[propertyName].typeName());
            item->addChild(iitem);
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
    return QSize(fit(280), fit(400));
}

#include "propertieswidget.moc"
