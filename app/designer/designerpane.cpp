#include <designerpane.h>
#include <designerview.h>
#include <anchoreditor.h>
#include <transparentstyle.h>
#include <paintutils.h>
#include <designersettings.h>
#include <scenesettings.h>

#include <QMenu>
#include <QEvent>
#include <QToolBar>
#include <QVBoxLayout>
#include <QContextMenuEvent>
#include <QToolButton>
#include <QComboBox>

DesignerPane::DesignerPane(QWidget* parent) : QWidget(parent)
  , m_menu(new QMenu(this))
  , m_toolBar(new QToolBar(this))
  , m_designerView(new DesignerView(this))
  , m_anchorEditor(new AnchorEditor(m_designerView->scene(), this))

  , m_refreshButton(new QToolButton(this))
  , m_clearButton(new QToolButton(this))
  , m_anchorsButton(new QToolButton(this))
  , m_snappingButton(new QToolButton(this))
  , m_gridViewButton(new QToolButton(this))
  , m_guidelinesButton(new QToolButton(this))
  , m_controlOutlineButton(new QToolButton(this))
  , m_backgroundTextureButton(new QToolButton(this))
  , m_sceneSettingsButton(new QToolButton(this))
  , m_themeSettingsButton(new QToolButton(this))
  , m_zoomLevelComboBox(new QComboBox(this))
  , m_themeComboBox(new QComboBox(this))

  , m_toggleSelectionAction(m_menu->addAction(tr("Toggle Selection")))
  , m_selectAllAction(m_menu->addAction(tr("Select All")))
  , m_sendBackAction(m_menu->addAction(tr("Send to Back")))
  , m_bringFrontAction(m_menu->addAction(tr("Bring to Front")))
  , m_cutAction(m_menu->addAction(tr("Cut")))
  , m_copyAction(m_menu->addAction(tr("Copy")))
  , m_pasteAction(m_menu->addAction(tr("Paste")))
  , m_deleteAction(m_menu->addAction(tr("Delete")))
  , m_deleteAllAction(m_menu->addAction(tr("Delete All")))
  , m_moveLeftAction(m_menu->addAction(tr("Move Left")))
  , m_moveRightAction(m_menu->addAction(tr("Move Right")))
  , m_moveUpAction(m_menu->addAction(tr("Move Up")))
  , m_moveDownAction(m_menu->addAction(tr("Move Down")))
{
    setFocusPolicy(Qt::NoFocus);
    // setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred); default for pure widgets

    // Workaround for QToolBarLayout's obsolote serMargin function usage
    QMetaObject::invokeMethod(this, [=] {
        m_toolBar->setContentsMargins(0, 0, 0, 0);
        m_toolBar->layout()->setContentsMargins(0, 0, 0, 0); // They must be all same
        m_toolBar->layout()->setSpacing(1);
    }, Qt::QueuedConnection);

    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(m_toolBar);
    layout->addWidget(m_designerView);

    m_toolBar->setFixedHeight(24);
    m_toolBar->addWidget(m_refreshButton);
    m_toolBar->addWidget(m_clearButton);
    m_toolBar->addSeparator();
    m_toolBar->addWidget(m_anchorsButton);
    m_toolBar->addWidget(m_snappingButton);
    m_toolBar->addWidget(m_gridViewButton);
    m_toolBar->addWidget(m_guidelinesButton);
    m_toolBar->addSeparator();
    m_toolBar->addWidget(m_controlOutlineButton);
    m_toolBar->addWidget(m_backgroundTextureButton);
    m_toolBar->addWidget(m_zoomLevelComboBox);
    m_toolBar->addWidget(m_sceneSettingsButton);
    m_toolBar->addSeparator();
    m_toolBar->addWidget(m_themeComboBox);
    m_toolBar->addWidget(m_themeSettingsButton);

    m_refreshButton->setCursor(Qt::PointingHandCursor);
    m_clearButton->setCursor(Qt::PointingHandCursor);
    m_anchorsButton->setCursor(Qt::PointingHandCursor);
    m_snappingButton->setCursor(Qt::PointingHandCursor);
    m_gridViewButton->setCursor(Qt::PointingHandCursor);
    m_guidelinesButton->setCursor(Qt::PointingHandCursor);
    m_controlOutlineButton->setCursor(Qt::PointingHandCursor);
    m_backgroundTextureButton->setCursor(Qt::PointingHandCursor);
    m_zoomLevelComboBox->setCursor(Qt::PointingHandCursor);
    m_sceneSettingsButton->setCursor(Qt::PointingHandCursor);
    m_themeComboBox->setCursor(Qt::PointingHandCursor);
    m_themeSettingsButton->setCursor(Qt::PointingHandCursor);

    m_refreshButton->setToolTip(tr("Refresh Control Images"));
    m_clearButton->setToolTip(tr("Clear Controls on the Form"));
    m_anchorsButton->setToolTip(tr("Show/Hide Anchors"));
    m_snappingButton->setToolTip(tr("Enable/Disable Snapping"));
    m_gridViewButton->setToolTip(tr("Enable/Disable Grid View Dots"));
    m_guidelinesButton->setToolTip(tr("Enable/Disable Guidelines"));
    m_controlOutlineButton->setToolTip(tr("Change Control Outline Decoration"));
    m_backgroundTextureButton->setToolTip(tr("Change Scene Background Texture"));
    m_zoomLevelComboBox->setToolTip(tr("Change Zoom Level of the Scene"));
    m_sceneSettingsButton->setToolTip(tr("Open Scene Settings"));
    m_themeComboBox->setToolTip(tr("Change Project Theme"));
    m_themeSettingsButton->setToolTip(tr("Open Project Theme Settings"));

    m_refreshButton->setFixedSize(QSize(22, 22));
    m_clearButton->setFixedSize(QSize(22, 22));
    m_anchorsButton->setFixedSize(QSize(22, 22));
    m_snappingButton->setFixedSize(QSize(22, 22));
    m_gridViewButton->setFixedSize(QSize(22, 22));
    m_guidelinesButton->setFixedSize(QSize(22, 22));
    m_controlOutlineButton->setFixedSize(QSize(22, 22));
    m_backgroundTextureButton->setFixedSize(QSize(22, 22));
    m_zoomLevelComboBox->setFixedSize(QSize(22, 22));
    m_sceneSettingsButton->setFixedSize(QSize(22, 22));
    m_themeComboBox->setFixedSize(QSize(22, 22));
    m_themeSettingsButton->setFixedSize(QSize(22, 22));

    m_refreshButton->setIconSize(QSize(16, 16));
    m_clearButton->setIconSize(QSize(16, 16));
    m_anchorsButton->setIconSize(QSize(16, 16));
    m_snappingButton->setIconSize(QSize(16, 16));
    m_gridViewButton->setIconSize(QSize(16, 16));
    m_guidelinesButton->setIconSize(QSize(16, 16));
    m_controlOutlineButton->setIconSize(QSize(16, 16));
    m_backgroundTextureButton->setIconSize(QSize(16, 16));
    m_zoomLevelComboBox->setIconSize(QSize(16, 16));
    m_sceneSettingsButton->setIconSize(QSize(16, 16));
    m_themeComboBox->setIconSize(QSize(16, 16));
    m_themeSettingsButton->setIconSize(QSize(16, 16));

    m_refreshButton->setIcon(QIcon(QStringLiteral(":/images/designer/refresh.svg")));
    m_clearButton->setIcon(QIcon(QStringLiteral(":/images/designer/delete-all.svg")));
    m_anchorsButton->setIcon(QIcon(QStringLiteral(":/images/designer/anchors.svg")));
    m_snappingButton->setIcon(QIcon(QStringLiteral(":/images/designer/snapping.svg")));
    m_gridViewButton->setIcon(QIcon(QStringLiteral(":/images/designer/grid-view.svg")));
    m_guidelinesButton->setIcon(QIcon(QStringLiteral(":/images/designer/guidelines.svg")));
    m_sceneSettingsButton->setIcon(QIcon(QStringLiteral(":/images/designer/scene-settings.svg")));
    m_themeSettingsButton->setIcon(QIcon(QStringLiteral(":/images/designer/theme-settings.svg")));

    using namespace PaintUtils;
    const QPen pen(Qt::black, 2);
    const qreal dpr = m_backgroundTextureButton->devicePixelRatioF();
    m_backgroundTextureButton->addAction(new QAction(
    {renderPropertyColorPixmap(QSize(13, 13), QString(":/images/texture.svg"), pen, dpr)}, tr("Checkered")));
    m_backgroundTextureButton->addAction(new QAction(
    {renderPropertyColorPixmap(QSize(13, 13), Qt::black, pen, dpr)}, tr("Black")));
    m_backgroundTextureButton->addAction(new QAction(
    {renderPropertyColorPixmap(QSize(13, 13), Qt::darkGray, pen, dpr)}, tr("Dark gray")));
    m_backgroundTextureButton->addAction(new QAction(
    {renderPropertyColorPixmap(QSize(13, 13), Qt::lightGray, pen, dpr)}, tr("Light gray")));
    m_backgroundTextureButton->addAction(new QAction(
    {renderPropertyColorPixmap(QSize(13, 13), Qt::white, pen, dpr)},tr("White")));
    m_backgroundTextureButton->setIcon(m_backgroundTextureButton->actions().first()->icon());

    TransparentStyle::attach(m_toolBar);

    m_toggleSelectionAction->setShortcutVisibleInContextMenu(true);
    m_selectAllAction->setShortcutVisibleInContextMenu(true);
    m_sendBackAction->setShortcutVisibleInContextMenu(true);
    m_bringFrontAction->setShortcutVisibleInContextMenu(true);
    m_cutAction->setShortcutVisibleInContextMenu(true);
    m_copyAction->setShortcutVisibleInContextMenu(true);
    m_pasteAction->setShortcutVisibleInContextMenu(true);
    m_deleteAction->setShortcutVisibleInContextMenu(true);
    m_deleteAllAction->setShortcutVisibleInContextMenu(true);
    m_moveLeftAction->setShortcutVisibleInContextMenu(true);
    m_moveRightAction->setShortcutVisibleInContextMenu(true);
    m_moveUpAction->setShortcutVisibleInContextMenu(true);
    m_moveDownAction->setShortcutVisibleInContextMenu(true);

    m_toggleSelectionAction->setIcon(QIcon(QStringLiteral(":/images/designer/toggle-selection.svg")));
    m_selectAllAction->setIcon(QIcon(QStringLiteral(":/images/designer/select-all.svg")));
    m_sendBackAction->setIcon(QIcon(QStringLiteral(":/images/designer/send-to-back.svg")));
    m_bringFrontAction->setIcon(QIcon(QStringLiteral(":/images/designer/bring-to-front.svg")));
    m_cutAction->setIcon(QIcon(QStringLiteral(":/images/designer/cut.svg")));
    m_copyAction->setIcon(QIcon(QStringLiteral(":/images/designer/copy.svg")));
    m_pasteAction->setIcon(QIcon(QStringLiteral(":/images/designer/paste.svg")));
    m_deleteAction->setIcon(QIcon(QStringLiteral(":/images/designer/delete.svg")));
    m_deleteAllAction->setIcon(QIcon(QStringLiteral(":/images/designer/delete-all.svg")));
    m_moveLeftAction->setIcon(QIcon(QStringLiteral(":/images/designer/move-left.svg")));
    m_moveRightAction->setIcon(QIcon(QStringLiteral(":/images/designer/move-right.svg")));
    m_moveUpAction->setIcon(QIcon(QStringLiteral(":/images/designer/move-up.svg")));
    m_moveDownAction->setIcon(QIcon(QStringLiteral(":/images/designer/move-down.svg")));

    m_selectAllAction->setShortcut(QKeySequence::SelectAll);
    m_sendBackAction->setShortcut(Qt::CTRL + Qt::Key_Down);
    m_bringFrontAction->setShortcut(Qt::CTRL + Qt::Key_Up);
    m_cutAction->setShortcut(QKeySequence::Cut);
    m_copyAction->setShortcut(QKeySequence::Copy);
    m_pasteAction->setShortcut(QKeySequence::Paste);
#if defined(Q_OS_MACOS)
    m_deleteAction->setShortcuts(QList<QKeySequence>() << (Qt::CTRL + Qt::Key_Backspace) << QKeySequence::Delete);
#else
    m_deleteAction->setShortcut(QKeySequence::Delete);
#endif
    m_moveLeftAction->setShortcut(Qt::Key_Left);
    m_moveRightAction->setShortcut(Qt::Key_Right);
    m_moveUpAction->setShortcut(Qt::Key_Up);
    m_moveDownAction->setShortcut(Qt::Key_Down);

    m_menu->insertSeparator(m_sendBackAction);
    m_menu->insertSeparator(m_cutAction);
    m_menu->insertSeparator(m_moveLeftAction);
}

