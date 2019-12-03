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
#include <formspane.h>
#include <formscontroller.h>
#include <assetspane.h>
#include <assetstree.h>
#include <assetscontroller.h>
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

#include <QComboBox>
#include <QPushButton>
#include <QWindow>
#include <QProcess>
#include <QLabel>
#include <QDockWidget>
#include <QLayout>
#include <QScrollBar>
#include <QScreen>
#include <QTime>
#include <QTimer>

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
  , m_formsPane(new FormsPane)
  , m_formsController(new FormsController(m_formsPane, m_centralWidget->designerPane()->designerView()->scene(), this))
  , m_assetsPane(new AssetsPane)
  , m_assetsController(new AssetsController(m_assetsPane))
  , m_controlsDockWidget(new QDockWidget(this))
  , m_propertiesDockWidget(new QDockWidget(this))
  , m_assetsDockWidget(new QDockWidget(this))
  , m_toolboxDockWidget(new QDockWidget(this))
  , m_formsDockWidget(new QDockWidget(this))
  , m_leftDockBar(new DockBar(this))
  , m_rightDockBar(new DockBar(this))
  , m_removeSizeRestrictionsOnDockWidgetsTimer(new QTimer(this))
  , m_shownForTheFirstTime(false)
  , m_assetsDockWidgetVisible(true)
  , m_propertiesDockWidgetVisible(true)
  , m_formsDockWidgetVisible(true)
  , m_toolboxDockWidgetVisible(true)
  , m_controlsDockWidgetVisible(true)
{
    setWindowTitle(APP_NAME + QStringLiteral(" (Beta)"));
    setAutoFillBackground(true);
    setCentralWidget(m_centralWidget);
    setContextMenuPolicy(Qt::NoContextMenu);
#if defined(Q_OS_MACOS)
    WindowOperations::removeTitleBar(this);
#endif

    /** Setup Tool Bars **/
    /* Add Run Pane */
    m_runPane->setObjectName("runPane");
    m_runPane->setMovable(false);
    m_runPane->setFloatable(false);
    m_runPane->setOrientation(Qt::Horizontal);
    addToolBar(Qt::TopToolBarArea, m_runPane);
    addToolBarBreak(Qt::TopToolBarArea);

    /* Add Mode Selector Pane */
    m_modeSelectorPane->setObjectName("modeSelectorPane");
    m_modeSelectorPane->setMovable(false);
    m_modeSelectorPane->setFloatable(false);
    m_modeSelectorPane->setOrientation(Qt::Horizontal);
    addToolBar(Qt::TopToolBarArea, m_modeSelectorPane);

    /* Add Left Dock Bar */
    m_leftDockBar->setObjectName("leftDockBar");
    m_leftDockBar->setMovable(false);
    m_leftDockBar->setFloatable(false);
    m_leftDockBar->setOrientation(Qt::Vertical);
    addToolBar(Qt::LeftToolBarArea, m_leftDockBar);

    /* Add Right Dock Bar */
    m_rightDockBar->setObjectName("rightDockBar");
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
    m_rightDockBar->addDockWidget(m_controlsDockWidget);

    /* Add Properties Pane */
    m_propertiesDockWidget->setObjectName("propertiesDockWidget");
    m_propertiesDockWidget->setWidget(m_propertiesPane);
    m_propertiesDockWidget->setWindowTitle(tr("Properties"));
    m_propertiesDockWidget->setWindowIcon(QIcon(QStringLiteral(":/images/settings/properties.svg")));
    m_propertiesDockWidget->setFeatures(QDockWidget::AllDockWidgetFeatures);
    m_propertiesDockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    m_propertiesDockWidget->setTitleBarWidget(new PinBar(m_propertiesDockWidget));
    addDockWidget(Qt::RightDockWidgetArea, m_propertiesDockWidget);
    m_rightDockBar->addDockWidget(m_propertiesDockWidget);

    /* Add Assets Pane */
    m_assetsDockWidget->setObjectName("assetsDockWidget");
    m_assetsDockWidget->setWidget(m_assetsPane);
    m_assetsDockWidget->setWindowTitle(tr("Assets"));
    m_assetsDockWidget->setWindowIcon(QIcon(QStringLiteral(":/images/settings/assets.svg")));
    m_assetsDockWidget->setFeatures(QDockWidget::AllDockWidgetFeatures);
    m_assetsDockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    m_assetsDockWidget->setTitleBarWidget(new PinBar(m_assetsDockWidget));
    addDockWidget(Qt::RightDockWidgetArea, m_assetsDockWidget);
    m_rightDockBar->addDockWidget(m_assetsDockWidget);

    /* Add Toolbox Pane */
    m_toolboxDockWidget->setObjectName("toolboxDockWidget");
    m_toolboxDockWidget->setWidget(m_toolboxPane);
    m_toolboxDockWidget->setWindowTitle(tr("Toolbox"));
    m_toolboxDockWidget->setWindowIcon(QIcon(QStringLiteral(":/images/settings/toolbox.svg")));
    m_toolboxDockWidget->setFeatures(QDockWidget::AllDockWidgetFeatures);
    m_toolboxDockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    m_toolboxDockWidget->setTitleBarWidget(new PinBar(m_toolboxDockWidget));
    addDockWidget(Qt::LeftDockWidgetArea, m_toolboxDockWidget);
    m_leftDockBar->addDockWidget(m_toolboxDockWidget);

    /* Add Forms Pane */
    m_formsDockWidget->setObjectName("formsDockWidget");
    m_formsDockWidget->setWidget(m_formsPane);
    m_formsDockWidget->setWindowTitle(tr("Forms"));
    m_formsDockWidget->setWindowIcon(QIcon(QStringLiteral(":/images/settings/forms.svg")));
    m_formsDockWidget->setFeatures(QDockWidget::AllDockWidgetFeatures);
    m_formsDockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    m_formsDockWidget->setTitleBarWidget(new PinBar(m_formsDockWidget));
    addDockWidget(Qt::LeftDockWidgetArea, m_formsDockWidget);
    m_leftDockBar->addDockWidget(m_formsDockWidget);

    static_cast<PinBar*>(m_assetsDockWidget->titleBarWidget())->addWidget(m_assetsPane->modeComboBox());
    static_cast<PinBar*>(m_assetsDockWidget->titleBarWidget())->addSeparator();
    static_cast<PinBar*>(m_formsDockWidget->titleBarWidget())->addWidget(m_formsPane->addButton());
    static_cast<PinBar*>(m_formsDockWidget->titleBarWidget())->addWidget(m_formsPane->removeButton());
    static_cast<PinBar*>(m_formsDockWidget->titleBarWidget())->addSeparator();

    m_removeSizeRestrictionsOnDockWidgetsTimer->setSingleShot(true);
    m_removeSizeRestrictionsOnDockWidgetsTimer->setInterval(1000);

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

    connect(m_controlsDockWidget, &QDockWidget::dockLocationChanged,
            this, &MainWindow::onDockWidgetLocationChange);
    connect(m_propertiesDockWidget, &QDockWidget::dockLocationChanged,
            this, &MainWindow::onDockWidgetLocationChange);
    connect(m_assetsDockWidget, &QDockWidget::dockLocationChanged,
            this, &MainWindow::onDockWidgetLocationChange);
    connect(m_toolboxDockWidget, &QDockWidget::dockLocationChanged,
            this, &MainWindow::onDockWidgetLocationChange);
    connect(m_formsDockWidget, &QDockWidget::dockLocationChanged,
            this, &MainWindow::onDockWidgetLocationChange);
    connect(m_leftDockBar, &DockBar::dockWidgetButtonClicked,
            this, &MainWindow::onDockBarDockWidgetButtonClick);
    connect(m_rightDockBar, &DockBar::dockWidgetButtonClicked,
            this, &MainWindow::onDockBarDockWidgetButtonClick);
    connect((PinBar*) m_controlsDockWidget->titleBarWidget(), &PinBar::dockWidgetCloseButtonClicked,
            this, &MainWindow::onPinBarDockWidgetCloseButtonClick);
    connect((PinBar*) m_propertiesDockWidget->titleBarWidget(), &PinBar::dockWidgetCloseButtonClicked,
            this, &MainWindow::onPinBarDockWidgetCloseButtonClick);
    connect((PinBar*) m_assetsDockWidget->titleBarWidget(), &PinBar::dockWidgetCloseButtonClicked,
            this, &MainWindow::onPinBarDockWidgetCloseButtonClick);
    connect((PinBar*) m_toolboxDockWidget->titleBarWidget(), &PinBar::dockWidgetCloseButtonClicked,
            this, &MainWindow::onPinBarDockWidgetCloseButtonClick);
    connect((PinBar*) m_formsDockWidget->titleBarWidget(), &PinBar::dockWidgetCloseButtonClicked,
            this, &MainWindow::onPinBarDockWidgetCloseButtonClick);
    connect(m_assetsPane->assetsTree(), &AssetsTree::fileOpened,
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
        if (index == 0)
            setDockWidgetAreasVisible(Qt::LeftDockWidgetArea, checked);
        else if (index == 1)
            m_centralWidget->outputController()->setPaneVisible(checked);
        else
            setDockWidgetAreasVisible(Qt::RightDockWidgetArea, checked);
    });
    connect(GeneralSettings::instance(), &GeneralSettings::designerStateReset,
            this, [=] {
        resetWidget();
        QTimer::singleShot(1000, this, [=] {
            resetWidget();
            writeSettings();
        });
    });
    connect(windowHandle(), &QWindow::screenChanged,
            this, &MainWindow::onScreenChange);
    connect(m_removeSizeRestrictionsOnDockWidgetsTimer, &QTimer::timeout,
            this, &MainWindow::onRemoveSizeRestrictionsOnDockWidgetsTimerTimeout);

    resetWidget();
    grab(); // Ensures everything laid out http://stackoverflow.com/q/2427103
    m_initialInterfaceState = saveState();
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
    m_runController->charge();
    m_controlsController->charge();
    m_formsController->charge();
    m_assetsController->charge();
    m_centralWidget->charge();
}

