#include <inspectorpane.h>
#include <css.h>
#include <centralwidget.h>
#include <filemanager.h>
#include <saveutils.h>
#include <savemanager.h>
#include <formspane.h>
#include <controlmonitoringmanager.h>
#include <designerscene.h>
#include <form.h>
#include <projectmanager.h>

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

        if (child->gui() && !child->hasErrors())
            item->setText(1, "Yes");
        else
            item->setText(1, "No");

        QIcon icon;
        icon.addFile(child->dir() + separator() + DIR_THIS +
                     separator() + "icon.png");
        if (icon.isNull())
            icon.addFile(":/images/item.png");
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
static bool _blockRefresh = false;

InspectorPane::InspectorPane(DesignerScene* designerScene, QWidget* parent) : QTreeWidget(parent)
  , m_designerScene(designerScene)
{
    QPalette p(palette());
    p.setColor(QPalette::Base, QColor("#ececec"));
    p.setColor(QPalette::Window, QColor("#ececec"));
    setPalette(p);
    setAutoFillBackground(true);

    QPalette p2(palette());
    p2.setColor(QPalette::All, QPalette::Base, QColor("#fefffc"));
    p2.setColor(QPalette::All, QPalette::Highlight, QColor("#cee7cb"));
    p2.setColor(QPalette::All, QPalette::Text, Qt::black);
    p2.setColor(QPalette::All, QPalette::HighlightedText, Qt::black);
    setPalette(p2);

    setHorizontalScrollMode(QTreeWidget::ScrollPerPixel);
    setVerticalScrollMode(QTreeWidget::ScrollPerPixel);
    setSelectionBehavior(QTreeWidget::SelectRows);
    setFocusPolicy(Qt::NoFocus);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setDragEnabled(false);
    setDropIndicatorShown(false);
    setColumnCount(2);
    setIndentation(16);
    headerItem()->setText(0, "Controls");
    headerItem()->setText(1, "Ui");
    verticalScrollBar()->setStyleSheet(CSS::ScrollBar);
    horizontalScrollBar()->setStyleSheet(CSS::ScrollBarH);
    viewport()->installEventFilter(this);
    header()->resizeSection(0, 250);
    header()->resizeSection(1, 50);
    setItemDelegate(new InspectorListDelegate(this));
//    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    header()->setFixedHeight(23);
    setStyleSheet("QTreeView { border: 1px solid #4A7C42; }"
                               "QHeaderView::section {"
                               "padding-left: 5px; color: white; border:none; border-bottom: 1px solid #4A7C42;"
                               "background: qlineargradient(spread:pad, x1:0.5, y1:0, x2:0.5, y2:1, stop:0 #62A558, stop:1 #599750);}");

    QFont f; f.setWeight(QFont::Medium);
    header()->setFont(f);

    connect(this, SIGNAL(itemClicked(QTreeWidgetItem*,int)),
      SLOT(onItemClick(QTreeWidgetItem*,int)));
    connect(this, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
      SLOT(onItemDoubleClick(QTreeWidgetItem*,int)));

//    _layout->setSpacing(2);
//    _layout->setContentsMargins(3, 3, 3, 3);
//    _layout->addWidget(_treeWidget);

    /* Prepare Properties Widget */
    connect(m_designerScene, SIGNAL(selectionChanged()), SLOT(refresh()));
    connect(m_designerScene, SIGNAL(currentFormChanged(Control*)), SLOT(refresh()));
//    connect(ControlMonitoringManager::instance(), SIGNAL(geometryChanged(Control*)), SLOT(refresh()));
//    connect(SaveManager::instance(), SIGNAL(databaseChanged()), SLOT(refresh()));
    connect(m_designerScene, &DesignerScene::controlAboutToBeRemoved, this, &InspectorPane::onControlRemove);
//    connect(m_designerScene, &DesignerScene::controlRemove);
}

