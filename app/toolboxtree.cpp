#include <toolboxtree.h>
#include <fit.h>
#include <css.h>
#include <flatbutton.h>
#include <global.h>

#include <QMimeData>
#include <QApplication>
#include <QMouseEvent>
#include <QScrollBar>
#include <QScroller>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QHeaderView>

//!
//! *********************** [ToolboxDelegate] ***********************
//!

const char* TOOL_KEY = "QURBUEFaQVJMSVlJWiBIQUZJWg";

class ToolboxDelegate: public QStyledItemDelegate
{
        Q_OBJECT

    public:
        ToolboxDelegate(QTreeView* view, QWidget* parent);

        void paint(QPainter* painter, const QStyleOptionViewItem &option,
          const QModelIndex &index) const override;

        QSize sizeHint(const QStyleOptionViewItem &opt,
          const QModelIndex &index) const override;

    private:
        QTreeView* m_view;
};

ToolboxDelegate::ToolboxDelegate(QTreeView* view, QWidget* parent)
    : QStyledItemDelegate(parent)
    , m_view(view)
{
}

void ToolboxDelegate::paint(QPainter* painter, const QStyleOptionViewItem &option,
    const QModelIndex &index) const
{
    const QAbstractItemModel* model = index.model();
    Q_ASSERT(model);
    painter->setRenderHint(QPainter::Antialiasing);

    if (!model->parent(index).isValid()) {
        // this is a top-level item.
        painter->save();

        // Only draw topline if the previous item is expanded
        QModelIndex previousIndex = model->index(index.row() - 1, index.column());
        bool drawTopline = (index.row() > 0 && m_view->isExpanded(previousIndex));

        auto frame = option.rect;

        QLinearGradient gradient(frame.topLeft(), frame.bottomLeft());
        gradient.setColorAt(0, QColor("#fafafa"));
        gradient.setColorAt(1, QColor("#e3e3e3"));
        painter->setPen(Qt::NoPen);
        painter->setBrush(gradient);
        painter->drawRect(frame);

        painter->setPen("#d0d0d0");
        if (drawTopline)
            painter->drawLine(frame.topLeft(), frame.topRight());
        painter->drawLine(frame.bottomLeft(), frame.bottomRight());
        painter->restore();

        QStyleOption branchOption;
        static const int i = fit::fx(9); // ### hardcoded in qcommonstyle.cpp
        QRect r = frame;
        branchOption.rect = QRect(r.left() + i/2, r.top() + (r.height() - i)/2, i, i);
        branchOption.state = QStyle::State_Children;

        if (m_view->isExpanded(index))
            branchOption.state |= QStyle::State_Open;

        qApp->style()->drawPrimitive(QStyle::PE_IndicatorBranch, &branchOption, painter, m_view);

        // draw text
        QRect textrect = QRect(r.left() + i*2, r.top(), r.width() - ((5*i)/2), r.height());
        QString text = elidedText(option.fontMetrics, textrect.width(), Qt::ElideMiddle,
                                  model->data(index, Qt::DisplayRole).toString());
        m_view->style()->drawItemText(painter, textrect, Qt::AlignCenter,
                                      option.palette, m_view->isEnabled(), text);

    } else {
        QStyledItemDelegate::paint(painter, option, index);
    }
}

QSize ToolboxDelegate::sizeHint(const QStyleOptionViewItem &opt, const QModelIndex &index) const
{
    QStyleOptionViewItem option = opt;
    QSize sz = QStyledItemDelegate::sizeHint(opt, index) + QSize(2, 2);
    return sz;
}

//!
//! ********************** [ToolboxTree] **********************
//!

ToolboxTree::ToolboxTree(QWidget *parent) : QTreeWidget(parent)
{
    QPalette p2(palette());
    p2.setColor(QPalette::Base, Qt::white);
    p2.setColor(QPalette::Highlight, "#d0d0d0");
    p2.setColor(QPalette::Text, Qt::black);
    p2.setColor(QPalette::HighlightedText, Qt::black);
    setAutoFillBackground(true);
    setPalette(p2);

    setIconSize(fit::fx(QSize{22, 22}));
    setFocusPolicy(Qt::NoFocus);
    setIndentation(0);
    setRootIsDecorated(false);
    setColumnCount(1);
    header()->hide();
    header()->setSectionResizeMode(QHeaderView::Stretch);
    setTextElideMode(Qt::ElideMiddle);
    verticalScrollBar()->setStyleSheet(CSS::ScrollBar);

    setDragEnabled(true);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setDragDropMode(QAbstractItemView::DragOnly);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    setItemDelegate(new ToolboxDelegate(this, this));
}

void ToolboxTree::addUrls(QTreeWidgetItem* item, const QList<QUrl>& urls)
{
    _urls.insert(item, urls);
}

void ToolboxTree::removeUrls(QTreeWidgetItem* item)
{
    _urls.remove(item);
}

void ToolboxTree::clearUrls()
{
    _urls.clear();
}

const QMap<QTreeWidgetItem*, QList<QUrl>>& ToolboxTree::allUrls() const
{
    return _urls;
}

QList<QUrl> ToolboxTree::urls(QTreeWidgetItem* item) const
{
    return _urls.value(item);
}

bool ToolboxTree::contains(const QString& itemName)
{
    for (int i = 0; i < topLevelItemCount(); i++)
        for (int j = 0; j < topLevelItem(i)->childCount(); j++)
            if (topLevelItem(i)->child(j)->text(0) == itemName)
                return true;
    return false;
}

bool ToolboxTree::categoryContains(const QString& categoryName)
{
    for (int i = 0; i < topLevelItemCount(); i++)
        if (topLevelItem(i)->text(0) == categoryName)
            return true;
    return false;
}

QTreeWidgetItem* ToolboxTree::categoryItem(const QString& categoryName)
{
    for (int i = 0; i < topLevelItemCount(); i++)
        if (topLevelItem(i)->text(0) == categoryName)
            return topLevelItem(i);
    return nullptr;
}

QMimeData* ToolboxTree::mimeData(const QList<QTreeWidgetItem*> items) const
{
    if (itemAt(_pressPoint) &&
      itemAt(_pressPoint)->parent() != nullptr) {
        QMimeData *data = QTreeWidget::mimeData(items);
        data->setUrls(_urls[items[0]]);
        data->setText(TOOL_KEY);
        return data;
    } else {
        return nullptr;
    }
}

void ToolboxTree::mousePressEvent(QMouseEvent* event)
{
    _pressPoint = event->pos();
    QTreeWidget::mousePressEvent(event);
}

#include "toolboxtree.moc"
