#include <inspectorpane.h>
#include <css.h>
#include <centralwidget.h>
#include <filemanager.h>
#include <saveutils.h>
#include <savemanager.h>
#include <formspane.h>
#include <controlmonitoringmanager.h>
#include <controlexposingmanager.h>
#include <controlremovingmanager.h>
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
#include <QPointer>

//!
//! *************************** [global] ****************************
//!
namespace {

QList<QTreeWidgetItem*> topLevelItems(const QTreeWidget* treeWidget)
{
    QList<QTreeWidgetItem*> items;

    if (!treeWidget)
        return items;

    for (int i = 0; i < treeWidget->topLevelItemCount(); ++i)
        items.append(treeWidget->topLevelItem(i));

    return items;
}

QList<QTreeWidgetItem*> allSubChildItems(QTreeWidgetItem* parentItem, bool includeParent = true)
{
    QList<QTreeWidgetItem*> items;

    if (!parentItem)
        return items;

    if (includeParent)
        items.append(parentItem);

    for (int i = 0; i < parentItem->childCount(); i++) {
        items.append(parentItem->child(i));
        items.append(allSubChildItems(parentItem->child(i), false));
    }

    return items;
}

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
    setSelectionMode(QTreeWidget::ExtendedSelection);
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

    header()->setFixedHeight(23);
    setStyleSheet("QTreeView {"
                  "  border: 1px solid #4A7C42;"
                  "} QHeaderView::section {"
                  "  padding-left: 5px;"
                  "  color: white;"
                  "  border: none;"
                  "  border-bottom: 1px solid #4A7C42;"
                  "  background: qlineargradient(spread:pad, x1:0.5, y1:0, x2:0.5, y2:1,"
                  "                              stop:0 #62A558, stop:1 #599750);"
                  "}");

    QFont f;
    f.setWeight(QFont::Medium);
    header()->setFont(f);

    //    _layout->setSpacing(2);
    //    _layout->setContentsMargins(3, 3, 3, 3);
    //    _layout->addWidget(_treeWidget);

    connect(this, &InspectorPane::itemClicked, this, &InspectorPane::onItemClick);
    connect(this, &InspectorPane::itemDoubleClicked, this, &InspectorPane::onItemDoubleClick);
    connect(m_designerScene, &DesignerScene::selectionChanged, this, &InspectorPane::onSelectionChange);
    connect(m_designerScene, &DesignerScene::currentFormChanged, this, &InspectorPane::onCurrentFormChange);
    connect(ControlRemovingManager::instance(), &ControlRemovingManager::controlAboutToBeRemoved,
            this, &InspectorPane::onControlRemove);
    connect(ControlExposingManager::instance(), &ControlExposingManager::controlExposed,
            this, &InspectorPane::onControlAdd);
    connect(ControlRemovingManager::instance(), &ControlRemovingManager::formAboutToBeRemoved,
            this, &InspectorPane::onFormRemove);

    // TODO: Handle reparent operations
    // TODO: Handle id change of a control
    // TODO: Handle code changes (ui, non-ui, id change, icon change)
    //    connect(ControlMonitoringManager::instance(), SIGNAL(geometryChanged(Control*)), SLOT(refresh()));
    //    connect(SaveManager::instance(), SIGNAL(databaseChanged()), SLOT(refresh()));
}

