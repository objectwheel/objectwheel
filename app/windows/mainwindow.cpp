#include <mainwindow.h>
#include <runpane.h>
#include <runcontroller.h>
#include <modeselectorpane.h>
#include <modeselectorcontroller.h>
#include <toolboxpane.h>
#include <toolboxcontroller.h>
#include <propertiespane.h>
#include <propertiescontroller.h>
#include <controlspane.h>
#include <controlscontroller.h>
#include <assetspane.h>
#include <formspane.h>
#include <centralwidget.h>
#include <designerview.h>
#include <controlcreationmanager.h>
#include <runmanager.h>
#include <projectmanager.h>
#include <consolewidget.h>
#include <form.h>
#include <qmlcodeeditorwidget.h>
#include <qmlcodedocument.h>
#include <qmlcodeeditor.h>
#include <utilityfunctions.h>
#include <outputbar.h>
#include <saveutils.h>
#include <welcomewindow.h>
#include <controlpropertymanager.h>
#include <preferenceswindow.h>
#include <generalsettings.h>
#include <interfacesettings.h>
#include <codeeditorsettings.h>
#include <behaviorsettings.h>
#include <segmentedbar.h>
#include <controlrenderingmanager.h>
#include <outputpane.h>
#include <outputcontroller.h>
#include <designerpane.h>
#include <designerscene.h>
#include <parserutils.h>
#include <designercontroller.h>
#include <pinbar.h>
#include <dockbar.h>

#include <QWindow>
#include <QProcess>
#include <QToolBar>
#include <QLabel>
#include <QToolButton>
#include <QDockWidget>
#include <QLayout>
#include <QScrollBar>
#include <QScreen>
#include <QTime>

