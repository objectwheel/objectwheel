#include <toolboxpane.h>
#include <toolboxtree.h>
#include <lineedit.h>
#include <paintutils.h>
#include <saveutils.h>
#include <toolutils.h>

#include <qmljs/qmljsmodelmanagerinterface.h>

#include <QApplication>
#include <QVBoxLayout>
#include <QDir>

const char* TOOL_KEY = "QURBUEFaQVJMSVlJWiBIQUZJWg";

ToolboxPane::ToolboxPane(QWidget* parent) : QWidget(parent)
{
    _layout = new QVBoxLayout(this);
    _searchEdit = new LineEdit(this);
    _toolboxTree = new ToolboxTree(this);

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    _searchEdit->addAction(QIcon(PaintUtils::renderOverlaidPixmap(":/images/search.svg", "#595959", _searchEdit)),
                           QLineEdit::LeadingPosition);
    _searchEdit->setPlaceholderText(tr("Search"));
    _searchEdit->setClearButtonEnabled(true);
    connect(_searchEdit, &LineEdit::textChanged, this, &ToolboxPane::filterList);

    _layout->addWidget(_searchEdit);
    _layout->addWidget(_toolboxTree);
    _layout->setSpacing(2);
    _layout->setContentsMargins(3, 3, 3, 3);

    connect(QmlJS::ModelManagerInterface::instance(), &QmlJS::ModelManagerInterface::idle,
            this, &ToolboxPane::fillPane);
}

ToolboxTree* ToolboxPane::toolboxTree()
{
    return _toolboxTree;
}

void ToolboxPane::discharge()
{
    _toolboxTree->clearSelection();
    _toolboxTree->setCurrentItem(nullptr);
    _searchEdit->clear();
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

//    QMimeData* mimeData = new QMimeData;
//    mimeData->setUrls(_urls.value(item));
//    mimeData->setText(TOOL_KEY);

//    QDrag* drag = new QDrag(this);
//    drag->setMimeData(mimeData);
//    drag->setPixmap(item->icon(column).pixmap(iconSize()));

//    Qt::DropAction dropAction = drag->exec();

void ToolboxPane::fillPane()
{
    if (_toolboxTree->topLevelItemCount() > 0)
        return;

    for (const QString& toolDirName : QDir(":/tools").entryList(QDir::AllDirs | QDir::NoDotAndDotDot)) {
        const QString& toolPath = ":/tools/" + toolDirName;
        Q_ASSERT(SaveUtils::isControlValid(toolPath));
        const QString& name = ToolUtils::toolName(toolPath);
        const QString& category = ToolUtils::toolCetegory(toolPath);
        const QIcon& icon = QIcon(ToolUtils::toolIcon(toolPath, devicePixelRatioF()));
        _toolboxTree->addTool(name, category, toolPath, icon);
    }

    emit filled();
}
