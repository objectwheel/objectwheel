#include <toolboxcontroller.h>
#include <toolboxpane.h>
#include <toolboxitem.h>
#include <toolboxtree.h>
#include <lineedit.h>
#include <saveutils.h>
#include <documentmanager.h>
#include <utilityfunctions.h>
#include <controlrenderingmanager.h>
#include <paintutils.h>
#include <designersettings.h>
#include <scenesettings.h>
#include <toolboxsettings.h>
#include <generalsettings.h>
#include <interfacesettings.h>

#include <QDir>
#include <QMimeData>
#include <QDrag>
#include <QTimer>

InitInfo ToolboxController::s_toolboxInitInfo;

ToolboxController::ToolboxController(ToolboxPane* toolboxPane, QObject* parent) : QObject(parent)
  , m_toolboxPane(toolboxPane)
{
    onToolboxSettingsChange();
    connect(DocumentManager::instance(), &DocumentManager::initialized,
            this, &ToolboxController::onDocumentManagerInitialization);
    connect(m_toolboxPane->toolboxTree(), &ToolboxTree::itemPressed,
            this, &ToolboxController::onToolboxItemPress);
    connect(m_toolboxPane->searchEdit(), &LineEdit::textEdited,
            this, &ToolboxController::onSearchEditEdit);
    connect(DesignerSettings::instance(), &DesignerSettings::toolboxSettingsChanged,
            this, &ToolboxController::onToolboxSettingsChange);
}

InitInfo ToolboxController::toolboxInitInfo()
{
    return s_toolboxInitInfo;
}

void ToolboxController::discharge()
{
    m_toolboxPane->searchEdit()->clear();
}

void ToolboxController::onToolboxSettingsChange()
{
    ToolboxTree* tree = m_toolboxPane->toolboxTree();
    const ToolboxSettings* settings = DesignerSettings::toolboxSettings();
    tree->setAlternatingRowColors(settings->enableAlternatingRowColors);
    tree->setTextElideMode(Qt::TextElideMode(settings->textElideMode));
    tree->setIconSize(QSize(settings->iconSize, settings->iconSize));
}

void ToolboxController::onDocumentManagerInitialization()
{
    Q_ASSERT(s_toolboxInitInfo.forms.isEmpty());
    ToolboxTree* toolboxTree = m_toolboxPane->toolboxTree();
    for (const QString& toolDirName : QDir(":/tools").entryList(QDir::AllDirs | QDir::NoDotAndDotDot)) {
        const QString& toolPath = ":/tools/" + toolDirName;
        Q_ASSERT(SaveUtils::isControlValid(toolPath));
        ToolboxItem* item = toolboxTree->addTool(toolPath);
        s_toolboxInitInfo.forms.append(QPair<QString, QString>(item->dir(), item->module()));
    }
    toolboxTree->sortByColumn(0, Qt::AscendingOrder); // Make the lower index to be at top
}

void ToolboxController::onSearchEditEdit(const QString& searchTerm)
{
    for (int i = 0; i < m_toolboxPane->toolboxTree()->topLevelItemCount(); ++i) {
        bool categoryItemVisible = false;
        QTreeWidgetItem* categoryItem = m_toolboxPane->toolboxTree()->topLevelItem(i);

        for (int j = 0; j < categoryItem->childCount(); ++j) {
            QTreeWidgetItem* toolboxItem = categoryItem->child(j);
            bool itemVisible = searchTerm.isEmpty()
                    ? true : toolboxItem->text(0).contains(searchTerm, Qt::CaseInsensitive);

            if (toolboxItem->isHidden() == itemVisible)
                toolboxItem->setHidden(!itemVisible);
            if (itemVisible)
                categoryItemVisible = true;
        }

        if (categoryItem->isHidden() == categoryItemVisible)
            categoryItem->setHidden(!categoryItemVisible);
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
                    [=] (RenderInfo info) {
        if (locked) {
            disconnect(*conn);
            delete conn;
            QDrag* drag = establishDrag(item);
            if (!PaintUtils::isBlankImage(info.image)) {
                info.image.setDevicePixelRatio(m_toolboxPane->devicePixelRatio()); // QDataStream cannot write dpr
                drag->setPixmap(UtilityFunctions::imageToPixmap(info.image));
            } else if (info.gui && info.visible && PaintUtils::isBlankImage(info.image)) {
                drag->setPixmap(UtilityFunctions::imageToPixmap(PaintUtils::renderBlankControlImage(
                    info.surroundingRect,
                    item->text(0),
                    m_toolboxPane->devicePixelRatioF(),
                    DesignerSettings::sceneSettings()->toBlankControlDecorationBrush(Qt::darkGray),
                    GeneralSettings::interfaceSettings()->highlightColor)));
            }
            info.image = drag->pixmap().toImage();
            drag->mimeData()->setData(QStringLiteral("application/x-objectwheel-module"),
                                      UtilityFunctions::push(item->module()));
            drag->mimeData()->setData(QStringLiteral("application/x-objectwheel-render-info"),
                                      UtilityFunctions::push(info));
            locked = false;
            drag->exec(Qt::CopyAction);
            processLocked = false;
        }
    });

    QTimer::singleShot(80, [=] {
        if (locked) {
            disconnect(*conn);
            delete conn;
            locked = false;
            establishDrag(item)->exec(Qt::CopyAction);
            processLocked = false;
        }
    });

    ControlRenderingManager::schedulePreview(SaveUtils::toControlMainQmlFile(item->dir()), item->module());
}

QDrag* ToolboxController::establishDrag(ToolboxItem* item)
{
    QMimeData* mimeData = new QMimeData;
    mimeData->setData(QStringLiteral("application/x-objectwheel-tool"),
                      UtilityFunctions::push(item->dir()));

    QPixmap pixmap(PaintUtils::pixmap(item->icon(), m_toolboxPane->toolboxTree()->iconSize(), m_toolboxPane));
    QPointer<QDrag> drag = new QDrag(this);
    drag->setMimeData(mimeData);
    drag->setPixmap(pixmap);
    drag->setHotSpot({5, 5});
    return drag;
}