#if defined(Q_OS_MACOS)
#include <windowoperations.h>
#endif

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent)
  , m_centralWidget(new CentralWidget)
  , m_runPane(new RunPane(this))
  , m_runController(new RunController(m_runPane, this))
  , m_modeSelectorPane(new ModeSelectorPane)
  , m_modeSelectorController(new ModeSelectorController(m_modeSelectorPane, this))
  , m_toolboxPane(new ToolboxPane)
  , m_toolboxController(new ToolboxController(m_toolboxPane, this))
  , m_propertiesPane(new PropertiesPane)
  , m_propertiesController(new PropertiesController(m_propertiesPane, m_centralWidget->designerPane()->designerView()->scene(), this))
  , m_controlsPane(new ControlsPane)
  , m_controlsController(new ControlsController(m_controlsPane, m_centralWidget->designerPane()->designerView()->scene(), this))
  , m_formsPane(new FormsPane(m_centralWidget->designerPane()->designerView()->scene()))
  , m_assetsPane(new AssetsPane)
  , m_controlsDockWidget(new QDockWidget(this))
  , m_propertiesDockWidget(new QDockWidget(this))
  , m_assetsDockWidget(new QDockWidget(this))
  , m_toolboxDockWidget(new QDockWidget(this))
  , m_formsDockWidget(new QDockWidget(this))
  , m_leftDockBar(new DockBar(this))
  , m_rightDockBar(new DockBar(this))
{
    setAnimated(false);
    setWindowTitle(APP_NAME);
    setAutoFillBackground(true);
    setCentralWidget(m_centralWidget);
    setContextMenuPolicy(Qt::NoContextMenu);
#if defined(Q_OS_MACOS)
    WindowOperations::removeTitleBar(this);
#endif

    QPalette palette(this->palette());
    palette.setColor(QPalette::Active, QPalette::Text, "#505050");
    palette.setColor(QPalette::Inactive, QPalette::Text, "#505050");
    palette.setColor(QPalette::Disabled, QPalette::Text, "#9c9c9c");
    palette.setColor(QPalette::Active, QPalette::WindowText, "#505050");
    palette.setColor(QPalette::Inactive, QPalette::WindowText, "#505050");
    palette.setColor(QPalette::Disabled, QPalette::WindowText, "#9c9c9c");
    palette.setColor(QPalette::Active, QPalette::ButtonText, "#505050");
    palette.setColor(QPalette::Inactive, QPalette::ButtonText, "#505050");
    palette.setColor(QPalette::Disabled, QPalette::ButtonText, "#9c9c9c");

    /** Setup Tool Bars **/
    /* Add Run Pane */
    m_runPane->setMovable(false);
    m_runPane->setFloatable(false);
    m_runPane->setPalette(palette);
    m_runPane->setOrientation(Qt::Horizontal);
    addToolBar(Qt::TopToolBarArea, m_runPane);
    addToolBarBreak(Qt::TopToolBarArea);

    /* Add Mode Selector Pane */
    m_modeSelectorPane->setMovable(false);
    m_modeSelectorPane->setFloatable(false);
    m_modeSelectorPane->setPalette(palette);
    m_modeSelectorPane->setOrientation(Qt::Horizontal);
    addToolBar(Qt::TopToolBarArea, m_modeSelectorPane);

    /* Add Left Dock Bar */
    m_leftDockBar->setMovable(false);
    m_leftDockBar->setFloatable(false);
    m_leftDockBar->setPalette(palette);
    m_leftDockBar->setOrientation(Qt::Vertical);
    addToolBar(Qt::LeftToolBarArea, m_leftDockBar);

    /* Add Right Dock Bar */
    m_rightDockBar->setMovable(false);
    m_rightDockBar->setFloatable(false);
    m_rightDockBar->setPalette(palette);
    m_rightDockBar->setOrientation(Qt::Vertical);
    addToolBar(Qt::RightToolBarArea, m_rightDockBar);

    /** Setup Dock Widgets **/
    /* Add Controls Pane */
    auto controlsPinBar = new PinBar(m_controlsDockWidget);
    controlsPinBar->setPalette(palette);
    controlsPinBar->setTitle(tr("Controls"));
    controlsPinBar->setIcon(QIcon(QStringLiteral(":/images/settings/controls.svg")));
    m_controlsDockWidget->setObjectName("controlsDockWidget");
    m_controlsDockWidget->setTitleBarWidget(controlsPinBar);
    m_controlsDockWidget->setWidget(m_controlsPane);
    m_controlsDockWidget->setWindowTitle(tr("Controls"));
    m_controlsDockWidget->setFeatures(QDockWidget::AllDockWidgetFeatures);
    m_controlsDockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    addDockWidget(Qt::RightDockWidgetArea, m_controlsDockWidget);
    m_rightDockBar->addDockWidget(m_controlsDockWidget);

    /* Add Properties Pane */
    auto propertiesPinBar = new PinBar(m_propertiesDockWidget);
    propertiesPinBar->setPalette(palette);
    propertiesPinBar->setTitle(tr("Properties"));
    propertiesPinBar->setIcon(QIcon(QStringLiteral(":/images/designer/properties.svg")));
    m_propertiesDockWidget->setObjectName("propertiesDockWidget");
    m_propertiesDockWidget->setTitleBarWidget(propertiesPinBar);
    m_propertiesDockWidget->setWidget(m_propertiesPane);
    m_propertiesDockWidget->setWindowTitle(tr("Properties"));
    m_propertiesDockWidget->setFeatures(QDockWidget::AllDockWidgetFeatures);
    m_propertiesDockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    addDockWidget(Qt::RightDockWidgetArea, m_propertiesDockWidget);
    m_rightDockBar->addDockWidget(m_propertiesDockWidget);

    /* Add Assets Pane */
    auto assetsPinBar = new PinBar(m_assetsDockWidget);
    assetsPinBar->setPalette(palette);
    assetsPinBar->setTitle(tr("Assets"));
    assetsPinBar->setIcon(QIcon(QStringLiteral(":/images/designer/assets.svg")));
    m_assetsDockWidget->setObjectName("assetsDockWidget");
    m_assetsDockWidget->setTitleBarWidget(assetsPinBar);
    m_assetsDockWidget->setWidget(m_assetsPane);
    m_assetsDockWidget->setWindowTitle(tr("Assets"));
    m_assetsDockWidget->setFeatures(QDockWidget::AllDockWidgetFeatures);
    m_assetsDockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    addDockWidget(Qt::RightDockWidgetArea, m_assetsDockWidget);
    m_rightDockBar->addDockWidget(m_assetsDockWidget);

    /* Add Toolbox Pane */
    auto toolboxPinBar = new PinBar(m_toolboxDockWidget);
    toolboxPinBar->setPalette(palette);
    toolboxPinBar->setTitle(tr("Toolbox"));
    toolboxPinBar->setIcon(QIcon(QStringLiteral(":/images/settings/toolbox.svg")));
    m_toolboxDockWidget->setObjectName("toolboxDockWidget");
    m_toolboxDockWidget->setTitleBarWidget(toolboxPinBar);
    m_toolboxDockWidget->setWidget(m_toolboxPane);
    m_toolboxDockWidget->setWindowTitle(tr("Toolbox"));
    m_toolboxDockWidget->setFeatures(QDockWidget::AllDockWidgetFeatures);
    m_toolboxDockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    addDockWidget(Qt::LeftDockWidgetArea, m_toolboxDockWidget);
    m_leftDockBar->addDockWidget(m_toolboxDockWidget);

    /* Add Forms Pane */
    auto formsPinBar = new PinBar(m_formsDockWidget);
    formsPinBar->setPalette(palette);
    formsPinBar->setTitle(tr("Forms"));
    formsPinBar->setIcon(QIcon(QStringLiteral(":/images/designer/forms.svg")));
    m_formsDockWidget->setObjectName("formsDockWidget");
    m_formsDockWidget->setTitleBarWidget(formsPinBar);
    m_formsDockWidget->setWidget(m_formsPane);
    m_formsDockWidget->setWindowTitle(tr("Forms"));
    m_formsDockWidget->setFeatures(QDockWidget::AllDockWidgetFeatures);
    m_formsDockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    addDockWidget(Qt::LeftDockWidgetArea, m_formsDockWidget);
    m_leftDockBar->addDockWidget(m_formsDockWidget);

    connect(m_assetsPane, &AssetsPane::fileOpened,
            centralWidget()->qmlCodeEditorWidget(), &QmlCodeEditorWidget::openAssets);
    connect(ModeManager::instance(), &ModeManager::modeChanged,
            this, [=] (ModeManager::Mode mode) {
        if (mode == ModeManager::Designer || mode == ModeManager::Split)
            m_runPane->segmentedBar()->setEnabled(true);
        else
            m_runPane->segmentedBar()->setEnabled(false);
    });
    connect(ModeManager::instance(), &ModeManager::modeChanged,
            this, &MainWindow::onModeChange);
    connect(m_controlsController, &ControlsController::controlSelectionChanged,
            [=] (const QList<Control*>& selectedControls) {
        m_centralWidget->designerPane()->designerView()->scene()->clearSelection();
        for (Control* control : selectedControls)
            control->setSelected(true);
    });
    connect(m_controlsController, &ControlsController::goToSlotActionTriggered,
            m_centralWidget->designerController(), &DesignerController::onGoToSlotActionTrigger);
    connect(m_controlsController, &ControlsController::editAnchorsActionTriggered,
            m_centralWidget->designerController(), &DesignerController::onEditAnchorsActionTrigger);
    connect(m_controlsController, &ControlsController::viewSourceCodeActionTriggered,
            m_centralWidget->designerController(), &DesignerController::onViewSourceCodeActionTrigger);
    connect(m_centralWidget->qmlCodeEditorWidget(), &QmlCodeEditorWidget::opened,
            [=] {
        if (m_centralWidget->qmlCodeEditorWidget()->count() <= 0
                && ModeManager::mode() != ModeManager::Split) {
            ModeManager::setMode(ModeManager::Designer);
        }
        if (m_centralWidget->qmlCodeEditorWidget()->count() > 0
                && ModeManager::mode() != ModeManager::Editor) {
            ModeManager::setMode(ModeManager::Split);
        }
    });
    connect(m_runController, &RunController::ran, this, [=] {
        const QString& timestamp = QTime::currentTime().toString();
        BehaviorSettings* settings = CodeEditorSettings::behaviorSettings();
        if (settings->autoSaveBeforeRunning)
            m_centralWidget->qmlCodeEditorWidget()->saveAll();
        m_centralWidget->outputPane()->consoleWidget()->fade();
        if (!m_centralWidget->outputPane()->consoleWidget()->toPlainText().isEmpty())
            m_centralWidget->outputPane()->consoleWidget()->press("\n");
        m_centralWidget->outputPane()->consoleWidget()->press(
                    timestamp + tr(": Starting") + " " + ProjectManager::name() + "...\n",
                    QColor("#025dbf"), QFont::DemiBold);
        m_centralWidget->outputPane()->consoleWidget()->verticalScrollBar()->
                setValue(m_centralWidget->outputPane()->consoleWidget()->verticalScrollBar()->maximum());
    });
    connect(m_centralWidget->outputController(), &OutputController::currentWidgetChanged,
            [=] (bool visible) {
        QAction* bottomAction = m_runPane->segmentedBar()->actions().at(1);
        bottomAction->setChecked(visible);
    });
    connect(m_runController, &RunController::segmentedBarActionTriggered,
            [=] (int index, bool checked) {
        if (index == 0) {
            showLeftPanes(checked);
        } else if (index == 1) {
            m_centralWidget->outputController()->setPaneVisible(checked);
        } else {
            showRightPanes(checked);
        }
    });
    connect(ProjectManager::instance(), &ProjectManager::started,
            this, [=] { m_assetsPane->setRootPath(SaveUtils::toProjectAssetsDir(ProjectManager::dir())); });
    connect(ControlPropertyManager::instance(), &ControlPropertyManager::idChanged,
            m_formsPane, &FormsPane::refresh);
    connect(GeneralSettings::instance(), &GeneralSettings::designerStateReset,
            this, &MainWindow::resetSettings);

    connect(windowHandle(), &QWindow::screenChanged,
            this, &MainWindow::onScreenChange);

    discharge();
    //resetState = saveState();
}

