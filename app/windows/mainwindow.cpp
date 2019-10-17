#include <mainwindow.h>
#include <runpane.h>
#include <runcontroller.h>
#include <modeselectorpane.h>
#include <modeselectorcontroller.h>
#include <toolboxpane.h>
#include <toolboxcontroller.h>
#include <propertiespane.h>
#include <propertiescontroller.h>
#include <navigatorpane.h>
#include <navigatorcontroller.h>
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

#define CSS_DESIGNER_PINBAR "\
QToolBar { \
    border-top: 1px solid  #c4c4c4;\
    border-bottom: 1px solid #c4c4c4;\
    border-right: 1px solid #c4c4c4;\
    border-left: 2px solid #0D74C8;\
margin: 0px;\
background: qlineargradient(spread:pad, x1:0.5, y1:0, x2:0.5, y2:1, stop:0 #ffffff, stop:1 #e3e3e3); \
spacing: 2px; \
}"

namespace {
QByteArray resetState;
QDockWidget* assetsDockWidget;
QDockWidget* propertiesDockWidget;
QDockWidget* formsDockWidget;
QDockWidget* toolboxDockWidget;
QDockWidget* navigatorDockWidget;
QToolBar* assetsTitleBar;
QToolBar* propertiesTitleBar;
QToolBar* formsTitleBar;
QToolBar* toolboxTitleBar;
QToolBar* navigatorTitleBar;
bool assetsDockWidgetVisible;
bool propertiesDockWidgetVisible;
bool formsDockWidgetVisible;
bool toolboxDockWidgetVisible;
bool navigatorDockWidgetVisible;
}

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
  , m_navigatorPane(new NavigatorPane)
  , m_navigatorController(new NavigatorController(m_navigatorPane, m_centralWidget->designerPane()->designerView()->scene(), this))
  , m_formsPane(new FormsPane(m_centralWidget->designerPane()->designerView()->scene()))
  , m_assetsPane(new AssetsPane)
{
    setWindowTitle(APP_NAME);
    setAutoFillBackground(true);
    setCentralWidget(m_centralWidget);
    setContextMenuPolicy(Qt::NoContextMenu);
    setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
#if defined(Q_OS_MACOS)
    WindowOperations::removeTitleBar(this);
#endif

    QPalette palette(this->palette());
    palette.setColor(QPalette::Active, QPalette::ButtonText, "#505050");
    palette.setColor(QPalette::Inactive, QPalette::ButtonText, "#505050");
    palette.setColor(QPalette::Disabled, QPalette::ButtonText, "#9c9c9c");

    /** Set Tool Bars **/
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

    /** Set Dock Widgets **/
    /* Add Navigator Pane */
    auto navigatorTitleLabel = new QLabel;
    navigatorTitleLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    navigatorTitleLabel->setText("  " + tr("Navigator"));

    auto navigatorTitlePinButton = new QToolButton;
    navigatorTitlePinButton->setToolTip(tr("Pin/Unpin pane."));
    navigatorTitlePinButton->setCursor(Qt::PointingHandCursor);
    navigatorTitlePinButton->setIcon(QIcon(QStringLiteral(":/images/detach.svg")));
    connect(navigatorTitlePinButton, &QToolButton::clicked,
            this, [] {
        navigatorDockWidget->setFloating(!navigatorDockWidget->isFloating());
    });

    navigatorTitleBar = new QToolBar;
    navigatorTitleBar->addWidget(navigatorTitleLabel);
    navigatorTitleBar->addWidget(navigatorTitlePinButton);
    navigatorTitleBar->setStyleSheet(CSS_DESIGNER_PINBAR);
    navigatorTitleBar->setIconSize(QSize(11, 11));
    navigatorTitleBar->setFixedHeight(20);

    navigatorDockWidget = new QDockWidget;
    navigatorDockWidget->setObjectName("navigatorDockWidget");
    navigatorDockWidget->setTitleBarWidget(navigatorTitleBar);
    navigatorDockWidget->setWidget(m_navigatorPane);
    navigatorDockWidget->setWindowTitle(tr("Navigator"));
    navigatorDockWidget->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    addDockWidget(Qt::RightDockWidgetArea, navigatorDockWidget);

    /* Add Properties Pane */
    auto propertiesTitleLabel = new QLabel;
    propertiesTitleLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    propertiesTitleLabel->setText("  " + tr("Properties"));

    auto propertiesTitlePinButton = new QToolButton;
    propertiesTitlePinButton->setToolTip(tr("Pin/Unpin pane."));
    propertiesTitlePinButton->setCursor(Qt::PointingHandCursor);
    propertiesTitlePinButton->setIcon(QIcon(QStringLiteral(":/images/detach.svg")));
    connect(propertiesTitlePinButton, &QToolButton::clicked,
            this, [] {
        propertiesDockWidget->setFloating(!propertiesDockWidget->isFloating());
    });

    propertiesTitleBar = new QToolBar;
    propertiesTitleBar->addWidget(propertiesTitleLabel);
    propertiesTitleBar->addWidget(propertiesTitlePinButton);
    propertiesTitleBar->setStyleSheet(CSS_DESIGNER_PINBAR);
    propertiesTitleBar->setIconSize(QSize(11, 11));
    propertiesTitleBar->setFixedHeight(20);

    propertiesDockWidget = new QDockWidget;
    propertiesDockWidget->setObjectName("propertiesDockWidget");
    propertiesDockWidget->setTitleBarWidget(propertiesTitleBar);
    propertiesDockWidget->setWidget(m_propertiesPane);
    propertiesDockWidget->setWindowTitle(tr("Properties"));
    propertiesDockWidget->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    addDockWidget(Qt::RightDockWidgetArea, propertiesDockWidget);

    /* Add Assets Pane */
    auto assetsTitleLabel = new QLabel;
    assetsTitleLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    assetsTitleLabel->setText("  " + tr("Assets"));

    auto assetsTitlePinButton = new QToolButton;
    assetsTitlePinButton->setToolTip(tr("Pin/Unpin pane."));
    assetsTitlePinButton->setCursor(Qt::PointingHandCursor);
    assetsTitlePinButton->setIcon(QIcon(QStringLiteral(":/images/detach.svg")));
    connect(assetsTitlePinButton, &QToolButton::clicked,
            this, [] {
        assetsDockWidget->setFloating(!assetsDockWidget->isFloating());
    });

    assetsTitleBar = new QToolBar;
    assetsTitleBar->addWidget(assetsTitleLabel);
    assetsTitleBar->addWidget(assetsTitlePinButton);
    assetsTitleBar->setStyleSheet(CSS_DESIGNER_PINBAR);
    assetsTitleBar->setIconSize(QSize(11, 11));
    assetsTitleBar->setFixedHeight(20);

    assetsDockWidget = new QDockWidget;
    assetsDockWidget->setObjectName("assetsDockWidget");
    assetsDockWidget->setTitleBarWidget(assetsTitleBar);
    assetsDockWidget->setWidget(m_assetsPane);
    assetsDockWidget->setWindowTitle(tr("Assets"));
    assetsDockWidget->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    addDockWidget(Qt::RightDockWidgetArea, assetsDockWidget);
    connect(m_assetsPane, &AssetsPane::fileOpened,
            centralWidget()->qmlCodeEditorWidget(), &QmlCodeEditorWidget::openAssets);

    /* Add Toolbox Pane */
    auto toolboxTitleLabel = new QLabel;
    toolboxTitleLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    toolboxTitleLabel->setText("  " + tr("Toolbox"));

    auto toolboxTitlePinButton = new QToolButton;
    toolboxTitlePinButton->setToolTip(tr("Pin/Unpin pane."));
    toolboxTitlePinButton->setCursor(Qt::PointingHandCursor);
    toolboxTitlePinButton->setIcon(QIcon(QStringLiteral(":/images/detach.svg")));
    connect(toolboxTitlePinButton, &QToolButton::clicked,
            this, [] {
        toolboxDockWidget->setFloating(!toolboxDockWidget->isFloating());
    });

    toolboxTitleBar = new QToolBar;
    toolboxTitleBar->addWidget(toolboxTitleLabel);
    toolboxTitleBar->addWidget(toolboxTitlePinButton);
    toolboxTitleBar->setStyleSheet(CSS_DESIGNER_PINBAR);
    toolboxTitleBar->setIconSize(QSize(11, 11));
    toolboxTitleBar->setFixedHeight(20);

    toolboxDockWidget = new QDockWidget;
    toolboxDockWidget->setObjectName("toolboxDockWidget");
    toolboxDockWidget->setTitleBarWidget(toolboxTitleBar);
    toolboxDockWidget->setWidget(m_toolboxPane);
    toolboxDockWidget->setWindowTitle(tr("Toolbox"));
    toolboxDockWidget->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    addDockWidget(Qt::LeftDockWidgetArea, toolboxDockWidget);

    /* Add Forms Pane */
    auto formsTitleLabel = new QLabel;
    formsTitleLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    formsTitleLabel->setText("  " + tr("Forms"));

    auto formsTitlePinButton = new QToolButton;
    formsTitlePinButton->setToolTip(tr("Pin/Unpin pane."));
    formsTitlePinButton->setCursor(Qt::PointingHandCursor);
    formsTitlePinButton->setIcon(QIcon(QStringLiteral(":/images/detach.svg")));
    connect(formsTitlePinButton, &QToolButton::clicked,
            this, [] {
        formsDockWidget->setFloating(!formsDockWidget->isFloating());
    });

    formsTitleBar = new QToolBar;
    formsTitleBar->addWidget(formsTitleLabel);
    formsTitleBar->addWidget(formsTitlePinButton);
    formsTitleBar->setStyleSheet(CSS_DESIGNER_PINBAR);
    formsTitleBar->setIconSize(QSize(11, 11));
    formsTitleBar->setFixedHeight(20);

    formsDockWidget = new QDockWidget;
    formsDockWidget->setObjectName("formsDockWidget");
    formsDockWidget->setTitleBarWidget(formsTitleBar);
    formsDockWidget->setWidget(m_formsPane);
    formsDockWidget->setWindowTitle(tr("Forms"));
    formsDockWidget->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    addDockWidget(Qt::LeftDockWidgetArea, formsDockWidget);

    connect(ModeManager::instance(), &ModeManager::modeChanged,
            this, [=] (ModeManager::Mode mode) {
        if (mode == ModeManager::Designer || mode == ModeManager::Split)
            m_runPane->segmentedBar()->setEnabled(true);
        else
            m_runPane->segmentedBar()->setEnabled(false);
    });

    connect(ModeManager::instance(), &ModeManager::modeChanged,
            this, &MainWindow::onModeChange);
    connect(m_navigatorController, &NavigatorController::controlSelectionChanged,
            [=] (const QList<Control*>& selectedControls) {
        m_centralWidget->designerPane()->designerView()->scene()->clearSelection();
        for (Control* control : selectedControls)
            control->setSelected(true);
    });
    connect(m_navigatorController, &NavigatorController::goToSlotActionTriggered,
            m_centralWidget->designerController(), &DesignerController::onGoToSlotActionTrigger);
    connect(m_navigatorController, &NavigatorController::editAnchorsActionTriggered,
            m_centralWidget->designerController(), &DesignerController::onEditAnchorsActionTrigger);
    connect(m_navigatorController, &NavigatorController::viewSourceCodeActionTriggered,
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
    m_navigatorController->discharge();
    m_centralWidget->discharge();
    m_formsPane->discharge();
    m_assetsPane->discharge();

    showLeftPanes(true);
    showRightPanes(true);
}

void MainWindow::showLeftPanes(bool show)
{
    if (dockWidgetArea(assetsDockWidget) == Qt::LeftDockWidgetArea) {
        assetsDockWidget->setVisible(show);
        assetsDockWidgetVisible = show;
    }
    if (dockWidgetArea(propertiesDockWidget) == Qt::LeftDockWidgetArea) {
        propertiesDockWidget->setVisible(show);
        propertiesDockWidgetVisible = show;
    }
    if (dockWidgetArea(formsDockWidget) == Qt::LeftDockWidgetArea) {
        formsDockWidget->setVisible(show);
        formsDockWidgetVisible = show;
    }
    if (dockWidgetArea(navigatorDockWidget) == Qt::LeftDockWidgetArea) {
        navigatorDockWidget->setVisible(show);
        navigatorDockWidgetVisible = show;
    }
    if (dockWidgetArea(toolboxDockWidget) == Qt::LeftDockWidgetArea) {
        toolboxDockWidget->setVisible(show);
        toolboxDockWidgetVisible = show;
    }
}

void MainWindow::showRightPanes(bool show)
{
    if (dockWidgetArea(assetsDockWidget) == Qt::RightDockWidgetArea) {
        assetsDockWidget->setVisible(show);
        assetsDockWidgetVisible = show;
    }
    if (dockWidgetArea(propertiesDockWidget) == Qt::RightDockWidgetArea) {
        propertiesDockWidget->setVisible(show);
        propertiesDockWidgetVisible = show;
    }
    if (dockWidgetArea(formsDockWidget) == Qt::RightDockWidgetArea) {
        formsDockWidget->setVisible(show);
        formsDockWidgetVisible = show;
    }
    if (dockWidgetArea(navigatorDockWidget) == Qt::RightDockWidgetArea) {
        navigatorDockWidget->setVisible(show);
        navigatorDockWidgetVisible = show;
    }
    if (dockWidgetArea(toolboxDockWidget) == Qt::RightDockWidgetArea) {
        toolboxDockWidget->setVisible(show);
        toolboxDockWidgetVisible = show;
    }
}

void MainWindow::hideDocks()
{
    assetsDockWidget->hide();
    propertiesDockWidget->hide();
    formsDockWidget->hide();
    navigatorDockWidget->hide();
    toolboxDockWidget->hide();
}

void MainWindow::showDocks()
{
    assetsDockWidget->show();
    propertiesDockWidget->show();
    formsDockWidget->show();
    navigatorDockWidget->show();
    toolboxDockWidget->show();
}

void MainWindow::restoreDocks()
{
    assetsDockWidget->setVisible(assetsDockWidgetVisible);
    propertiesDockWidget->setVisible(propertiesDockWidgetVisible);
    formsDockWidget->setVisible(formsDockWidgetVisible);
    navigatorDockWidget->setVisible(navigatorDockWidgetVisible);
    toolboxDockWidget->setVisible(toolboxDockWidgetVisible);
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

NavigatorPane* MainWindow::navigatorPane() const
{
    return m_navigatorPane;
}

void MainWindow::resetSettings()
{
    InterfaceSettings* settings = GeneralSettings::interfaceSettings();
    settings->begin();
    settings->setValue("MainWindow.Size", sizeHint());
    settings->setValue("MainWindow.Position", UtilityFunctions::centerPos(sizeHint()));
    settings->setValue("MainWindow.Maximized", false);
    settings->setValue("MainWindow.Fullscreen", false);
    settings->setValue("MainWindow.WindowState", resetState);
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
