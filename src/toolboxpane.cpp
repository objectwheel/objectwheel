#include <toolboxpane.h>
#include <toolsbackend.h>
#include <filemanager.h>
#include <savebackend.h>
#include <toolboxtree.h>
#include <flatbutton.h>
#include <filterlineedit.h>
#include <zipper.h>
#include <fit.h>
#include <mainwindow.h>

#include <QApplication>
#include <QTreeWidget>
#include <QVBoxLayout>

#define DURATION 500

ToolboxPane::ToolboxPane(MainWindow* parent) : QWidget(parent)
{
    _layout = new QVBoxLayout(this);
    _searchEdit = new FilterLineEdit;
    _toolboxTree = new ToolboxTree;

    QPalette p(palette());
    p.setColor(backgroundRole(), "#E0E4E7");
    setAutoFillBackground(true);
    setPalette(p);

    _toolboxTree->setIndicatorButtonVisible(true);
    _toolboxTree->indicatorButton()->setRadius(fit::fx(7));
    _toolboxTree->indicatorButton()->setColor(QColor("#0D74C8"));
    _toolboxTree->indicatorButton()->setFixedSize(fit::fx(15), fit::fx(15));
    _toolboxTree->indicatorButton()->setIconSize(QSize(fit::fx(10), fit::fx(10)));
    _toolboxTree->indicatorButton()->setIcon(QIcon(":/resources/images/right-arrow.png"));
    connect(_toolboxTree, &QTreeWidget::itemPressed, this, &ToolboxPane::handleMousePress);

    _searchEdit->setFixedHeight(fit::fx(22));
    _searchEdit->setClearButtonEnabled(true);
    _searchEdit->setPlaceholderText("Filter");
    connect(_searchEdit, SIGNAL(textChanged(QString)), SLOT(filterList(QString)));

    _layout->addWidget(_searchEdit);
    _layout->addWidget(_toolboxTree);
    _layout->setSpacing(fit::fx(2));
    _layout->setContentsMargins(fit::fx(3), fit::fx(3), fit::fx(3), fit::fx(3));

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

ToolboxTree* ToolboxPane::toolboxTree()
{
    return _toolboxTree;
}

void ToolboxPane::handleMousePress(QTreeWidgetItem* item)
{
    if (item == 0)
        return;

    if (QApplication::mouseButtons() != Qt::LeftButton)
        return;

    if (item->parent() == 0) {
        _toolboxTree->setItemExpanded(item,
          !_toolboxTree->isItemExpanded(item));
        return;
    }
}

void ToolboxPane::filterList(const QString& filter)
{
    for (int i = 0; i < _toolboxTree->topLevelItemCount(); i++) {
        auto tli = _toolboxTree->topLevelItem(i);
        auto tlv = false;

        for (int j = 0; j < tli->childCount(); j++) {
            auto tci = tli->child(j);
            auto v = filter.isEmpty() ? true :
                tci->text(0).contains(filter, Qt::CaseInsensitive);

            tci->setHidden(!v);
            if (v)
                tlv = v;
        }

        auto v = filter.isEmpty() ? true : tlv;
        tli->setHidden(!v);
    }
}

QSize ToolboxPane::sizeHint() const
{
    return fit::fx(QSizeF{200, 600}).toSize();
}
