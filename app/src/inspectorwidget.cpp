#include <inspectorwidget.h>
#include <fit.h>
#include <designmanager.h>
#include <css.h>
#include <filemanager.h>
#include <savemanager.h>
#include <formswidget.h>

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
//! *********************** [InspectorWidget] ***********************
//!

InspectorWidget::InspectorWidget(QWidget* parent)
    : QWidget(parent)
    , _blockRefresh(false)
{
    setAutoFillBackground(true);
    QPalette p(palette());
    p.setColor(QPalette::Window, QColor("#E0E4E7"));
    setPalette(p);

    QPalette p2(_treeWidget.palette());
    p2.setColor(QPalette::Base, QColor("#F3F7FA"));
    p2.setColor(QPalette::Highlight, QColor("#d0d4d7"));
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
    _treeWidget.setColumnCount(2);
    _treeWidget.setIndentation(fit(12));
    _treeWidget.headerItem()->setText(0, "Controls");
    _treeWidget.headerItem()->setText(1, "Ui");
    _treeWidget.verticalScrollBar()->setStyleSheet(CSS::ScrollBar);
    _treeWidget.horizontalScrollBar()->setStyleSheet(CSS::ScrollBarH);
    _treeWidget.setRootIsDecorated(true);
    _treeWidget.setSortingEnabled(true);
    _treeWidget.viewport()->installEventFilter(this);
    _treeWidget.header()->resizeSection(0, fit(250));
    _treeWidget.header()->resizeSection(1, fit(50));

    connect(&_treeWidget, SIGNAL(itemClicked(QTreeWidgetItem*,int)),
      SLOT(handleClick(QTreeWidgetItem*,int)));
    connect(&_treeWidget, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
      SLOT(handleDoubleClick(QTreeWidgetItem*,int)));

    _layout.setSpacing(fit(2));
    _layout.setContentsMargins(fit(3), fit(3), fit(3), fit(3));
    _layout.addWidget(&_treeWidget);
    setLayout(&_layout);

    /* Prepare Properties Widget */
    connect(DesignManager::formScene(), SIGNAL(selectionChanged()),
      SLOT(refreshList()));
    connect(DesignManager::controlScene(), SIGNAL(selectionChanged()),
      SLOT(refreshList()));
    connect(DesignManager::instance(), SIGNAL(modeChanged()),
      SLOT(refreshList()));
    connect(ControlWatcher::instance(), SIGNAL(geometryChanged(Control*)),
      SLOT(refreshList()));
    connect(FormsWidget::instance(), SIGNAL(currentFormChanged()),
      SLOT(refreshList()));
    QTimer::singleShot(1500, [this] {
        connect(SaveManager::instance(), SIGNAL(databaseChanged()),
          SLOT(refreshList()));
    });
}

bool InspectorWidget::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == _treeWidget.viewport()) {
        if (event->type() == QEvent::Paint &&
          _treeWidget.topLevelItemCount() > 0) {
            QPainter painter(_treeWidget.viewport());
            const auto tli = _treeWidget.topLevelItem(0);
            const auto tlir = _treeWidget.visualItemRect(tli);
            const qreal ic = (_treeWidget.viewport()->height() +
              qAbs(tlir.y())) / (qreal)tlir.height();
            for (int i = 0; i < ic; i++) {
                if (i % 2) {
                    painter.fillRect(0, tlir.y() + i * tlir.height(),
                      _treeWidget.viewport()->width(),
                        tlir.height(), QColor("#E5E9EC"));
                }
            }
        }
        return false;
    } else {
        return QWidget::eventFilter(watched, event);
    }
}

void InspectorWidget::clearList()
{
    for (int i = 0; i < _treeWidget.topLevelItemCount(); ++i)
        qDeleteAll(_treeWidget.topLevelItem(i)->takeChildren());

    _treeWidget.clear();
}

void InspectorWidget::refreshList()
{
    if (_blockRefresh)
        return;

    clearList();

    auto cs = DesignManager::currentScene();
    auto scs = cs->selectedControls();
    auto mc = cs->mainControl();

    if (!mc)
        return;

    QTreeWidgetItem* item = new QTreeWidgetItem;
    item->setText(0, mc->id());
    item->setText(1, "Yes");

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

    auto items = tree(item);
    items << item;
    for (auto sc : scs) {
        for (auto i : items) {
            if (i->text(0) == sc->id()) {
                i->setSelected(true);
            }
        }
    }
}

void InspectorWidget::handleDoubleClick(QTreeWidgetItem* item, int)
{
    const auto id = item->text(0);
    const auto mc = DesignManager::currentScene()->mainControl();
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

    DesignManager::qmlEditorView()->addControl(c);
    if (DesignManager::qmlEditorView()->pinned())
        DesignManager::setMode(DesignManager::CodeEdit);
    DesignManager::qmlEditorView()->setMode(QmlEditorView::CodeEditor);
    DesignManager::qmlEditorView()->openControl(c);
    DesignManager::qmlEditorView()->raiseContainer();
}

void InspectorWidget::handleClick(QTreeWidgetItem* item, int)
{
    const auto id = item->text(0);
    const auto mc = DesignManager::currentScene()->mainControl();
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
    DesignManager::currentScene()->clearSelection();
    c->setSelected(true);
    _blockRefresh = false;
}

QSize InspectorWidget::sizeHint() const
{
    return QSize(fit(200), fit(400));
}
