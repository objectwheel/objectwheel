#include <toolboxcontroller.h>
#include <toolboxpane.h>
#include <toolboxitem.h>
#include <toolboxtree.h>
#include <lineedit.h>
#include <saveutils.h>
#include <toolutils.h>
#include <documentmanager.h>
#include <utilityfunctions.h>
#include <controlrenderingmanager.h>
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
    connect(m_toolboxPane->searchEdit(), &LineEdit::textChanged,
            this, &ToolboxController::onSearchTextChange);
}

void ToolboxController::discharge()
{
    m_toolboxPane->searchEdit()->clear();
}

void ToolboxController::onProjectInfoUpdate()
{
    DocumentManager::instance()->disconnect(this);
    for (const QString& toolDirName : QDir(":/tools").entryList(QDir::AllDirs | QDir::NoDotAndDotDot)) {
        const QString& toolPath = ":/tools/" + toolDirName;
        Q_ASSERT(SaveUtils::isControlValid(toolPath));
        m_toolboxPane->toolboxTree()->addTool(
                    ToolUtils::toolName(toolPath),
                    ToolUtils::toolCetegory(toolPath), toolPath,
                    QIcon(ToolUtils::toolIconPath(toolPath)));
    }
}

void ToolboxController::onToolboxItemPress(ToolboxItem* item)
{
    static bool locked = false;
    static bool processLocked = false;

    if (locked)
        return;

    if (processLocked)
        return;

    locked = true;
    processLocked = true;

    auto conn = new QMetaObject::Connection;
    *conn = connect(ControlRenderingManager::instance(), &ControlRenderingManager::previewDone,
                    [=] (const RenderResult& result) {
        if (locked) {
            disconnect(*conn);
            delete conn;
            QDrag* drag = establishDrag(item);
            if (!PaintUtils::isBlankImage(result.image)) {
                QPixmap pixmap(QPixmap::fromImage(result.image));
                pixmap.setDevicePixelRatio(ControlRenderingManager::devicePixelRatio());
                drag->setPixmap(pixmap);
            } else if (result.gui && result.visible && PaintUtils::isBlankImage(result.image)) {
                drag->setPixmap(QPixmap::fromImage(PaintUtils::renderBlankControlImage(
                                    result.boundingRect, result.boundingRect,
                                    m_toolboxPane->toolboxTree()->currentItem()->text(0),
                                    m_toolboxPane->devicePixelRatioF())));
            }
            drag->mimeData()->setData(QStringLiteral("application/x-objectwheel-render-result"),
                                      UtilityFunctions::push(result));
            locked = false;
            drag->exec(Qt::CopyAction);
            processLocked = false;
        }
    });

    QTimer::singleShot(150, [=] {
        if (locked) {
            disconnect(*conn);
            delete conn;
            locked = false;
            establishDrag(item)->exec(Qt::CopyAction);
            processLocked = false;
        }
    });

    ControlRenderingManager::schedulePreview(SaveUtils::toControlMainQmlFile(item->dir()));
}

void ToolboxController::onSearchTextChange(const QString& text)
{
    for (int i = 0; i < m_toolboxPane->toolboxTree()->topLevelItemCount(); ++i) {
        bool categoryItemVisible = false;
        QTreeWidgetItem* categoryItem = m_toolboxPane->toolboxTree()->topLevelItem(i);

        for (int j = 0; j < categoryItem->childCount(); ++j) {
            QTreeWidgetItem* toolboxItem = categoryItem->child(j);
            bool itemVisible = text.isEmpty()
                    ? true : toolboxItem->text(0).contains(text, Qt::CaseInsensitive);

            if (toolboxItem->isHidden() == itemVisible)
                toolboxItem->setHidden(!itemVisible);
            if (itemVisible)
                categoryItemVisible = true;
        }

        if (categoryItem->isHidden() == categoryItemVisible)
            categoryItem->setHidden(!categoryItemVisible);
    }
}

QDrag* ToolboxController::establishDrag(ToolboxItem* item)
{
    QMimeData* mimeData = new QMimeData;
    mimeData->setData(QStringLiteral("application/x-objectwheel-tool"),
                      UtilityFunctions::push(item->dir()));

    QPixmap pixmap(item->icon().pixmap(UtilityFunctions::window(m_toolboxPane), m_toolboxPane->toolboxTree()->iconSize()));
    pixmap.setDevicePixelRatio(m_toolboxPane->devicePixelRatioF());

    QPointer<QDrag> drag = new QDrag(this);
    drag->setMimeData(mimeData);
    drag->setPixmap(pixmap);
    drag->setHotSpot({5, 5});
    return drag;
}