void MainWindow::discharge()
{
    m_runController->discharge();
    m_modeSelectorController->discharge();
    m_toolboxController->discharge();
    m_propertiesController->discharge();
    m_controlsController->discharge();
    m_formsController->discharge();
    m_assetsController->discharge();
    m_centralWidget->discharge();
}

void MainWindow::setDockWidgetAreasVisible(Qt::DockWidgetAreas areas, bool visible)
{
    if (dockWidgetArea(m_toolboxDockWidget) & areas) {
        if (m_toolboxDockWidget->isVisible())
            m_toolboxDockWidget->setFixedSize(m_toolboxDockWidget->size());
        m_toolboxDockWidget->setVisible(visible && m_toolboxDockWidgetVisible);
    }
    if (dockWidgetArea(m_formsDockWidget) & areas) {
        if (m_formsDockWidget->isVisible())
            m_formsDockWidget->setFixedSize(m_formsDockWidget->size());
        m_formsDockWidget->setVisible(visible && m_formsDockWidgetVisible);
    }
    if (dockWidgetArea(m_controlsDockWidget) & areas) {
        if (m_controlsDockWidget->isVisible())
            m_controlsDockWidget->setFixedSize(m_controlsDockWidget->size());
        m_controlsDockWidget->setVisible(visible && m_controlsDockWidgetVisible);
    }
    if (dockWidgetArea(m_propertiesDockWidget) & areas) {
        if (m_propertiesDockWidget->isVisible())
            m_propertiesDockWidget->setFixedSize(m_propertiesDockWidget->size());
        m_propertiesDockWidget->setVisible(visible && m_propertiesDockWidgetVisible);
    }
    if (dockWidgetArea(m_assetsDockWidget) & areas) {
        if (m_assetsDockWidget->isVisible())
            m_assetsDockWidget->setFixedSize(m_assetsDockWidget->size());
        m_assetsDockWidget->setVisible(visible && m_assetsDockWidgetVisible);
    }
    if (areas & Qt::LeftDockWidgetArea)
        m_leftDockBar->setVisible(visible);
    if (areas & Qt::RightDockWidgetArea)
        m_rightDockBar->setVisible(visible);
    m_removeSizeRestrictionsOnDockWidgetsTimer->start();
}

