#include <inspectorpane.h>
#include <fit.h>
#include <css.h>
#include <designerwidget.h>
#include <filemanager.h>
#include <savebackend.h>
#include <formspane.h>
#include <controlwatcher.h>

#include <QVBoxLayout>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QHeaderView>
#include <QScrollBar>
#include <QTimer>

//!
//! *************************** [global] ****************************
//!

void fillItem(QTreeWidgetItem* parentItem, const QList<Control*>& children)
{
    for (auto child : children) {
        QTreeWidgetItem* item = new QTreeWidgetItem;
        item->setText(0, child->id());
        item->setData(0, Qt::UserRole, child->hasErrors());
        item->setData(1, Qt::UserRole, child->hasErrors());

        if (child->hasErrors()) {
            item->setTextColor(0, "#D02929");
            item->setTextColor(1, "#D02929");
        }

        if (child->gui())
            item->setText(1, "Yes");
        else
            item->setText(1, "No");

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

QList<QTreeWidgetItem*> tree(QTreeWidgetItem* item)
{
    QList<QTreeWidgetItem*> items;
    for (int i = 0; i < item->childCount(); i++) {
        items << item->child(i);
        items << tree(item->child(i));
    }
    return items;
}

//!
//! *********************** [InspectorListDelegate] ***********************
//!

class InspectorListDelegate: public QStyledItemDelegate
{
        Q_OBJECT

    public:
        InspectorListDelegate(QWidget* parent);
        void paint(QPainter* painter, const QStyleOptionViewItem &option,
          const QModelIndex &index) const override;
};

InspectorListDelegate::InspectorListDelegate(QWidget* parent)
    : QStyledItemDelegate(parent)
{
}

void InspectorListDelegate::paint(QPainter* painter, const QStyleOptionViewItem &option,
    const QModelIndex &index) const
{
    const QAbstractItemModel* model = index.model();
    Q_ASSERT(model);
    painter->setRenderHint(QPainter::Antialiasing);

    auto o = option;
    if (model->data(index, Qt::UserRole).toBool() &&
      option.state & QStyle::State_Selected) {
        QPalette p(o.palette);
        p.setColor(QPalette::HighlightedText, "#D02929");
        o.palette = p;
    }

    QStyledItemDelegate::paint(painter, o, index);
}

//!
//! *********************** [InspectorPane] ***********************
//!

InspectorPane::InspectorPane(QWidget* parent)
    : QWidget(parent)
    , _blockRefresh(false)
{
    _layout = new QVBoxLayout(this);
    _treeWidget = new QTreeWidget;

    QPalette p(palette());
    p.setColor(QPalette::Window, QColor("#E0E4E7"));
    setPalette(p);
    setAutoFillBackground(true);

    QPalette p2(_treeWidget->palette());
    p2.setColor(QPalette::Base, QColor("#F3F7FA"));
    p2.setColor(QPalette::Highlight, QColor("#d0d4d7"));
    p2.setColor(QPalette::Text, QColor("#202427"));
    p2.setColor(QPalette::HighlightedText, QColor("#202427"));
    _treeWidget->setPalette(p2);

    _treeWidget->setHorizontalScrollMode(QTreeWidget::ScrollPerPixel);
    _treeWidget->setVerticalScrollMode(QTreeWidget::ScrollPerPixel);
    _treeWidget->setSelectionBehavior(QTreeWidget::SelectRows);
    _treeWidget->setFocusPolicy(Qt::NoFocus);
    _treeWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    _treeWidget->setDragEnabled(false);
    _treeWidget->setDropIndicatorShown(false);
    _treeWidget->setColumnCount(2);
    _treeWidget->setIndentation(fit::fx(14));
    _treeWidget->headerItem()->setText(0, "Controls");
    _treeWidget->headerItem()->setText(1, "Ui");
    _treeWidget->verticalScrollBar()->setStyleSheet(CSS::ScrollBar);
    _treeWidget->horizontalScrollBar()->setStyleSheet(CSS::ScrollBarH);
    _treeWidget->setRootIsDecorated(true);
    _treeWidget->setSortingEnabled(true);
    _treeWidget->viewport()->installEventFilter(this);
    _treeWidget->header()->resizeSection(0, fit::fx(250));
    _treeWidget->header()->resizeSection(1, fit::fx(50));
    _treeWidget->setItemDelegate(new InspectorListDelegate(_treeWidget));

    connect(_treeWidget, SIGNAL(itemClicked(QTreeWidgetItem*,int)),
      SLOT(handleClick(QTreeWidgetItem*,int)));
    connect(_treeWidget, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
      SLOT(handleDoubleClick(QTreeWidgetItem*,int)));

    _layout->setSpacing(fit::fx(2));
    _layout->setContentsMargins(fit::fx(3), fit::fx(3), fit::fx(3), fit::fx(3));
    _layout->addWidget(_treeWidget);

    /* Prepare Properties Widget */
    connect(DesignerWidget::formScene(), SIGNAL(selectionChanged()),
      SLOT(refresh()));
    connect(DesignerWidget::controlScene(), SIGNAL(selectionChanged()),
      SLOT(refresh()));
    connect(DesignerWidget::instance(), SIGNAL(modeChanged()),
      SLOT(refresh()));
    connect(ControlWatcher::instance(), SIGNAL(geometryChanged(Control*)),
      SLOT(refresh()));
//    connect(FormsPane::instance(), SIGNAL(currentFormChanged()),
//      SLOT(refresh())); //FIXME
    QTimer::singleShot(3000, [this] {
        connect(SaveBackend::instance(), SIGNAL(databaseChanged()),
          SLOT(refresh()));
    });
}

bool InspectorPane::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == _treeWidget->viewport()) {
        if (event->type() == QEvent::Paint &&
          _treeWidget->topLevelItemCount() > 0) {
            QPainter painter(_treeWidget->viewport());
            const auto tli = _treeWidget->topLevelItem(0);
            const auto tlir = _treeWidget->visualItemRect(tli);
            const qreal ic = (_treeWidget->viewport()->height() +
              qAbs(tlir.y())) / (qreal)tlir.height();
            for (int i = 0; i < ic; i++) {
                if (i % 2) {
                    painter.fillRect(0, tlir.y() + i * tlir.height(),
                      _treeWidget->viewport()->width(),
                        tlir.height(), QColor("#E5E9EC"));
                }
            }
        }
        return false;
    } else {
        return QWidget::eventFilter(watched, event);
    }
}