void InspectorPane::reset()
{
    QTreeWidget::reset();
//    clear();
//    _blockRefresh = false;
}

bool InspectorPane::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == viewport()) {
        if (event->type() == QEvent::Paint) {
            QPainter painter(viewport());

            if (topLevelItemCount() > 0) {
                const auto tli = topLevelItem(0);
                const auto& tlir = visualItemRect(tli);
                const qreal ic = (
                    viewport()->height() +
                    qAbs(tlir.y())
                ) / (qreal) tlir.height();

                for (int i = 0; i < ic; i++) {
                    if (i % 2) {
                        painter.fillRect(
                            0,
                            tlir.y() + i * tlir.height(),
                            viewport()->width(),
                            tlir.height(),
                            QColor("#ecfbea")
                        );
                    }
                }
            } else {
                const qreal hg = 20.0;
                const qreal ic = viewport()->height() / hg;

                for (int i = 0; i < ic; i++) {
                    if (i % 2) {
                        painter.fillRect(
                            0, i * hg,
                            viewport()->width(),
                            hg, QColor("#ecfbea")
                        );
                    } else if (i == int(ic / 2.0) || i == int(ic / 2.0) + 1) {
                        painter.setPen(QColor("#a6afa5"));
                        painter.drawText(0, i * hg, viewport()->width(),
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

void InspectorPane::onControlAdd()
{

}

void InspectorPane::onControlRemove()
{

}

void InspectorPane::onSelectionChange()
{

}

void InspectorPane::onCurrentFormChange()
{

}

//void InspectorPane::clear()
//{
//    for (int i = 0; i < topLevelItemCount(); ++i)
//        qDeleteAll(topLevelItem(i)->takeChildren());

//    clear();
//}

//void InspectorPane::refresh()
//{
//    if (_blockRefresh)
//        return;

//    int vsp = verticalScrollBar()->sliderPosition();
//    int hsp = horizontalScrollBar()->sliderPosition();

////    clear();

//    auto scs = m_designerScene->selectedControls();
//    auto mc = m_designerScene->currentForm();

//    if (!mc)
//        return;

//    QTreeWidgetItem* item = new QTreeWidgetItem;
//    item->setText(0, mc->id());
//    item->setText(1, "Yes");
//    item->setData(0, Qt::UserRole, mc->hasErrors());
//    item->setData(1, Qt::UserRole, mc->hasErrors());

//    if (mc->hasErrors()) {
//        item->setTextColor(0, "#D02929");
//        item->setTextColor(1, "#D02929");
//    }

//    if (mc->form()) {
//        if (SaveUtils::isMain(mc->dir()))
//            item->setIcon(0, QIcon(":/images/mform.png"));
//        else
//            item->setIcon(0, QIcon(":/images/form.png"));
//    } else {
//        QIcon icon;
//        icon.addFile(mc->dir() + separator() + DIR_THIS +
//                     separator() + "icon.png");
//        if (icon.isNull())
//            icon.addFile(":/images/item.png");
//        item->setIcon(0, icon);
//    }

//    fillItem(item, mc->childControls(false));
//    addTopLevelItem(item);

//    auto items = tree(item);
//    items << item;
//    for (auto sc : scs) {
//        for (auto i : items) {
//            if (i->text(0) == sc->id()) {
//                i->setSelected(true);
//            }
//        }
//    }

//    expandAll();
//    verticalScrollBar()->setSliderPosition(vsp);
//    horizontalScrollBar()->setSliderPosition(hsp);
//}

void InspectorPane::onItemDoubleClick(QTreeWidgetItem* item, int)
{
    const auto id = item->text(0);
    const auto mc = m_designerScene->currentForm();
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

void InspectorPane::onItemClick(QTreeWidgetItem* item, int)
{
    const auto id = item->text(0);
    const auto mc = m_designerScene->currentForm();
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
    return QSize{200, 230};
}

#include "inspectorpane.moc"
