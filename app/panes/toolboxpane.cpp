#include <toolboxpane.h>
#include <toolmanager.h>
#include <toolboxtree.h>
#include <lineedit.h>
#include <mainwindow.h>
#include <paintutils.h>

#include <QApplication>
#include <QTreeWidget>
#include <QVBoxLayout>

ToolboxPane::ToolboxPane(QWidget* parent) : QWidget(parent)
{
    _layout = new QVBoxLayout(this);
    _searchEdit = new LineEdit(this);
    _toolboxTree = new ToolboxTree(this);

    connect(_toolboxTree, &QTreeWidget::itemPressed, this, &ToolboxPane::handleMousePress);

    connect(_toolboxTree, &QTreeWidget::itemDoubleClicked, this, [=]
    {
        if (_toolboxTree->currentItem()
                && _toolboxTree->currentItem()->parent()) {
            emit itemDoubleClicked(
                        _toolboxTree->urls(_toolboxTree->currentItem()).first().toLocalFile());
        }
    });

    _searchEdit->addAction(QIcon(PaintUtils::renderOverlaidPixmap(":/images/search.svg", "#595959", _searchEdit)),
                           QLineEdit::LeadingPosition);
    _searchEdit->setPlaceholderText(tr("Search"));
    _searchEdit->setClearButtonEnabled(true);
    connect(_searchEdit, &LineEdit::textChanged, this, &ToolboxPane::filterList);

    _layout->addWidget(_searchEdit);
    _layout->addWidget(_toolboxTree);
    _layout->setSpacing(2);
    _layout->setContentsMargins(3, 3, 3, 3);

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

ToolboxTree* ToolboxPane::toolboxTree()
{
    return _toolboxTree;
}

void ToolboxPane::discharge()
{
    ToolManager::clear();
    _searchEdit->clear();
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
    return QSize{140, 710};
}