void InspectorPane::clear()
{
    for (int i = 0; i < _treeWidget->topLevelItemCount(); ++i)
        qDeleteAll(_treeWidget->topLevelItem(i)->takeChildren());

    _treeWidget->clear();
}

void InspectorPane::refresh()
{
    if (_blockRefresh)
        return;

    int vsp = _treeWidget->verticalScrollBar()->sliderPosition();
    int hsp = _treeWidget->horizontalScrollBar()->sliderPosition();

    clear();

    auto cs = DesignerWidget::currentScene();
    auto scs = cs->selectedControls();
    auto mc = cs->mainControl();

    if (!mc)
        return;

    QTreeWidgetItem* item = new QTreeWidgetItem;
    item->setText(0, mc->id());
    item->setText(1, "Yes");
    item->setData(0, Qt::UserRole, mc->hasErrors());
    item->setData(1, Qt::UserRole, mc->hasErrors());

    if (mc->hasErrors()) {
        item->setTextColor(0, "#D02929");
        item->setTextColor(1, "#D02929");
    }

    if (mc->form()) {
        if (SaveBackend::isMain(mc->dir()))
            item->setIcon(0, QIcon(":/resources/images/mform.png"));
        else
            item->setIcon(0, QIcon(":/resources/images/form.png"));
    } else {
        QIcon icon;
        icon.addFile(mc->dir() + separator() + DIR_THIS +
                     separator() + "icon.png");
        if (icon.isNull())
            icon.addFile(":/resources/images/item.png");
        item->setIcon(0, icon);
    }

    fillItem(item, mc->childControls(false));
    _treeWidget->addTopLevelItem(item);
    _treeWidget->expandAll();
    _treeWidget->setItemsExpandable(false);

    auto items = tree(item);
    items << item;
    for (auto sc : scs) {
        for (auto i : items) {
            if (i->text(0) == sc->id()) {
                i->setSelected(true);
            }
        }
    }

    _treeWidget->verticalScrollBar()->setSliderPosition(vsp);
    _treeWidget->horizontalScrollBar()->setSliderPosition(hsp);
}

void InspectorPane::handleDoubleClick(QTreeWidgetItem* item, int)
{
    const auto id = item->text(0);
    const auto mc = DesignerWidget::currentScene()->mainControl();
    QList<Control*> cl;
    cl << mc;
    cl << mc->childControls();

    Control* c = nullptr;
    for (auto control : cl) {
        if (control->id() == id) {
            c = control;
            break;
        }
    }

    if (c == nullptr)
        return;

    emit controlDoubleClicked(c);
}

void InspectorPane::handleClick(QTreeWidgetItem* item, int)
{
    const auto id = item->text(0);
    const auto mc = DesignerWidget::currentScene()->mainControl();
    QList<Control*> cl;
    cl << mc;
    cl << mc->childControls();

    Control* c = nullptr;
    for (auto control : cl) {
        if (control->id() == id) {
            c = control;
            break;
        }
    }

    if (c == nullptr)
        return;

    _blockRefresh = true;
    emit controlClicked(c);
    _blockRefresh = false;
}

QSize InspectorPane::sizeHint() const
{
    return fit::fx(QSizeF{200, 400}).toSize();
}

#include "inspectorwidget.moc"
