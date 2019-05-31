#include <toolboxpane.h>
#include <toolboxtree.h>
#include <lineedit.h>
#include <mainwindow.h>
#include <paintutils.h>
#include <saveutils.h>

#include <QApplication>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <QDir>

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

    const QString& toolsDir = SaveUtils::toProjectToolsDir(ProjectManager::dir());

    for (const QString& toolDirName
         : QDir(":/tools").entryList(QDir::AllDirs | QDir::NoDotAndDotDot)) {
        const QString& toolPath = toolsDir + '/' + toolDirName;
        Q_ASSERT(SaveUtils::isControlValid(toolPath));

        QList<QUrl> urls;
        QString category = SaveUtils::controlToolCategory(toolPath);
        QString name = SaveUtils::controlToolName(toolPath);

        urls << QUrl::fromLocalFile(toolPath);

        if (category.isEmpty())
            category = QStringLiteral("Others");

        if (name.isEmpty())
            name = QStringLiteral("Tool");

        QTreeWidgetItem* topItem = _toolboxTree->categoryItem(category);
        if (!topItem) {
            topItem = new QTreeWidgetItem;
            topItem->setText(0, category);
            _toolboxTree->addTopLevelItem(topItem);
            topItem->setExpanded(true);
        }

        QPixmap icon = QPixmap::fromImage(QImage::fromData(SaveUtils::controlIcon(toolPath)));
        icon.setDevicePixelRatio(devicePixelRatioF());
        QTreeWidgetItem* item = new QTreeWidgetItem;
        item->setText(0, name);
        item->setIcon(0, QIcon(icon));
        topItem->addChild(item);
        _toolboxTree->addUrls(item, urls);
    }

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

ToolboxTree* ToolboxPane::toolboxTree()
{
    return _toolboxTree;
}

void ToolboxPane::discharge()
{
    _toolboxTree->clearSelection();
    _toolboxTree->setCurrentItem(nullptr);
    _toolboxTree->clear();
    _toolboxTree->clearUrls();
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
