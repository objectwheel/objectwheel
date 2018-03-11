#include <inspectorpane.h>
#include <fit.h>
#include <css.h>
#include <centralwidget.h>
#include <filemanager.h>
#include <saveutils.h>
#include <savebackend.h>
#include <formspane.h>
#include <control.h>
#include <controlwatcher.h>
#include <frontend.h>
#include <controlscene.h>
#include <formscene.h>
#include <mainwindow.h>

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

    const QPen oldPen = painter->pen();
    painter->setPen("#10000000");

    if (index.column() == 0) {
        if (model->parent(index).isValid()) {
            painter->drawLine(option.rect.right() + 0.5, option.rect.y() + 0.5,
                              option.rect.right() + 0.5, option.rect.bottom() + 0.5);
        }
    } else {
        painter->drawLine(QPointF(0.5, option.rect.bottom() + 0.5),
                          QPointF(option.rect.right() + 0.5, option.rect.bottom() + 0.5));
    }

    painter->setPen(oldPen);

    QStyledItemDelegate::paint(painter, o, index);
}

//!
//! *********************** [InspectorPane] ***********************
//!

InspectorPane::InspectorPane(MainWindow* parent)
    : QWidget(parent)
    , _blockRefresh(false)
{
    _layout = new QVBoxLayout(this);
    _treeWidget = new QTreeWidget;

    QPalette p(palette());
    p.setColor(QPalette::Base, QColor("#E0E4E7"));
    p.setColor(QPalette::Window, QColor("#E0E4E7"));
    setPalette(p);
    setAutoFillBackground(true);

    QPalette p2(_treeWidget->palette());
    p2.setColor(QPalette::All, QPalette::Base, QColor("#FAFFF9"));
    p2.setColor(QPalette::All, QPalette::Highlight, QColor("#cee7cb"));
    p2.setColor(QPalette::All, QPalette::Text, QColor("#202427"));
    p2.setColor(QPalette::All, QPalette::HighlightedText, QColor("#202427"));
    _treeWidget->setPalette(p2);

    _treeWidget->setHorizontalScrollMode(QTreeWidget::ScrollPerPixel);
    _treeWidget->setVerticalScrollMode(QTreeWidget::ScrollPerPixel);
    _treeWidget->setSelectionBehavior(QTreeWidget::SelectRows);
    _treeWidget->setFocusPolicy(Qt::NoFocus);
    _treeWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    _treeWidget->setDragEnabled(false);
    _treeWidget->setDropIndicatorShown(false);
    _treeWidget->setColumnCount(2);
    _treeWidget->setIndentation(fit::fx(16));
    _treeWidget->headerItem()->setText(0, "Controls");
    _treeWidget->headerItem()->setText(1, "Ui");
    _treeWidget->verticalScrollBar()->setStyleSheet(CSS::ScrollBar);
    _treeWidget->horizontalScrollBar()->setStyleSheet(CSS::ScrollBarH);
    _treeWidget->setSortingEnabled(true);
    _treeWidget->viewport()->installEventFilter(this);
    _treeWidget->header()->resizeSection(0, fit::fx(250));
    _treeWidget->header()->resizeSection(1, fit::fx(50));
    _treeWidget->setItemDelegate(new InspectorListDelegate(_treeWidget));
//    _treeWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    _treeWidget->header()->setFixedHeight(fit::fx(23));
    _treeWidget->setStyleSheet("QTreeView { border: 1px solid #4A7C42; }");
    _treeWidget->header()->setStyleSheet(
        "color: white; font-weight: Medium; border:none; border-bottom: 1px solid #4A7C42;"
        "background: qlineargradient(spread:pad, x1:0.5, y1:0, x2:0.5, y2:1, stop:0 #62A558, stop:1 #599750);"
    );

    connect(_treeWidget, SIGNAL(itemClicked(QTreeWidgetItem*,int)),
      SLOT(handleClick(QTreeWidgetItem*,int)));
    connect(_treeWidget, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
      SLOT(handleDoubleClick(QTreeWidgetItem*,int)));

    _layout->setSpacing(fit::fx(2));
    _layout->setContentsMargins(fit::fx(3), fit::fx(3), fit::fx(3), fit::fx(3));
    _layout->addWidget(_treeWidget);

    /* Prepare Properties Widget */
    connect(parent->centralWidget()->formScene(), SIGNAL(selectionChanged()),
      SLOT(refresh()));
    connect(parent->centralWidget()->controlScene(), SIGNAL(selectionChanged()),
      SLOT(refresh()));
    connect(parent->centralWidget(), SIGNAL(modeChanged()),
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
        if (event->type() == QEvent::Paint) {
            QPainter painter(_treeWidget->viewport());

            if (_treeWidget->topLevelItemCount() > 0) {
                const auto tli = _treeWidget->topLevelItem(0);
                const auto& tlir = _treeWidget->visualItemRect(tli);
                const qreal ic = (
                    _treeWidget->viewport()->height() +
                    qAbs(tlir.y())
                ) / (qreal) tlir.height();

                for (int i = 0; i < ic; i++) {
                    if (i % 2) {
                        painter.fillRect(
                            0,
                            tlir.y() + i * tlir.height(),
                            _treeWidget->viewport()->width(),
                            tlir.height(),
                            QColor("#ecfbea")
                        );
                    }
                }
            } else {
                const qreal hg = fit::fx(20.0);
                const qreal ic = _treeWidget->viewport()->height() / hg;

                for (int i = 0; i < ic; i++) {
                    if (i % 2) {
                        painter.fillRect(
                            0, i * hg,
                            _treeWidget->viewport()->width(),
                            hg, QColor("#ecfbea")
                        );
                    } else if (i == int(ic / 2.0) || i == int(ic / 2.0) + 1) {
                        painter.setPen(QColor("#a6afa5"));
                        painter.drawText(0, i * hg, _treeWidget->viewport()->width(),
                          hg, Qt::AlignCenter, "No items to show");
                    }
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

    auto cs = dW->currentScene();
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
        if (SaveUtils::isMain(mc->dir()))
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

    auto items = tree(item);
    items << item;
    for (auto sc : scs) {
        for (auto i : items) {
            if (i->text(0) == sc->id()) {
                i->setSelected(true);
            }
        }
    }

    _treeWidget->expandAll();
    _treeWidget->verticalScrollBar()->setSliderPosition(vsp);
    _treeWidget->horizontalScrollBar()->setSliderPosition(hsp);
}

void InspectorPane::handleDoubleClick(QTreeWidgetItem* item, int)
{
    const auto id = item->text(0);
    const auto mc = dW->currentScene()->mainControl();
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
    const auto mc = dW->currentScene()->mainControl();
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
    return fit::fx(QSizeF{200, 230}).toSize();
}

#include "inspectorpane.moc"
