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
        QStyleOptionButton buttonOption;

        buttonOption.state = option.state;
#ifdef Q_OS_MACOS
        buttonOption.state |= QStyle::State_Raised;
#endif
        buttonOption.state &= ~QStyle::State_HasFocus;

        buttonOption.rect = option.rect;
        buttonOption.palette = option.palette;
        buttonOption.features = QStyleOptionButton::None;

        painter->save();
        QColor buttonColor(230, 230, 230);
        QBrush buttonBrush = option.palette.button();
        if (!buttonBrush.gradient() && buttonBrush.texture().isNull())
            buttonColor = buttonBrush.color();
        QColor outlineColor = buttonColor.darker(150);
        QColor highlightColor = buttonColor.lighter(130);

        // Only draw topline if the previous item is expanded
        QModelIndex previousIndex = model->index(index.row() - 1, index.column());
        bool drawTopline = (index.row() > 0 && m_view->isExpanded(previousIndex));
        int highlightOffset = drawTopline ? 1 : 0;

        QLinearGradient gradient(option.rect.topLeft(), option.rect.bottomLeft());
        gradient.setColorAt(0, QColor("#EAEEF1"));
        gradient.setColorAt(1, QColor("#D0D4D7"));

        painter->setPen(Qt::NoPen);
        painter->setBrush(gradient);
        painter->drawRect(option.rect);
        QPen p(highlightColor);
        p.setWidthF(fit::fx(0.5));
        painter->setPen(p);
        painter->drawLine(option.rect.topLeft() + QPoint(0, highlightOffset),
                          option.rect.topRight() + QPoint(0, highlightOffset));
        p.setColor(outlineColor);
        painter->setPen(p);
        if (drawTopline)
            painter->drawLine(option.rect.topLeft(), option.rect.topRight());
        painter->drawLine(option.rect.bottomLeft(), option.rect.bottomRight());
        painter->restore();

        QStyleOption branchOption;
        static const int i = fit::fx(9); // ### hardcoded in qcommonstyle.cpp
        QRect r = option.rect;
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

ToolboxTree::ToolboxTree(QWidget *parent)
    : QTreeWidget(parent)
    , _indicatorButtonVisible(false)
{
    _indicatorButton = new FlatButton(this);

    QPalette p2(palette());
    p2.setColor(QPalette::Base, "#F3F7FA");
    p2.setColor(QPalette::Highlight, "#d0d4d7");
    p2.setColor(QPalette::Text, "#202427");
    p2.setColor(QPalette::HighlightedText, "#202427");
    setAutoFillBackground(true);
    setPalette(p2);

    QScroller::grabGesture(viewport(), QScroller::TouchGesture);
    QScrollerProperties prop = QScroller::scroller(viewport())->scrollerProperties();
    prop.setScrollMetric(QScrollerProperties::VerticalOvershootPolicy, QScrollerProperties::OvershootAlwaysOff);
    prop.setScrollMetric(QScrollerProperties::HorizontalOvershootPolicy, QScrollerProperties::OvershootAlwaysOff);
    prop.setScrollMetric(QScrollerProperties::DragStartDistance, 0.009);
    QScroller::scroller(viewport())->setScrollerProperties(prop);

    _indicatorButton->setVisible(_indicatorButtonVisible);
    connect(this, &ToolboxTree::currentItemChanged, [=] {
        if (currentItem() == 0)
            return;
        if (model()->rowCount() < 1) {
            _indicatorButton->hide();
            return;
        }
        auto rect = visualItemRect(currentItem());
        _indicatorButton->move(rect.width() - _indicatorButton->width() - fit::fx(5),
          rect.y() + rect.height()/2.0 - _indicatorButton->height()/2.0);
        _indicatorButton->setVisible(_indicatorButtonVisible && currentItem()->parent() != 0);
    });

    connect(verticalScrollBar(), &QSlider::valueChanged , this, [=] {
        if (currentItem() == 0)
            return;
        if (model()->rowCount() < 1) {
            _indicatorButton->hide();
            return;
        }
        auto rect = visualItemRect(currentItem());
        _indicatorButton->move(rect.width() - _indicatorButton->width() - fit::fx(5),
                              rect.y() + rect.height()/2.0 - _indicatorButton->height()/2.0);
        _indicatorButton->setVisible(_indicatorButtonVisible && currentItem()->parent() != 0);
    });

    setStyleSheet(QString("QTreeWidget::item{padding:%1px 0;}").arg(fit::fx(1.5)));
    setIconSize(fit::fx(QSize{24, 24}));
    setFocusPolicy(Qt::NoFocus);
    setIndentation(0);
    setRootIsDecorated(false);
    setSortingEnabled(true);
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

FlatButton* ToolboxTree::indicatorButton()
{
    return _indicatorButton;
}

void ToolboxTree::setIndicatorButtonVisible(bool value)
{
    _indicatorButtonVisible = value;
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