void MainWindow::charge()
{
    m_centralWidget->charge();
}

void MainWindow::discharge()
{
    m_runController->discharge();
    m_modeSelectorController->discharge();
    m_toolboxController->discharge();
    m_propertiesController->discharge();
    m_controlsController->discharge();
    m_centralWidget->discharge();
    m_formsPane->discharge();
    m_assetsPane->discharge();

    showLeftPanes(true);
    showRightPanes(true);
}

void MainWindow::showLeftPanes(bool show)
{
    if (dockWidgetArea(m_assetsDockWidget) == Qt::LeftDockWidgetArea) {
        m_assetsDockWidget->setVisible(show);
        m_assetsDockWidgetVisible = show;
    }
    if (dockWidgetArea(m_propertiesDockWidget) == Qt::LeftDockWidgetArea) {
        m_propertiesDockWidget->setVisible(show);
        m_propertiesDockWidgetVisible = show;
    }
    if (dockWidgetArea(m_formsDockWidget) == Qt::LeftDockWidgetArea) {
        m_formsDockWidget->setVisible(show);
        m_formsDockWidgetVisible = show;
    }
    if (dockWidgetArea(m_controlsDockWidget) == Qt::LeftDockWidgetArea) {
        m_controlsDockWidget->setVisible(show);
        m_controlsDockWidgetVisible = show;
    }
    if (dockWidgetArea(m_toolboxDockWidget) == Qt::LeftDockWidgetArea) {
        m_toolboxDockWidget->setVisible(show);
        m_toolboxDockWidgetVisible = show;
    }
}

