#include <toolboxcontroller.h>
#include <toolboxpane.h>
#include <toolboxitem.h>
#include <toolboxtree.h>
#include <lineedit.h>
#include <saveutils.h>
#include <toolutils.h>
#include <documentmanager.h>
#include <utilityfunctions.h>
#include <controlpreviewingmanager.h>
#include <paintutils.h>

#include <QDir>
#include <QMimeData>
#include <QDrag>
#include <QTimer>

ToolboxController::ToolboxController(ToolboxPane* toolboxPane, QObject* parent) : QObject(parent)
  , m_toolboxPane(toolboxPane)
{
    connect(DocumentManager::instance(), &DocumentManager::projectInfoUpdated,
            this, &ToolboxController::onProjectInfoUpdate);
    connect(m_toolboxPane->toolboxTree(), &ToolboxTree::itemPressed,
            this, &ToolboxController::onToolboxItemPress);
    connect(m_toolboxPane->searchEdit(), &LineEdit::textEdited,
            this, &ToolboxController::onSearchTextEdit);
}

void ToolboxController::discharge()
{
    m_toolboxPane->searchEdit()->clear();
}

void ToolboxController::onProjectInfoUpdate()
{
    for (const QString& toolDirName : QDir(":/tools").entryList(QDir::AllDirs | QDir::NoDotAndDotDot)) {
        const QString& toolPath = ":/tools/" + toolDirName;
        Q_ASSERT(SaveUtils::isControlValid(toolPath));
        const QString& name = ToolUtils::toolName(toolPath);
        const QString& category = ToolUtils::toolCetegory(toolPath);
        const QIcon& icon = QIcon(ToolUtils::toolIcon(toolPath, m_toolboxPane->devicePixelRatioF()));
        m_toolboxPane->toolboxTree()->addTool(name, category, toolPath, icon);
    }
}

void ToolboxController::onToolboxItemPress(ToolboxItem* item)
{
    QMimeData* mimeData = new QMimeData;
    mimeData->setData(QStringLiteral("application/x-objectwheel-tool"),
                      UtilityFunctions::push(item->dir()));

    QPixmap pixmap(item->icon().pixmap(m_toolboxPane->toolboxTree()->iconSize()));
    pixmap.setDevicePixelRatio(m_toolboxPane->devicePixelRatioF());

    QPointer<QDrag> drag = new QDrag(this);
    drag->setMimeData(mimeData);
    drag->setPixmap(pixmap);
    drag->setHotSpot({1, 1});

    QPointer<QMetaObject::Connection> conn = new QMetaObject::Connection;
    *conn = connect(ControlPreviewingManager::instance(), &ControlPreviewingManager::individualPreviewDone,
                    [=] (const QImage& preview) {
        if (conn) {
            disconnect(*conn);
            delete conn;
        }
        if (drag && !PaintUtils::isBlankImage(preview)) {
            QPixmap pixmap(QPixmap::fromImage(preview));
            pixmap.setDevicePixelRatio(m_toolboxPane->devicePixelRatioF());
            drag->setPixmap(pixmap);
        }
    });

    QTimer::singleShot(150, [=] {
        if (conn) {
            disconnect(*conn);
            delete conn;
        }
        if (drag)
            drag->exec(Qt::CopyAction);
    });

    ControlPreviewingManager::scheduleIndividualPreview(SaveUtils::toControlMainQmlFile(item->dir()));
}

void ToolboxController::onSearchTextEdit(const QString& filter)
{
    for (int i = 0; i < m_toolboxPane->toolboxTree()->topLevelItemCount(); i++) {
        QTreeWidgetItem* tli = m_toolboxPane->toolboxTree()->topLevelItem(i);
        bool tlv = false;

        for (int j = 0; j < tli->childCount(); j++) {
            QTreeWidgetItem* tci = tli->child(j);
            bool v = filter.isEmpty() ? true : tci->text(0).contains(filter, Qt::CaseInsensitive);

            tci->setHidden(!v);
            if (v)
                tlv = v;
        }

        bool v = filter.isEmpty() ? true : tlv;
        tli->setHidden(!v);
    }
}