void MainWindow::onModeChange(ModeManager::Mode mode)
{
    switch (mode) {
    case ModeManager::Builds:
    case ModeManager::Documents:
    case ModeManager::Editor:
    case ModeManager::Options:
        setDockWidgetAreasVisible(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea, false);
        break;
    case ModeManager::Designer:
    case ModeManager::Split:
        setDockWidgetAreasVisible(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea, true);
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

void MainWindow::onDockWidgetLocationChange(Qt::DockWidgetArea area)
{
    auto dockWidget = static_cast<QDockWidget*>(sender());
    if (area == Qt::LeftDockWidgetArea && m_leftDockBar->buttonExists(dockWidget))
        return;
    if (area == Qt::RightDockWidgetArea && m_rightDockBar->buttonExists(dockWidget))
        return;
    m_leftDockBar->removeDockWidget(dockWidget);
    m_rightDockBar->removeDockWidget(dockWidget);
    if (area == Qt::LeftDockWidgetArea) {
        m_leftDockBar->addDockWidget(dockWidget);
        m_leftDockBar->setDockWidgetButtonChecked(dockWidget, !dockWidget->isVisible());
    } else if (area == Qt::RightDockWidgetArea) {
        m_rightDockBar->addDockWidget(dockWidget);
        m_rightDockBar->setDockWidgetButtonChecked(dockWidget, !dockWidget->isVisible());
    }
}

void MainWindow::onPinBarDockWidgetCloseButtonClick(QDockWidget* dockWidget)
{
    onDockBarDockWidgetButtonClick(dockWidget, true);
    m_leftDockBar->setDockWidgetButtonChecked(dockWidget, true);
    m_rightDockBar->setDockWidgetButtonChecked(dockWidget, true);
}

void MainWindow::onDockBarDockWidgetButtonClick(QDockWidget* dockWidget, bool checked)
{
    if (dockWidget == m_assetsDockWidget)
        m_assetsDockWidgetVisible = !checked;
    else if (dockWidget == m_propertiesDockWidget)
        m_propertiesDockWidgetVisible = !checked;
    else if (dockWidget == m_formsDockWidget)
        m_formsDockWidgetVisible = !checked;
    else if (dockWidget == m_controlsDockWidget)
        m_controlsDockWidgetVisible = !checked;
    else if (dockWidget == m_toolboxDockWidget)
        m_toolboxDockWidgetVisible = !checked;
    dockWidget->setVisible(!checked);
}

void MainWindow::onRemoveSizeRestrictionsOnDockWidgetsTimerTimeout()
{
    if (m_toolboxDockWidget->isVisible())
        m_toolboxDockWidget->setFixedSize(QSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX));
    if (m_formsDockWidget->isVisible())
        m_formsDockWidget->setFixedSize(QSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX));
    if (m_controlsDockWidget->isVisible())
        m_controlsDockWidget->setFixedSize(QSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX));
    if (m_propertiesDockWidget->isVisible())
        m_propertiesDockWidget->setFixedSize(QSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX));
    if (m_assetsDockWidget->isVisible())
        m_assetsDockWidget->setFixedSize(QSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX));
}