void MainWindow::showRightPanes(bool show)
{
    if (dockWidgetArea(m_assetsDockWidget) == Qt::RightDockWidgetArea) {
        m_assetsDockWidget->setVisible(show);
        m_assetsDockWidgetVisible = show;
    }
    if (dockWidgetArea(m_propertiesDockWidget) == Qt::RightDockWidgetArea) {
        m_propertiesDockWidget->setVisible(show);
        m_propertiesDockWidgetVisible = show;
    }
    if (dockWidgetArea(m_formsDockWidget) == Qt::RightDockWidgetArea) {
        m_formsDockWidget->setVisible(show);
        m_formsDockWidgetVisible = show;
    }
    if (dockWidgetArea(m_controlsDockWidget) == Qt::RightDockWidgetArea) {
        m_controlsDockWidget->setVisible(show);
        m_controlsDockWidgetVisible = show;
    }
    if (dockWidgetArea(m_toolboxDockWidget) == Qt::RightDockWidgetArea) {
        m_toolboxDockWidget->setVisible(show);
        m_toolboxDockWidgetVisible = show;
    }
}

void MainWindow::hideDocks()
{
    m_assetsDockWidget->hide();
    m_propertiesDockWidget->hide();
    m_formsDockWidget->hide();
    m_controlsDockWidget->hide();
    m_toolboxDockWidget->hide();
}

