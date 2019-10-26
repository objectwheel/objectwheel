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
#include <QLabel>
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
  , m_assetsDockWidgetVisible(true)
  , m_propertiesDockWidgetVisible(true)
  , m_formsDockWidgetVisible(true)
  , m_toolboxDockWidgetVisible(true)
  , m_controlsDockWidgetVisible(true)
{
    setWindowTitle(APP_NAME);
    setAutoFillBackground(true);
    setCentralWidget(m_centralWidget);
    setContextMenuPolicy(Qt::NoContextMenu);
#if defined(Q_OS_MACOS)
    WindowOperations::removeTitleBar(this);
#endif

    /** Setup Tool Bars **/
    /* Add Run Pane */
    m_runPane->setMovable(false);
    m_runPane->setFloatable(false);
    m_runPane->setOrientation(Qt::Horizontal);
    addToolBar(Qt::TopToolBarArea, m_runPane);
    addToolBarBreak(Qt::TopToolBarArea);

    /* Add Mode Selector Pane */
    m_modeSelectorPane->setMovable(false);
    m_modeSelectorPane->setFloatable(false);
    m_modeSelectorPane->setOrientation(Qt::Horizontal);
    addToolBar(Qt::TopToolBarArea, m_modeSelectorPane);

    /* Add Left Dock Bar */
    m_leftDockBar->setMovable(false);
    m_leftDockBar->setFloatable(false);
    m_leftDockBar->setOrientation(Qt::Vertical);
    addToolBar(Qt::LeftToolBarArea, m_leftDockBar);

    /* Add Right Dock Bar */
    m_rightDockBar->setMovable(false);
    m_rightDockBar->setFloatable(false);
    m_rightDockBar->setOrientation(Qt::Vertical);
    addToolBar(Qt::RightToolBarArea, m_rightDockBar);

    /** Setup Dock Widgets **/
    /* Add Controls Pane */
    m_controlsDockWidget->setObjectName("controlsDockWidget");
    m_controlsDockWidget->setWidget(m_controlsPane);
    m_controlsDockWidget->setWindowTitle(tr("Controls"));
    m_controlsDockWidget->setWindowIcon(QIcon(QStringLiteral(":/images/settings/controls.svg")));
    m_controlsDockWidget->setFeatures(QDockWidget::AllDockWidgetFeatures);
    m_controlsDockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    m_controlsDockWidget->setTitleBarWidget(new PinBar(m_controlsDockWidget));
    addDockWidget(Qt::RightDockWidgetArea, m_controlsDockWidget);

    /* Add Properties Pane */
    m_propertiesDockWidget->setObjectName("propertiesDockWidget");
    m_propertiesDockWidget->setWidget(m_propertiesPane);
    m_propertiesDockWidget->setWindowTitle(tr("Properties"));
    m_propertiesDockWidget->setWindowIcon(QIcon(QStringLiteral(":/images/settings/properties.svg")));
    m_propertiesDockWidget->setFeatures(QDockWidget::AllDockWidgetFeatures);
    m_propertiesDockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    m_propertiesDockWidget->setTitleBarWidget(new PinBar(m_propertiesDockWidget));
    addDockWidget(Qt::RightDockWidgetArea, m_propertiesDockWidget);

    /* Add Assets Pane */
    m_assetsDockWidget->setObjectName("assetsDockWidget");
    m_assetsDockWidget->setWidget(m_assetsPane);
    m_assetsDockWidget->setWindowTitle(tr("Assets"));
    m_assetsDockWidget->setWindowIcon(QIcon(QStringLiteral(":/images/settings/assets.svg")));
    m_assetsDockWidget->setFeatures(QDockWidget::AllDockWidgetFeatures);
    m_assetsDockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    m_assetsDockWidget->setTitleBarWidget(new PinBar(m_assetsDockWidget));
    addDockWidget(Qt::RightDockWidgetArea, m_assetsDockWidget);

    /* Add Toolbox Pane */
    m_toolboxDockWidget->setObjectName("toolboxDockWidget");
    m_toolboxDockWidget->setWidget(m_toolboxPane);
    m_toolboxDockWidget->setWindowTitle(tr("Toolbox"));
    m_toolboxDockWidget->setWindowIcon(QIcon(QStringLiteral(":/images/settings/toolbox.svg")));
    m_toolboxDockWidget->setFeatures(QDockWidget::AllDockWidgetFeatures);
    m_toolboxDockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    m_toolboxDockWidget->setTitleBarWidget(new PinBar(m_toolboxDockWidget));
    addDockWidget(Qt::LeftDockWidgetArea, m_toolboxDockWidget);

    /* Add Forms Pane */
    m_formsDockWidget->setObjectName("formsDockWidget");
    m_formsDockWidget->setWidget(m_formsPane);
    m_formsDockWidget->setWindowTitle(tr("Forms"));
    m_formsDockWidget->setWindowIcon(QIcon(QStringLiteral(":/images/settings/forms.svg")));
    m_formsDockWidget->setFeatures(QDockWidget::AllDockWidgetFeatures);
    m_formsDockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    m_formsDockWidget->setTitleBarWidget(new PinBar(m_formsDockWidget));
    addDockWidget(Qt::LeftDockWidgetArea, m_formsDockWidget);

    auto updatePalette = [=] {
        QPalette p(palette());
        p.setColor(QPalette::Active, QPalette::Text, "#505050");
        p.setColor(QPalette::Inactive, QPalette::Text, "#505050");
        p.setColor(QPalette::Disabled, QPalette::Text, "#9c9c9c");
        p.setColor(QPalette::Active, QPalette::WindowText, "#505050");
        p.setColor(QPalette::Inactive, QPalette::WindowText, "#505050");
        p.setColor(QPalette::Disabled, QPalette::WindowText, "#9c9c9c");
        p.setColor(QPalette::Active, QPalette::ButtonText, "#505050");
        p.setColor(QPalette::Inactive, QPalette::ButtonText, "#505050");
        p.setColor(QPalette::Disabled, QPalette::ButtonText, "#9c9c9c");
        qApp->setPalette(p, "RunPane");
        qApp->setPalette(p, "ModeSelectorPane");
        qApp->setPalette(p, "PinBar");
        qApp->setPalette(p, "DockBar");
        qApp->setPalette(p, "OutputPane");
        qApp->setPalette(p, "AnchorRow");
    };
    connect(qApp, &QApplication::paletteChanged, this, updatePalette);
    updatePalette();

    connect(m_leftDockBar, &DockBar::dockWidgetShown,
            this, &MainWindow::onDockBarDockWidgetShown);
    connect(m_rightDockBar, &DockBar::dockWidgetShown,
            this, &MainWindow::onDockBarDockWidgetShown);
    connect((PinBar*) m_controlsDockWidget->titleBarWidget(), &PinBar::dockWidgetHid,
            this, &MainWindow::onPinBarDockWidgetHid);
    connect((PinBar*) m_propertiesDockWidget->titleBarWidget(), &PinBar::dockWidgetHid,
            this, &MainWindow::onPinBarDockWidgetHid);
    connect((PinBar*) m_assetsDockWidget->titleBarWidget(), &PinBar::dockWidgetHid,
            this, &MainWindow::onPinBarDockWidgetHid);
    connect((PinBar*) m_toolboxDockWidget->titleBarWidget(), &PinBar::dockWidgetHid,
            this, &MainWindow::onPinBarDockWidgetHid);
    connect((PinBar*) m_formsDockWidget->titleBarWidget(), &PinBar::dockWidgetHid,
            this, &MainWindow::onPinBarDockWidgetHid);

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
    m_rightDockBar->setEnabled(true);
}