void MainWindow::showEvent(QShowEvent* event)
{
    QMainWindow::showEvent(event);

    if (m_shownForTheFirstTime != true) {
        const_cast<bool&>(m_shownForTheFirstTime) = true;

        readSettings();

        m_controlsDockWidgetVisible = m_controlsDockWidget->isVisible();
        m_propertiesDockWidgetVisible = m_propertiesDockWidget->isVisible();
        m_assetsDockWidgetVisible = m_assetsDockWidget->isVisible();
        m_toolboxDockWidgetVisible = m_toolboxDockWidget->isVisible();
        m_formsDockWidgetVisible = m_formsDockWidget->isVisible();

        bool leftSideVisible = m_runPane->segmentedBar()->actions().at(0)->isChecked();
        bool rightSideVisible = m_runPane->segmentedBar()->actions().at(2)->isChecked();
        bool bottomSideVisible = m_runPane->segmentedBar()->actions().at(1)->isChecked();

        setDockWidgetAreasVisible(Qt::LeftDockWidgetArea, leftSideVisible);
        setDockWidgetAreasVisible(Qt::RightDockWidgetArea, rightSideVisible);
        m_centralWidget->outputController()->setPaneVisible(bottomSideVisible);

        m_leftDockBar->removeDockWidget(m_controlsDockWidget);
        m_leftDockBar->removeDockWidget(m_propertiesDockWidget);
        m_leftDockBar->removeDockWidget(m_assetsDockWidget);
        m_leftDockBar->removeDockWidget(m_toolboxDockWidget);
        m_leftDockBar->removeDockWidget(m_formsDockWidget);

        m_rightDockBar->removeDockWidget(m_controlsDockWidget);
        m_rightDockBar->removeDockWidget(m_propertiesDockWidget);
        m_rightDockBar->removeDockWidget(m_assetsDockWidget);
        m_rightDockBar->removeDockWidget(m_toolboxDockWidget);
        m_rightDockBar->removeDockWidget(m_formsDockWidget);

        if (dockWidgetArea(m_controlsDockWidget) & Qt::LeftDockWidgetArea)
            m_leftDockBar->addDockWidget(m_controlsDockWidget);
        else
            m_rightDockBar->addDockWidget(m_controlsDockWidget);
        if (dockWidgetArea(m_propertiesDockWidget) & Qt::LeftDockWidgetArea)
            m_leftDockBar->addDockWidget(m_propertiesDockWidget);
        else
            m_rightDockBar->addDockWidget(m_propertiesDockWidget);
        if (dockWidgetArea(m_assetsDockWidget) & Qt::LeftDockWidgetArea)
            m_leftDockBar->addDockWidget(m_assetsDockWidget);
        else
            m_rightDockBar->addDockWidget(m_assetsDockWidget);
        if (dockWidgetArea(m_toolboxDockWidget) & Qt::LeftDockWidgetArea)
            m_leftDockBar->addDockWidget(m_toolboxDockWidget);
        else
            m_rightDockBar->addDockWidget(m_toolboxDockWidget);
        if (dockWidgetArea(m_formsDockWidget) & Qt::LeftDockWidgetArea)
            m_leftDockBar->addDockWidget(m_formsDockWidget);
        else
            m_rightDockBar->addDockWidget(m_formsDockWidget);

        m_leftDockBar->setDockWidgetButtonChecked(m_controlsDockWidget, !m_controlsDockWidgetVisible);
        m_leftDockBar->setDockWidgetButtonChecked(m_propertiesDockWidget, !m_propertiesDockWidgetVisible);
        m_leftDockBar->setDockWidgetButtonChecked(m_assetsDockWidget, !m_assetsDockWidgetVisible);
        m_leftDockBar->setDockWidgetButtonChecked(m_toolboxDockWidget, !m_toolboxDockWidgetVisible);
        m_leftDockBar->setDockWidgetButtonChecked(m_formsDockWidget, !m_formsDockWidgetVisible);

        m_rightDockBar->setDockWidgetButtonChecked(m_controlsDockWidget, !m_controlsDockWidgetVisible);
        m_rightDockBar->setDockWidgetButtonChecked(m_propertiesDockWidget, !m_propertiesDockWidgetVisible);
        m_rightDockBar->setDockWidgetButtonChecked(m_assetsDockWidget, !m_assetsDockWidgetVisible);
        m_rightDockBar->setDockWidgetButtonChecked(m_toolboxDockWidget, !m_toolboxDockWidgetVisible);
        m_rightDockBar->setDockWidgetButtonChecked(m_formsDockWidget, !m_formsDockWidgetVisible);
    }
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    if (GeneralSettings::interfaceSettings()->preserveDesignerState) {
        setDockWidgetAreasVisible(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea, true);
        writeSettings();
    }

    QMainWindow::closeEvent(event);
}

