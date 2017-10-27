#include <inspectorwidget.h>
#include <fit.h>
#include <designmanager.h>
#include <css.h>
#include <filemanager.h>
#include <savemanager.h>
#include <QStyledItemDelegate>
#include <QTreeWidget>
#include <QPainter>
#include <QApplication>
#include <QHeaderView>

using namespace Fit;

//!
//! *************************** [global] ****************************
//!

void fillItem(QTreeWidgetItem* parentItem, const QList<Control*>& children)
{
    for (auto child : children) {
        QTreeWidgetItem* item = new QTreeWidgetItem;
        item->setText(0, child->id());

        QIcon icon;
        icon.addFile(child->dir() + separator() + DIR_THIS +
                     separator() + "icon.png");
        if (icon.isNull())
            icon.addFile(":/resources/images/item.png");
        item->setIcon(0, icon);

        parentItem->addChild(item);
        fillItem(item, child->childControls(false));
    }
}

//!
//! ********************** [InspectorDelegate] **********************
//!

class InspectorDelegate : public QStyledItemDelegate
{
        Q_OBJECT
    public:
        explicit InspectorDelegate(QTreeWidget* view, QObject* parent = 0);
        void paint(QPainter* painter, const QStyleOptionViewItem &opt,
                   const QModelIndex &index) const override;

    private:
        QTreeWidget* m_view;
};

InspectorDelegate::InspectorDelegate(QTreeWidget* view, QObject* parent)
    : QStyledItemDelegate(parent)
    , m_view(view)
{
}

void InspectorDelegate::paint(QPainter* painter, const QStyleOptionViewItem &opt,
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
        option.font.setWeight(QFont::DemiBold);
    }

    QStyledItemDelegate::paint(painter, option, index);

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

//!
//! *********************** [InspectorWidget] ***********************
//!

InspectorWidget::InspectorWidget(QWidget* parent) : QWidget(parent)
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

    _treeWidget.setHorizontalScrollMode(QTreeWidget::ScrollPerPixel);
    _treeWidget.setVerticalScrollMode(QTreeWidget::ScrollPerPixel);
    _treeWidget.setSelectionBehavior(QTreeWidget::SelectRows);
    _treeWidget.setFocusPolicy(Qt::NoFocus);
    _treeWidget.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    _treeWidget.setDragEnabled(false);
    _treeWidget.setDropIndicatorShown(false);
    _treeWidget.setColumnCount(1);
    _treeWidget.headerItem()->setText(0, "Controls");
    _treeWidget.verticalScrollBar()->setStyleSheet(CSS::ScrollBar);
    _treeWidget.horizontalScrollBar()->setStyleSheet(CSS::ScrollBarH);
//    _treeWidget.setIndentation(fit(10));
    //    _treeWidget.setItemDelegate(new InspectorDelegate(&_treeWidget, &_treeWidget));
    _treeWidget.setRootIsDecorated(true);

    _layout.setSpacing(fit(2));
    _layout.setContentsMargins(fit(3), fit(3), fit(3), fit(3));
    _layout.addWidget(&_treeWidget);
    setLayout(&_layout);

    /* Prepare Properties Widget */
    connect(DesignManager::formScene(), SIGNAL(selectionChanged()), SLOT(refreshList()));
    connect(DesignManager::controlScene(), SIGNAL(selectionChanged()), SLOT(refreshList()));
    connect(DesignManager::instance(), SIGNAL(modeChanged()), SLOT(refreshList()));
    connect(ControlWatcher::instance(), SIGNAL(geometryChanged()), SLOT(refreshList()));
}

QSize InspectorWidget::sizeHint() const
{
    return QSize(fit(200), fit(200));
}

void InspectorWidget::clearList()
{
    for (int i = 0; i < _treeWidget.topLevelItemCount(); ++i)
        qDeleteAll(_treeWidget.topLevelItem(i)->takeChildren());

    _treeWidget.clear();
}

void InspectorWidget::refreshList()
{
    clearList();

    auto cs = DesignManager::currentScene();
    auto scs = cs->selectedControls();
    auto mc = cs->mainControl();


    QTreeWidgetItem* item = new QTreeWidgetItem;
    item->setText(0, mc->id());

    if (mc->form()) {
        item->setIcon(0, QIcon(":/resources/images/frm.png"));
    } else {
        QIcon icon;
        icon.addFile(mc->dir() + separator() + DIR_THIS +
                     separator() + "icon.png");
        if (icon.isNull())
            icon.addFile(":/resources/images/item.png");
        item->setIcon(0, icon);
    }

    fillItem(item, mc->childControls(false));
    _treeWidget.addTopLevelItem(item);
    _treeWidget.expandAll();
}

#include "inspectorwidget.moc"
