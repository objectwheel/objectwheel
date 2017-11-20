#include <toolbox.h>
#include <toolsmanager.h>
#include <zipper.h>
#include <filemanager.h>
#include <savemanager.h>

#include <QApplication>
#include <QTreeView>

#define DURATION 500



ToolBox::ToolBox(QWidget* parent)
    : QWidget(parent)
    , _settingsDialog(this)
{
    setAutoFillBackground(true);
    QPalette p(palette());
    p.setColor(QPalette::Window, QColor("#E0E4E7"));
    setPalette(p);

    _toolboxTree.setIndicatorButtonVisible(true);
    _toolboxTree.indicatorButton()->setIcon(QIcon(":/resources/images/right-arrow.png"));
    _toolboxTree.indicatorButton()->setColor(QColor("#0D74C8"));
    _toolboxTree.indicatorButton()->setRadius(fit::fx(7));
    _toolboxTree.indicatorButton()->setIconSize(QSize(fit::fx(10), fit::fx(10)));
    _toolboxTree.indicatorButton()->setFixedSize(fit::fx(15), fit::fx(15));
    connect(&_toolboxTree, &QTreeWidget::itemPressed, this, &ToolBox::handleMousePress);

    _searchEdit.setPlaceholderText("Filter");
    _searchEdit.setClearButtonEnabled(true);
    connect(&_searchEdit, SIGNAL(textChanged(QString)), SLOT(filterList(QString)));

    _toolboxVLay.addWidget(&_searchEdit);
    _toolboxVLay.addWidget(&_toolboxTree);
    _toolboxVLay.setSpacing(fit::fx(2));
    _toolboxVLay.setContentsMargins(fit::fx(3), fit::fx(3), fit::fx(3), fit::fx(3));

    setLayout(&_toolboxVLay);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

ToolboxTree* ToolBox::toolboxTree()
{
    return &_toolboxTree;
}

void ToolBox::handleMousePress(QTreeWidgetItem* item)
{
    if (item == 0)
        return;

    if (QApplication::mouseButtons() != Qt::LeftButton)
        return;

    if (item->parent() == 0) {
        _toolboxTree.setItemExpanded(item,
          !_toolboxTree.isItemExpanded(item));
        return;
    }
}

void ToolBox::filterList(const QString& filter)
{
    for (int i = 0; i < _toolboxTree.topLevelItemCount(); i++) {
        auto tli = _toolboxTree.topLevelItem(i);
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

void ToolBox::showSettings()
{
    _settingsDialog.exec();
}

QSize ToolBox::sizeHint() const
{
    return QSize(fit::fx(200), fit::fx(3000));
}