QMenu* DesignerPane::menu() const
{
    return m_menu;
}

QToolBar* DesignerPane::toolBar() const
{
    return m_toolBar;
}

DesignerView* DesignerPane::designerView() const
{
    return m_designerView;
}

AnchorEditor* DesignerPane::anchorEditor() const
{
    return m_anchorEditor;
}

QSize DesignerPane::sizeHint() const
{
    return {680, 680};
}

QSize DesignerPane::minimumSizeHint() const
{
    return {0, 0};
}

void DesignerPane::updateIcons()
{
    //    using namespace PaintUtils;
    //    m_runDevicesButton->setIcon(QIcon(":/images/devices.png"));
    //    m_runButton->setIcon(renderButtonIcon(":/images/run.svg", palette()));
    //    m_stopButton->setIcon(renderButtonIcon(":/images/stop.svg", palette()));
    //    m_preferencesButton->setIcon(renderButtonIcon(":/images/settings.svg", palette()));
    //    m_projectsButton->setIcon(renderButtonIcon(":/images/projects.svg", palette()));

    //    QStringList iconFileNames {
    //        ":/images/left.svg",
    //        ":/images/bottom.svg",
    //        ":/images/right.svg"
    //    };
    //    for (int i = 0; i < iconFileNames.size(); ++i) {
    //        QAction* action = m_segmentedBar->actions().at(i);
    //        action->setIcon(renderButtonIcon(iconFileNames.at(i), palette()));
    //    }
}

void DesignerPane::changeEvent(QEvent* event)
{
    if(event->type() == QEvent::ApplicationFontChange
            || event->type() == QEvent::PaletteChange) {
        updateIcons();
    }
    QWidget::changeEvent(event);
}

void DesignerPane::contextMenuEvent(QContextMenuEvent* event)
{
    event->accept();
    m_menu->popup(event->globalPos());
}