void MainWindow::resetWidget()
{
    const bool wasVisible = isVisible();
    resize(sizeHint());
    move(UtilityFunctions::centerPos(size()));
    setWindowState(windowState() & ~Qt::WindowMaximized);
    restoreState(m_initialInterfaceState);
    setVisible(wasVisible); // setWindowState might hide the window

    m_controlsDockWidgetVisible = true;
    m_propertiesDockWidgetVisible = true;
    m_assetsDockWidgetVisible = true;
    m_toolboxDockWidgetVisible = true;
    m_formsDockWidgetVisible = true;

    m_runPane->segmentedBar()->actions().at(0)->setChecked(true);
    m_runPane->segmentedBar()->actions().at(2)->setChecked(true);
    m_centralWidget->outputController()->setPaneVisible(false);

    m_leftDockBar->removeDockWidget(m_controlsDockWidget);
    m_leftDockBar->removeDockWidget(m_propertiesDockWidget);
    m_leftDockBar->removeDockWidget(m_assetsDockWidget);
    m_leftDockBar->removeDockWidget(m_toolboxDockWidget);
    m_leftDockBar->removeDockWidget(m_formsDockWidget);

    m_rightDockBar->removeDockWidget(m_controlsDockWidget);
    m_rightDockBar->removeDockWidget(m_propertiesDockWidget);
    m_rightDockBar->removeDockWidget(m_assetsDockWidget);
    m_rightDockBar->removeDockWidget(m_toolboxDockWidget);
    m_rightDockBar->removeDockWidget(m_formsDockWidget);

    if (dockWidgetArea(m_controlsDockWidget) & Qt::LeftDockWidgetArea)
        m_leftDockBar->addDockWidget(m_controlsDockWidget);
    else
        m_rightDockBar->addDockWidget(m_controlsDockWidget);
    if (dockWidgetArea(m_propertiesDockWidget) & Qt::LeftDockWidgetArea)
        m_leftDockBar->addDockWidget(m_propertiesDockWidget);
    else
        m_rightDockBar->addDockWidget(m_propertiesDockWidget);
    if (dockWidgetArea(m_assetsDockWidget) & Qt::LeftDockWidgetArea)
        m_leftDockBar->addDockWidget(m_assetsDockWidget);
    else
        m_rightDockBar->addDockWidget(m_assetsDockWidget);
    if (dockWidgetArea(m_toolboxDockWidget) & Qt::LeftDockWidgetArea)
        m_leftDockBar->addDockWidget(m_toolboxDockWidget);
    else
        m_rightDockBar->addDockWidget(m_toolboxDockWidget);
    if (dockWidgetArea(m_formsDockWidget) & Qt::LeftDockWidgetArea)
        m_leftDockBar->addDockWidget(m_formsDockWidget);
    else
        m_rightDockBar->addDockWidget(m_formsDockWidget);
}