void MainWindow::onModeChange(ModeManager::Mode mode)
{
    switch (mode) {
    case ModeManager::Builds:
    case ModeManager::Documents:
    case ModeManager::Editor:
    case ModeManager::Options:
        m_assetsDockWidget->hide();
        m_propertiesDockWidget->hide();
        m_formsDockWidget->hide();
        m_controlsDockWidget->hide();
        m_toolboxDockWidget->hide();
        m_leftDockBar->setEnabled(false);
        m_rightDockBar->setEnabled(false);
        break;
    case ModeManager::Designer:
    case ModeManager::Split:
        m_assetsDockWidget->setVisible(m_assetsDockWidgetVisible);
        m_propertiesDockWidget->setVisible(m_propertiesDockWidgetVisible);
        m_formsDockWidget->setVisible(m_formsDockWidgetVisible);
        m_controlsDockWidget->setVisible(m_controlsDockWidgetVisible);
        m_toolboxDockWidget->setVisible(m_toolboxDockWidgetVisible);
        m_leftDockBar->setEnabled(true);
        m_rightDockBar->setEnabled(true);
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

void MainWindow::onPinBarDockWidgetHid(QDockWidget* dockWidget)
{
    if (dockWidget == m_assetsDockWidget)
        m_assetsDockWidgetVisible = false;
    else if (dockWidget == m_propertiesDockWidget)
        m_propertiesDockWidgetVisible = false;
    else if (dockWidget == m_formsDockWidget)
        m_formsDockWidgetVisible = false;
    else if (dockWidget == m_controlsDockWidget)
        m_controlsDockWidgetVisible = false;
    else if (dockWidget == m_toolboxDockWidget)
        m_toolboxDockWidgetVisible = false;
    if (dockWidgetArea(dockWidget) == Qt::LeftDockWidgetArea)
        m_leftDockBar->addDockWidget(dockWidget);
    else
        m_rightDockBar->addDockWidget(dockWidget);
}

void MainWindow::onDockBarDockWidgetShown(QDockWidget* dockWidget)
{
    if (dockWidget == m_assetsDockWidget)
        m_assetsDockWidgetVisible = true;
    else if (dockWidget == m_propertiesDockWidget)
        m_propertiesDockWidgetVisible = true;
    else if (dockWidget == m_formsDockWidget)
        m_formsDockWidgetVisible = true;
    else if (dockWidget == m_controlsDockWidget)
        m_controlsDockWidgetVisible = true;
    else if (dockWidget == m_toolboxDockWidget)
        m_toolboxDockWidgetVisible = true;
    m_leftDockBar->removeDockWidget(dockWidget);
    m_rightDockBar->removeDockWidget(dockWidget);
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
