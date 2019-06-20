#include <toolboxcontroller.h>
#include <toolboxpane.h>
#include <toolboxitem.h>
#include <toolboxtree.h>
#include <lineedit.h>
#include <saveutils.h>
#include <toolutils.h>

#include <qmljs/qmljsmodelmanagerinterface.h>

#include <QDir>

const char* TOOL_KEY = "QURBUEFaQVJMSVlJWiBIQUZJWg";

ToolboxController::ToolboxController(ToolboxPane* toolboxPane, QObject* parent) : QObject(parent)
  , m_toolboxPane(toolboxPane)
{
    connect(m_toolboxPane->searchEdit(), &LineEdit::textEdited,
            this, &ToolboxController::onSearchTextEdit);
    connect(QmlJS::ModelManagerInterface::instance(), &QmlJS::ModelManagerInterface::idle,
            this, &ToolboxController::fillPane);
}

void ToolboxController::discharge()
{
    m_toolboxPane->searchEdit()->clear();
}

void ToolboxController::onToolboxPress(ToolboxItem* item)
{
    //    QMimeData* mimeData = new QMimeData;
    //    mimeData->setUrls(_urls.value(item));
    //    mimeData->setText(TOOL_KEY);

    //    QDrag* drag = new QDrag(this);
    //    drag->setMimeData(mimeData);
    //    drag->setPixmap(item->icon(column).pixmap(iconSize()));

    //    Qt::DropAction dropAction = drag->exec();

}

void ToolboxController::onSearchTextEdit(const QString& filter)
{
    for (int i = 0; i < m_toolboxPane->toolboxTree()->topLevelItemCount(); i++) {
        auto tli = m_toolboxPane->toolboxTree()->topLevelItem(i);
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

void ToolboxController::fillPane()
{
    if (m_toolboxPane->toolboxTree()->topLevelItemCount() > 0)
        return;

    for (const QString& toolDirName : QDir(":/tools").entryList(QDir::AllDirs | QDir::NoDotAndDotDot)) {
        const QString& toolPath = ":/tools/" + toolDirName;
        Q_ASSERT(SaveUtils::isControlValid(toolPath));
        const QString& name = ToolUtils::toolName(toolPath);
        const QString& category = ToolUtils::toolCetegory(toolPath);
        const QIcon& icon = QIcon(ToolUtils::toolIcon(toolPath, m_toolboxPane->devicePixelRatioF()));
        m_toolboxPane->toolboxTree()->addTool(name, category, toolPath, icon);
    }

    emit filled();
}