void InspectorPane::sweep()
{
    m_formStates.clear();
    _blockRefresh = false;
    clear();
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

void InspectorPane::onSelectionChange()
{    
    for (QTreeWidgetItem* topLevelItem : topLevelItems(this)) {
        for (QTreeWidgetItem* childItem : allSubChildItems(topLevelItem))
                childItem->setSelected(false);
    }

    for (const Control* selectedControl : m_designerScene->selectedControls()) {
        for (QTreeWidgetItem* topLevelItem : topLevelItems(this)) {
            for (QTreeWidgetItem* childItem : allSubChildItems(topLevelItem)) {
                if (selectedControl->id() == childItem->text(0))
                    childItem->setSelected(true);
            }
        }
    }
}

void InspectorPane::onCurrentFormChange(Form* currentForm)
{
    if (!currentForm)
        return;

    if (_blockRefresh)
        return;

    /* Save outgoing form's state */
    if (m_currentForm) {
        FormState state;
        state.verticalScrollBarPosition = verticalScrollBar()->sliderPosition();
        state.horizontalScrollBarPosition = horizontalScrollBar()->sliderPosition();

        for (QTreeWidgetItem* topLevelItem : topLevelItems(this)) {
            for (QTreeWidgetItem* childItem : allSubChildItems(topLevelItem)) {
                if (childItem->isSelected())
                    state.selectedIds << childItem->text(0);

                if (!childItem->isExpanded())
                    state.collapsedIds << childItem->text(0);
            }
        }

        m_formStates[m_currentForm] = state;
    }

    /* Clear things */
    m_currentForm = currentForm;
    clear();

    /* Create items for incoming form */
    auto formItem = new QTreeWidgetItem;
    formItem->setText(0, currentForm->id());
    formItem->setText(1, "Yes");
    formItem->setData(0, Qt::UserRole, currentForm->hasErrors());
    formItem->setData(1, Qt::UserRole, currentForm->hasErrors());
    formItem->setIcon(0, SaveUtils::isMain(currentForm->dir()) ? QIcon(":/images/mform.png") :
                                                                 QIcon(":/images/form.png"));

    if (currentForm->hasErrors()) {
        formItem->setTextColor(0, "#D02929");
        formItem->setTextColor(1, "#D02929");
    }

    fillItem(formItem, currentForm->childControls(false));
    addTopLevelItem(formItem);
    expandAll();

    /* Restore incoming form's state */
    const FormState& state = m_formStates.value(currentForm);
    for (QTreeWidgetItem* topLevelItem : topLevelItems(this)) {
        for (QTreeWidgetItem* childItem : allSubChildItems(topLevelItem)) {
            for (const QString& id : state.selectedIds) {
                if (id == childItem->text(0))
                    childItem->setSelected(true);
            }

            for (const QString& id : state.collapsedIds) {
                if (id == childItem->text(0))
                    childItem->setExpanded(false);
            }
        }
    }

    verticalScrollBar()->setSliderPosition(state.verticalScrollBarPosition);
    horizontalScrollBar()->setSliderPosition(state.horizontalScrollBarPosition);
}

void InspectorPane::onFormRemove(Form* form)
{
    m_formStates.remove(form);
}

void InspectorPane::onControlAdd(Control* control)
{
    const Control* parentControl = control->parentControl();
    for (QTreeWidgetItem* topLevelItem : topLevelItems(this)) {
        for (QTreeWidgetItem* childItem : allSubChildItems(topLevelItem)) {
            if (parentControl->id() == childItem->text(0)) {
                fillItem(childItem, QList<Control*>() << control);
                break;
            }
        }
    }
}

void InspectorPane::onControlRemove(Control* control)
{
    for (QTreeWidgetItem* topLevelItem : topLevelItems(this)) {
        for (QTreeWidgetItem* childItem : allSubChildItems(topLevelItem)) {
            if (control->id() == childItem->text(0)) {
                topLevelItem->removeChild(childItem);
                delete childItem;
                break;
            }
        }
    }
}

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
//    const QString& id = item->text(0);
//    const Form* currentForm = m_designerScene->currentForm();

//    QList<Control*> childControls;
//    childControls.append(currentForm);
//    childControls.append(currentForm->childControls());

//    Control* c = nullptr;
//    for (auto control : cl) {
//        if (control->id() == id) {
//            c = control;
//            break;
//        }
//    }

//    if (c == nullptr)
//        return;

//    _blockRefresh = true;
//    emit controlClicked(c);
//    _blockRefresh = false;
}

QSize InspectorPane::sizeHint() const
{
    return QSize{200, 230};
}

#include "inspectorpane.moc"
