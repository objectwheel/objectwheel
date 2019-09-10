#include <designerpane.h>
#include <designerview.h>
#include <anchoreditor.h>
#include <transparentstyle.h>

#include <QMenu>
#include <QToolBar>
#include <QVBoxLayout>
#include <QToolButton>
#include <QComboBox>

static QString symbol(const QToolButton* toolButton)
{
    const QKeySequence& seq = toolButton->shortcut();
    if (seq.isEmpty())
        return QStringLiteral("");
    return QStringLiteral(" (%1)").arg(seq.toString(QKeySequence::NativeText));
}

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
  , m_sceneSettingsButton(new QToolButton(this))
  , m_themeSettingsButton(new QToolButton(this))
  , m_zoomLevelComboBox(new QComboBox(this))
  , m_themeComboBox(new QComboBox(this))

  , m_invertSelectionAction(new QAction(this))
  , m_selectAllAction(new QAction(this))
  , m_sendBackAction(new QAction(this))
  , m_bringFrontAction(new QAction(this))
  , m_cutAction(new QAction(this))
  , m_copyAction(new QAction(this))
  , m_pasteAction(new QAction(this))
  , m_deleteAction(new QAction(this))
  , m_deleteAllAction(new QAction(this))
  , m_moveLeftAction(new QAction(this))
  , m_moveRightAction(new QAction(this))
  , m_moveUpAction(new QAction(this))
  , m_moveDownAction(new QAction(this))
{
    setFocusPolicy(Qt::NoFocus);
//    setContextMenuPolicy(Qt::CustomContextMenu);

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
    m_zoomLevelComboBox->setCursor(Qt::PointingHandCursor);
    m_sceneSettingsButton->setCursor(Qt::PointingHandCursor);
    m_themeComboBox->setCursor(Qt::PointingHandCursor);
    m_themeSettingsButton->setCursor(Qt::PointingHandCursor);

    m_refreshButton->setShortcut(Qt::Key_R);
    m_refreshButton->setToolTip(tr("Refresh control images") + symbol(m_refreshButton));
    m_clearButton->setToolTip(tr("Clear controls on the form"));
    m_anchorsButton->setToolTip(tr("Enable/Disable painting all the anchors"));
    m_snappingButton->setToolTip(tr("Enable/Disable snapping"));
    m_gridViewButton->setToolTip(tr("Enable/Disable grid view dots"));
    m_guidelinesButton->setToolTip(tr("Enable/Disable guidelines"));
    m_zoomLevelComboBox->setToolTip(tr("Change zoom level of the scene"));
    m_sceneSettingsButton->setToolTip(tr("Open scene settings"));
    m_themeComboBox->setToolTip(tr("Change project theme"));
    m_themeSettingsButton->setToolTip(tr("Open project theme settings"));

    m_refreshButton->setFixedSize(QSize(20, 20));
    m_clearButton->setFixedSize(QSize(20, 20));
    m_anchorsButton->setFixedSize(QSize(20, 20));
    m_snappingButton->setFixedSize(QSize(20, 20));
    m_gridViewButton->setFixedSize(QSize(20, 20));
    m_guidelinesButton->setFixedSize(QSize(20, 20));
    m_zoomLevelComboBox->setFixedHeight(20);
    m_sceneSettingsButton->setFixedSize(QSize(20, 20));
    m_themeComboBox->setFixedHeight(20);
    m_themeSettingsButton->setFixedSize(QSize(20, 20));

    m_refreshButton->setIconSize(QSize(16, 16));
    m_clearButton->setIconSize(QSize(16, 16));
    m_anchorsButton->setIconSize(QSize(16, 16));
    m_snappingButton->setIconSize(QSize(16, 16));
    m_gridViewButton->setIconSize(QSize(16, 16));
    m_guidelinesButton->setIconSize(QSize(16, 16));
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

    m_anchorsButton->setCheckable(true);
    m_snappingButton->setCheckable(true);
    m_gridViewButton->setCheckable(true);
    m_guidelinesButton->setCheckable(true);

    TransparentStyle::attach(m_toolBar);

    m_invertSelectionAction->setShortcutVisibleInContextMenu(true);
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

    m_invertSelectionAction->setText(tr("Invert Selection"));
    m_selectAllAction->setText(tr("Select All"));
    m_sendBackAction->setText(tr("Send to Back"));
    m_bringFrontAction->setText(tr("Bring to Front"));
    m_cutAction->setText(tr("Cut"));
    m_copyAction->setText(tr("Copy"));
    m_pasteAction->setText(tr("Paste"));
    m_deleteAction->setText(tr("Delete"));
    m_deleteAllAction->setText(tr("Delete All"));
    m_moveLeftAction->setText(tr("Move Left"));
    m_moveRightAction->setText(tr("Move Right"));
    m_moveUpAction->setText(tr("Move Up"));
    m_moveDownAction->setText(tr("Move Down"));

    m_invertSelectionAction->setIcon(QIcon(QStringLiteral(":/images/designer/invert-selection.svg")));
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

    addAction(m_invertSelectionAction);
    addAction(m_selectAllAction);
    addAction(m_sendBackAction);
    addAction(m_bringFrontAction);
    addAction(m_cutAction);
    addAction(m_copyAction);
    addAction(m_pasteAction);
    addAction(m_deleteAction);
    addAction(m_deleteAllAction);
    addAction(m_moveLeftAction);
    addAction(m_moveRightAction);
    addAction(m_moveUpAction);
    addAction(m_moveDownAction);

    m_menu->addAction(m_invertSelectionAction);
    m_menu->addAction(m_selectAllAction);
    m_menu->addAction(m_sendBackAction);
    m_menu->addAction(m_bringFrontAction);
    m_menu->addAction(m_cutAction);
    m_menu->addAction(m_copyAction);
    m_menu->addAction(m_pasteAction);
    m_menu->addAction(m_deleteAction);
    m_menu->addAction(m_deleteAllAction);
    m_menu->addAction(m_moveLeftAction);
    m_menu->addAction(m_moveRightAction);
    m_menu->addAction(m_moveUpAction);
    m_menu->addAction(m_moveDownAction);

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

QToolButton* DesignerPane::refreshButton() const
{
    return m_refreshButton;
}

QToolButton* DesignerPane::clearButton() const
{
    return m_clearButton;
}

QToolButton* DesignerPane::anchorsButton() const
{
    return m_anchorsButton;
}

QToolButton* DesignerPane::snappingButton() const
{
    return m_snappingButton;
}

QToolButton* DesignerPane::gridViewButton() const
{
    return m_gridViewButton;
}

QToolButton* DesignerPane::guidelinesButton() const
{
    return m_guidelinesButton;
}

QToolButton* DesignerPane::sceneSettingsButton() const
{
    return m_sceneSettingsButton;
}

QToolButton* DesignerPane::themeSettingsButton() const
{
    return m_themeSettingsButton;
}

QComboBox* DesignerPane::zoomLevelComboBox() const
{
    return m_zoomLevelComboBox;
}

QComboBox* DesignerPane::themeComboBox() const
{
    return m_themeComboBox;
}

QAction* DesignerPane::invertSelectionAction() const
{
    return m_invertSelectionAction;
}

QAction* DesignerPane::selectAllAction() const
{
    return m_selectAllAction;
}

QAction* DesignerPane::sendBackAction() const
{
    return m_sendBackAction;
}

QAction* DesignerPane::bringFrontAction() const
{
    return m_bringFrontAction;
}

QAction* DesignerPane::cutAction() const
{
    return m_cutAction;
}

QAction* DesignerPane::copyAction() const
{
    return m_copyAction;
}

QAction* DesignerPane::pasteAction() const
{
    return m_pasteAction;
}

QAction* DesignerPane::deleteAction() const
{
    return m_deleteAction;
}

QAction* DesignerPane::deleteAllAction() const
{
    return m_deleteAllAction;
}

QAction* DesignerPane::moveLeftAction() const
{
    return m_moveLeftAction;
}

QAction* DesignerPane::moveRightAction() const
{
    return m_moveRightAction;
}

QAction* DesignerPane::moveUpAction() const
{
    return m_moveUpAction;
}

QAction* DesignerPane::moveDownAction() const
{
    return m_moveDownAction;
}

QSize DesignerPane::sizeHint() const
{
    return {680, 680};
}

QSize DesignerPane::minimumSizeHint() const
{
    return {0, 0};
}