void MainWindow::showDocks()
{
    m_assetsDockWidget->show();
    m_propertiesDockWidget->show();
    m_formsDockWidget->show();
    m_controlsDockWidget->show();
    m_toolboxDockWidget->show();
}

void MainWindow::restoreDocks()
{
    m_assetsDockWidget->setVisible(m_assetsDockWidgetVisible);
    m_propertiesDockWidget->setVisible(m_propertiesDockWidgetVisible);
    m_formsDockWidget->setVisible(m_formsDockWidgetVisible);
    m_controlsDockWidget->setVisible(m_controlsDockWidgetVisible);
    m_toolboxDockWidget->setVisible(m_toolboxDockWidgetVisible);
}

void MainWindow::onModeChange(ModeManager::Mode mode)
{
    switch (mode) {
    case ModeManager::Builds:
    case ModeManager::Documents:
    case ModeManager::Editor:
    case ModeManager::Options:
        hideDocks();
        break;
    case ModeManager::Designer:
    case ModeManager::Split:
        restoreDocks();
        break;
    default:
        break;
    }
}

void MainWindow::onScreenChange(QScreen* screen)
{
    // TODO: Do not unnecessarily update the dpr if it is not changed
    ControlRenderingManager::scheduleDevicePixelRatioUpdate(screen->devicePixelRatio());
}

CentralWidget* MainWindow::centralWidget() const
{
    return m_centralWidget;
}

AssetsPane* MainWindow::assetsPane() const
{
    return m_assetsPane;
}

ToolboxPane* MainWindow::toolboxPane() const
{
    return m_toolboxPane;
}

PropertiesPane* MainWindow::propertiesPane() const
{
    return m_propertiesPane;
}

ControlsPane* MainWindow::controlsPane() const
{
    return m_controlsPane;
}

void MainWindow::resetSettings()
{
    InterfaceSettings* settings = GeneralSettings::interfaceSettings();
    settings->begin();
    settings->setValue("MainWindow.Size", sizeHint());
    settings->setValue("MainWindow.Position", UtilityFunctions::centerPos(sizeHint()));
    settings->setValue("MainWindow.Maximized", false);
    settings->setValue("MainWindow.Fullscreen", false);
    settings->setValue("MainWindow.WindowState", m_resetState);
    settings->end();

    if (isVisible())
        readSettings();
}

void MainWindow::readSettings()
{
    InterfaceSettings* settings = GeneralSettings::interfaceSettings();
    settings->begin();
    resize(settings->value<QSize>("MainWindow.Size", sizeHint()));
    move(settings->value<QPoint>("MainWindow.Position", UtilityFunctions::centerPos(size())));
    if (settings->value<bool>("MainWindow.Fullscreen", false))
        showFullScreen();
    else if (settings->value<bool>("MainWindow.Maximized", false))
        showMaximized();
    else
        showNormal();
    restoreState(settings->value<QByteArray>("MainWindow.WindowState", {}));
    settings->end();
}

void MainWindow::writeSettings()
{
    InterfaceSettings* settings = GeneralSettings::interfaceSettings();
    settings->begin();
    settings->setValue("MainWindow.Size", size());
    settings->setValue("MainWindow.Position", pos());
    settings->setValue("MainWindow.Maximized", isMaximized());
    settings->setValue("MainWindow.Fullscreen", isFullScreen());
    settings->setValue("MainWindow.WindowState", saveState());
    settings->end();
}

void MainWindow::showEvent(QShowEvent* event)
{
    static bool firstShown = true;
    if (firstShown)
        readSettings();
    firstShown = false;
    event->accept();
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    //if (GeneralSettings::interfaceSettings()->preserveDesignerState)
    //    writeSettings();
    QMainWindow::closeEvent(event);
}

QSize MainWindow::sizeHint() const
{
    return {1220, 720};
}
