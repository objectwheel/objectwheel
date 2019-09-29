#include <toolboxtree.h>
#include <toolboxitem.h>
#include <toolboxdelegate.h>

#include <QApplication>
#include <QHeaderView>

ToolboxTree::ToolboxTree(QWidget* parent) : QTreeWidget(parent)
{
    QPalette p(palette());
    p.setColor(QPalette::Light, "#AB8157");
    p.setColor(QPalette::Dark, "#9C7650");
    p.setColor(QPalette::AlternateBase, "#f7efe6");
    setPalette(p);

    header()->setFixedHeight(23);
    header()->setDefaultSectionSize(1);
    header()->setMinimumSectionSize(1);
    header()->resizeSection(0, 165); // Don't resize the last (stretched) column

    headerItem()->setText(1, tr("Value"));
    headerItem()->setText(0, tr("Property"));

    setColumnCount(2);
    setIndentation(16);
    setDragEnabled(false);
    setUniformRowHeights(true);
    setDropIndicatorShown(false);
    setExpandsOnDoubleClick(true);
    setItemDelegate(new PropertiesListDelegate(this));
    setFocusPolicy(Qt::NoFocus);
    setAttribute(Qt::WA_MacShowFocusRect, false);
    setSelectionBehavior(QTreeWidget::SelectRows);
    setSelectionMode(QTreeWidget::NoSelection);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setVerticalScrollMode(QTreeWidget::ScrollPerPixel);
    setHorizontalScrollMode(QTreeWidget::ScrollPerPixel);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setStyleSheet(
                QString {
                    "QTreeView {"
                    "    border: 1px solid %1;"
                    "} QHeaderView::section {"
                    "    color: %4;"
                    "    padding-left: 5px;"
                    "    padding-top: 3px;"
                    "    padding-bottom: 3px;"
                    "    border-style: solid;"
                    "    border-left-width: 0px;"
                    "    border-top-width: 0px;"
                    "    border-bottom-color: %1;"
                    "    border-bottom-width: 1px;"
                    "    border-right-color: %1; "
                    "    border-right-width: 1px;"
                    "    background: qlineargradient(spread:pad, x1:0.5, y1:0, x2:0.5, y2:1,"
                    "                                stop:0 %2, stop:1 %3);"
                    "}"
                    "QHeaderView::section:last{"
                    "    border-left-width: 0px;"
                    "    border-right-width: 0px;"
                    "}"
                }
                .arg(palette().dark().color().darker(120).name())
                .arg(palette().light().color().name())
                .arg(palette().dark().color().name())
                .arg(palette().brightText().color().name())
                );
}