void MainWindow::readSettings()
{
    const bool wasVisible = isVisible();
    InterfaceSettings* settings = GeneralSettings::interfaceSettings();
    settings->begin();
    resize(settings->value<QSize>("MainWindow.Size", sizeHint()));
    move(settings->value<QPoint>("MainWindow.Position", UtilityFunctions::centerPos(size())));
    setWindowState(settings->value<bool>("MainWindow.Maximized", false)
                   ? (windowState() | Qt::WindowMaximized)
                   : (windowState() & ~Qt::WindowMaximized));
    restoreState(settings->value<QByteArray>("MainWindow.InterfaceState", QByteArray()));
    m_runPane->segmentedBar()->actions().at(0)->setChecked(settings->value<bool>("MainWindow.LeftSideVisible", true));
    m_runPane->segmentedBar()->actions().at(2)->setChecked(settings->value<bool>("MainWindow.RightSideVisible", true));
    m_runPane->segmentedBar()->actions().at(1)->setChecked(settings->value<bool>("MainWindow.BottomSideVisible", false));
    settings->end();
    setVisible(wasVisible); // setWindowState might hide the window
}

void MainWindow::writeSettings() const
{
    InterfaceSettings* settings = GeneralSettings::interfaceSettings();
    settings->begin();
    settings->setValue("MainWindow.Size", size());
    settings->setValue("MainWindow.Position", pos());
    settings->setValue("MainWindow.Maximized", isMaximized());
    settings->setValue("MainWindow.InterfaceState", saveState());
    settings->setValue("MainWindow.LeftSideVisible", m_runPane->segmentedBar()->actions().at(0)->isChecked());
    settings->setValue("MainWindow.RightSideVisible", m_runPane->segmentedBar()->actions().at(2)->isChecked());
    settings->setValue("MainWindow.BottomSideVisible", m_runPane->segmentedBar()->actions().at(1)->isChecked());
    settings->end();
}

QSize MainWindow::sizeHint() const
{
    return {1200, 700};
}
