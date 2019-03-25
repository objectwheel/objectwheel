#include <mainwindow.h>
#include <runpane.h>
#include <runcontroller.h>
#include <toolboxpane.h>
#include <propertiespane.h>
#include <globalresourcespane.h>
#include <formspane.h>
#include <inspectorpane.h>
#include <pageswitcherpane.h>
#include <centralwidget.h>
#include <windowmanager.h>
#include <toolmanager.h>
#include <designerwidget.h>
#include <controlcreationmanager.h>
#include <runmanager.h>
#include <projectmanager.h>
#include <consolepane.h>
#include <control.h>
#include <qmlcodeeditorwidget.h>
#include <utilityfunctions.h>
#include <bottombar.h>
#include <saveutils.h>
#include <windowmanager.h>
#include <welcomewindow.h>
#include <controlpropertymanager.h>
#include <preferenceswindow.h>
#include <generalsettings.h>
#include <interfacesettings.h>
#include <codeeditorsettings.h>
#include <behaviorsettings.h>

#include <QProcess>
#include <QToolBar>
#include <QLabel>
#include <QToolButton>
#include <QDockWidget>
#include <QLayout>
#include <QScrollBar>
#include <QScreen>

#if defined(Q_OS_MACOS)
#include <windowoperations.h>
#endif

#define CSS_DESIGNER_PINBAR "\
QToolBar { \
    border-top: 1px solid  #c4c4c4;\
    border-bottom: 1px solid #c4c4c4;\
    border-right: 1px solid #c4c4c4;\
    border-left: 3px solid #0D74C8;\
    margin: 0px;\
    background: qlineargradient(spread:pad, x1:0.5, y1:0, x2:0.5, y2:1, stop:0 #ffffff, stop:1 #e3e3e3); \
    spacing: 5px; \
}"

namespace {
QByteArray resetState;
QDockWidget* globalDockWidget;
QDockWidget* propertiesDockWidget;
QDockWidget* formsDockWidget;
QDockWidget* toolboxDockWidget;
QDockWidget* inspectorDockWidget;
QToolBar* globalTitleBar;
QToolBar* propertiesTitleBar;
QToolBar* formsTitleBar;
QToolBar* toolboxTitleBar;
QToolBar* inspectorTitleBar;
bool globalDockWidgetVisible;
bool propertiesDockWidgetVisible;
bool formsDockWidgetVisible;
bool toolboxDockWidgetVisible;
bool inspectorDockWidgetVisible;
}

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent)
  , m_centralWidget(new CentralWidget)
  , m_runPane(new RunPane(this))
  , m_runController(new RunController(m_runPane, this))
  , m_formsPane(new FormsPane(m_centralWidget->designerWidget()->designerScene()))
  , m_toolboxPane(new ToolboxPane)
  , m_inspectorPane(new InspectorPane(m_centralWidget->designerWidget()->designerScene()))
  , m_propertiesPane(new PropertiesPane(m_centralWidget->designerWidget()->designerScene()))
  , m_globalResourcesPane(new GlobalResourcesPane)
  , m_pageSwitcherPane(new PageSwitcherPane)
{
    setWindowTitle(APP_NAME);
    setAutoFillBackground(true);
    setCentralWidget(m_centralWidget);
    setContextMenuPolicy(Qt::NoContextMenu);
    setStyleSheet("QMainWindow::separator{ height: 1px; }");
#if defined(Q_OS_MACOS)
    WindowOperations::removeTitleBar(this);
#endif

    /** Set Tool Bars **/
    /* Add Run Pane */
    m_runPane->setObjectName("runController");
    m_runPane->setOrientation(Qt::Horizontal);
    m_runPane->setFloatable(false);
    m_runPane->setMovable(false);
    m_runPane->setWindowTitle(tr("Run Bar"));
    addToolBar(Qt::TopToolBarArea, m_runPane);

    /* Add Page Switcher Pane */
    auto pageSwitcherBar = new QToolBar;
    pageSwitcherBar->setObjectName("pageSwitcherBar");
    pageSwitcherBar->setOrientation(Qt::Vertical);
    pageSwitcherBar->setFixedWidth(70);
    pageSwitcherBar->setWindowFlags(pageSwitcherBar->windowFlags() | Qt::WindowStaysOnTopHint);
    pageSwitcherBar->setFloatable(false);
    pageSwitcherBar->setMovable(false);
    pageSwitcherBar->setWindowTitle(tr("Page Bar"));
    pageSwitcherBar->setStyleSheet("border: none; spacing: 0;"
                                   "QToolBar { background: #3b444c; border: none }");
    pageSwitcherBar->setContentsMargins(0, 0, 0, 0);
    pageSwitcherBar->layout()->setContentsMargins(0, 0, 0, 0);
    pageSwitcherBar->layout()->setSpacing(0);
    pageSwitcherBar->addWidget(m_pageSwitcherPane);
    m_pageSwitcherPane->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    addToolBar(Qt::LeftToolBarArea , pageSwitcherBar);

    /** Set Dock Widgets **/
    /* Add Inspector Pane */
    auto inspectorTitleLabel = new QLabel;
    inspectorTitleLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    inspectorTitleLabel->setText("   " + tr("Control Inspector"));

    auto inspectorTitlePinButton = new QToolButton;
    inspectorTitlePinButton->setToolTip(tr("Pin/Unpin pane."));
    inspectorTitlePinButton->setCursor(Qt::PointingHandCursor);
    inspectorTitlePinButton->setIcon(QIcon(":/images/unpin.png"));
    connect(inspectorTitlePinButton, &QToolButton::clicked,
            this, [] {
        inspectorDockWidget->setFloating(!inspectorDockWidget->isFloating());
    });

    inspectorTitleBar = new QToolBar;
    inspectorTitleBar->addWidget(inspectorTitleLabel);
    inspectorTitleBar->addWidget(inspectorTitlePinButton);
    inspectorTitleBar->setStyleSheet(CSS_DESIGNER_PINBAR);
    inspectorTitleBar->setIconSize(QSize(11, 11));
    inspectorTitleBar->setFixedHeight(24);

    inspectorDockWidget = new QDockWidget;
    inspectorDockWidget->setObjectName("inspectorDockWidget");
    inspectorDockWidget->setStyleSheet("QDockWidget { border: none }");
    inspectorDockWidget->setTitleBarWidget(inspectorTitleBar);
    inspectorDockWidget->setWidget(m_inspectorPane);
    inspectorDockWidget->setWindowTitle(tr("Control Inspector"));
    inspectorDockWidget->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    addDockWidget(Qt::RightDockWidgetArea, inspectorDockWidget);

    /* Add Properties Pane */
    auto propertiesTitleLabel = new QLabel;
    propertiesTitleLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    propertiesTitleLabel->setText("   " + tr("Properties"));

    auto propertiesTitlePinButton = new QToolButton;
    propertiesTitlePinButton->setToolTip(tr("Pin/Unpin pane."));
    propertiesTitlePinButton->setCursor(Qt::PointingHandCursor);
    propertiesTitlePinButton->setIcon(QIcon(":/images/unpin.png"));
    connect(propertiesTitlePinButton, &QToolButton::clicked,
            this, [] {
        propertiesDockWidget->setFloating(!propertiesDockWidget->isFloating());
    });

    propertiesTitleBar = new QToolBar;
    propertiesTitleBar->addWidget(propertiesTitleLabel);
    propertiesTitleBar->addWidget(propertiesTitlePinButton);
    propertiesTitleBar->setStyleSheet(CSS_DESIGNER_PINBAR);
    propertiesTitleBar->setIconSize(QSize(11, 11));
    propertiesTitleBar->setFixedHeight(24);

    propertiesDockWidget = new QDockWidget;
    propertiesDockWidget->setObjectName("propertiesDockWidget");
    propertiesDockWidget->setStyleSheet("QDockWidget { border: none }");
    propertiesDockWidget->setTitleBarWidget(propertiesTitleBar);
    propertiesDockWidget->setWidget(m_propertiesPane);
    propertiesDockWidget->setWindowTitle(tr("Properties"));
    propertiesDockWidget->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    addDockWidget(Qt::RightDockWidgetArea, propertiesDockWidget);

    /* Add Global Resources Pane */
    auto globalTitleLabel = new QLabel;
    globalTitleLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    globalTitleLabel->setText("   " + tr("Global Resources"));

    auto globalTitlePinButton = new QToolButton;
    globalTitlePinButton->setToolTip(tr("Pin/Unpin pane."));
    globalTitlePinButton->setCursor(Qt::PointingHandCursor);
    globalTitlePinButton->setIcon(QIcon(":/images/unpin.png"));
    connect(globalTitlePinButton, &QToolButton::clicked,
            this, [] {
        globalDockWidget->setFloating(!globalDockWidget->isFloating());
    });

    globalTitleBar = new QToolBar;
    globalTitleBar->addWidget(globalTitleLabel);
    globalTitleBar->addWidget(globalTitlePinButton);
    globalTitleBar->setStyleSheet(CSS_DESIGNER_PINBAR);
    globalTitleBar->setIconSize(QSize(11, 11));
    globalTitleBar->setFixedHeight(24);

    globalDockWidget = new QDockWidget;
    globalDockWidget->setObjectName("globalDockWidget");
    globalDockWidget->setStyleSheet("QDockWidget { border: none }");
    globalDockWidget->setTitleBarWidget(globalTitleBar);
    globalDockWidget->setWidget(m_globalResourcesPane);
    globalDockWidget->setWindowTitle(tr("Global Resources"));
    globalDockWidget->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    addDockWidget(Qt::RightDockWidgetArea, globalDockWidget);
    connect(m_globalResourcesPane, &GlobalResourcesPane::fileOpened,
            centralWidget()->qmlCodeEditorWidget(), &QmlCodeEditorWidget::openGlobal);

    /* Add Toolbox Pane */
    auto toolboxTitleLabel = new QLabel;
    toolboxTitleLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    toolboxTitleLabel->setText("   " + tr("Toolbox"));

    auto toolboxTitlePinButton = new QToolButton;
    toolboxTitlePinButton->setToolTip(tr("Pin/Unpin pane."));
    toolboxTitlePinButton->setCursor(Qt::PointingHandCursor);
    toolboxTitlePinButton->setIcon(QIcon(":/images/unpin.png"));
    connect(toolboxTitlePinButton, &QToolButton::clicked,
            this, [] {
        toolboxDockWidget->setFloating(!toolboxDockWidget->isFloating());
    });

    toolboxTitleBar = new QToolBar;
    toolboxTitleBar->addWidget(toolboxTitleLabel);
    toolboxTitleBar->addWidget(toolboxTitlePinButton);
    toolboxTitleBar->setStyleSheet(CSS_DESIGNER_PINBAR);
    toolboxTitleBar->setIconSize(QSize(11, 11));
    toolboxTitleBar->setFixedHeight(24);

    toolboxDockWidget = new QDockWidget;
    toolboxDockWidget->setObjectName("toolboxDockWidget");
    toolboxDockWidget->setStyleSheet("QDockWidget { border: none }");
    toolboxDockWidget->setTitleBarWidget(toolboxTitleBar);
    toolboxDockWidget->setWidget(m_toolboxPane);
    toolboxDockWidget->setWindowTitle(tr("Toolbox"));
    toolboxDockWidget->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    addDockWidget(Qt::LeftDockWidgetArea, toolboxDockWidget);
    ToolManager::addToolboxTree(m_toolboxPane->toolboxTree());

    /* Add Forms Pane */
    auto formsTitleLabel = new QLabel;
    formsTitleLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    formsTitleLabel->setText("   " + tr("Form Navigator"));

    auto formsTitlePinButton = new QToolButton;
    formsTitlePinButton->setToolTip(tr("Pin/Unpin pane."));
    formsTitlePinButton->setCursor(Qt::PointingHandCursor);
    formsTitlePinButton->setIcon(QIcon(":/images/unpin.png"));
    connect(formsTitlePinButton, &QToolButton::clicked,
            this, [] {
        formsDockWidget->setFloating(!formsDockWidget->isFloating());
    });

    formsTitleBar = new QToolBar;
    formsTitleBar->addWidget(formsTitleLabel);
    formsTitleBar->addWidget(formsTitlePinButton);
    formsTitleBar->setStyleSheet(CSS_DESIGNER_PINBAR);
    formsTitleBar->setIconSize(QSize(11, 11));
    formsTitleBar->setFixedHeight(24);

    formsDockWidget = new QDockWidget;
    formsDockWidget->setObjectName("formsDockWidget");
    formsDockWidget->setStyleSheet("QDockWidget { border: none }");
    formsDockWidget->setTitleBarWidget(formsTitleBar);
    formsDockWidget->setWidget(m_formsPane);
    formsDockWidget->setWindowTitle(tr("Form Navigator"));
    formsDockWidget->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    addDockWidget(Qt::LeftDockWidgetArea, formsDockWidget);

    connect(m_centralWidget->designerWidget(), &DesignerWidget::hideDockWidgetTitleBars,
            this, &MainWindow::setDockWidgetTitleBarsHidden);
    connect(m_centralWidget->bottomBar(), &BottomBar::showHideLeftPanesButtonActivated,
            this, &MainWindow::showLeftPanes);
    connect(m_centralWidget->bottomBar(), &BottomBar::showHideRightPanesButtonActivated,
            this, &MainWindow::showRightPanes);
    connect(m_pageSwitcherPane, &PageSwitcherPane::buildsActivated,
            this, &MainWindow::hideDocks);
    connect(m_pageSwitcherPane, &PageSwitcherPane::designerActivated,
            this, &MainWindow::restoreDocks);
    connect(m_pageSwitcherPane, &PageSwitcherPane::splitViewActivated,
            this, &MainWindow::restoreDocks);
    connect(m_pageSwitcherPane, &PageSwitcherPane::helpActivated,
            this, &MainWindow::hideDocks);
    connect(m_pageSwitcherPane, &PageSwitcherPane::qmlCodeEditorActivated,
            this, &MainWindow::hideDocks);
    connect(m_pageSwitcherPane, &PageSwitcherPane::projectOptionsActivated,
            this, &MainWindow::hideDocks);
    connect(m_pageSwitcherPane, &PageSwitcherPane::currentPageChanged,
            m_centralWidget, &CentralWidget::setCurrentPage);
    connect(m_inspectorPane, &InspectorPane::controlSelectionChanged,
            m_centralWidget->designerWidget(), &DesignerWidget::onControlSelectionChange);
    connect(m_inspectorPane, &InspectorPane::controlDoubleClicked,
            m_centralWidget->designerWidget(), &DesignerWidget::onInspectorItemDoubleClick);
    connect(m_centralWidget->qmlCodeEditorWidget(), &QmlCodeEditorWidget::opened,
            [=] {
        if (m_centralWidget->qmlCodeEditorWidget()->count() <= 0
                && m_pageSwitcherPane->currentPage() != Page_SplitView) {
            m_pageSwitcherPane->setCurrentPage(Page_Designer);
        }
        if (m_centralWidget->qmlCodeEditorWidget()->count() > 0
                && m_pageSwitcherPane->currentPage() != Page_QmlCodeEditor) {
            m_pageSwitcherPane->setCurrentPage(Page_SplitView);
        }
    });
    connect(ProjectManager::instance(), &ProjectManager::started,
            this, [=] { m_globalResourcesPane->setRootPath(SaveUtils::toGlobalDir(ProjectManager::dir())); });
    connect(ControlPropertyManager::instance(), &ControlPropertyManager::idChanged,
            m_formsPane, &FormsPane::refresh);
    connect(GeneralSettings::instance(), &GeneralSettings::designerStateReset,
            this, &MainWindow::resetSettings);


    connect(RunManager::instance(), &RunManager::processReadyOutput, this, [=] (const QString& output)
    { m_centralWidget->consolePane()->press(output, palette().linkVisited()); });
    connect(RunManager::instance(), &RunManager::deviceReadyOutput, this, [=] (const QString& output)
    { m_centralWidget->consolePane()->press(output, palette().linkVisited()); });
    connect(RunManager::instance(), &RunManager::processFinished,
            [=] (int exitCode, QProcess::ExitStatus exitStatus) {
        auto console = m_centralWidget->consolePane();
        auto timestamp = QTime::currentTime().toString();
        if (exitStatus != QProcess::CrashExit) {
            if (exitCode == EXIT_FAILURE) {
                console->press(timestamp + tr(": The application has not exited normally.\n"),
                               QColor("#b34b46"), QFont::DemiBold);
            }
            console->press(timestamp + ": " + ProjectManager::name() + tr(" exited with code ") +
                           QString::fromUtf8("%1.\n").arg(exitCode), QColor("#025dbf"), QFont::DemiBold);
        }
    });
    connect(RunManager::instance(), &RunManager::processErrorOccurred,
            [=] (QProcess::ProcessError error, const QString& errorString) {
        auto console = m_centralWidget->consolePane();
        auto timestamp = QTime::currentTime().toString();
        if (error == QProcess::FailedToStart) {
            console->press(timestamp + tr(": System Failure: ") + errorString + "\n",
                           QColor("#b34b46"), QFont::DemiBold);
            console->press(timestamp + ": " + ProjectManager::name() + tr(" has failed to start.\n"),
                           QColor("#025dbf"), QFont::DemiBold);
        } else {
            console->press(timestamp + tr(": The application has unexpectedly finished.\n"),
                           QColor("#b34b46"), QFont::DemiBold);
            console->press(timestamp + ": " + ProjectManager::name() + tr(" has crashed.\n"),
                           QColor("#025dbf"), QFont::DemiBold);
        }
    });
    connect(m_runController, &RunController::ran, this, [=] {
        auto timestamp = QTime::currentTime().toString();
        BehaviorSettings* settings = CodeEditorSettings::behaviorSettings();
        if (settings->autoSaveBeforeRunning)
            WindowManager::mainWindow()->centralWidget()->qmlCodeEditorWidget()->saveAll();
        m_centralWidget->consolePane()->fade();
        if (!m_centralWidget->consolePane()->toPlainText().isEmpty())
            m_centralWidget->consolePane()->press("\n");
        m_centralWidget->consolePane()->press(timestamp + tr(": Starting") + " " + ProjectManager::name() + "...\n",
                                              QColor("#025dbf"), QFont::DemiBold);
        m_centralWidget->consolePane()->verticalScrollBar()->
                setValue(m_centralWidget->consolePane()->verticalScrollBar()->maximum());
    });

    discharge();
    //resetState = saveState();    
}

void MainWindow::discharge()
{
    m_runController->discharge();
    m_centralWidget->discharge();
    m_formsPane->discharge();
    m_toolboxPane->discharge();
    m_inspectorPane->discharge();
    m_propertiesPane->discharge();
    m_globalResourcesPane->discharge();
    m_pageSwitcherPane->discharge();

    showLeftPanes(true);
    showRightPanes(true);
}

void MainWindow::showLeftPanes(bool show)
{
    if (dockWidgetArea(globalDockWidget) == Qt::LeftDockWidgetArea) {
        globalDockWidget->setVisible(show);
        globalDockWidgetVisible = show;
    }
    if (dockWidgetArea(propertiesDockWidget) == Qt::LeftDockWidgetArea) {
        propertiesDockWidget->setVisible(show);
        propertiesDockWidgetVisible = show;
    }
    if (dockWidgetArea(formsDockWidget) == Qt::LeftDockWidgetArea) {
        formsDockWidget->setVisible(show);
        formsDockWidgetVisible = show;
    }
    if (dockWidgetArea(inspectorDockWidget) == Qt::LeftDockWidgetArea) {
        inspectorDockWidget->setVisible(show);
        inspectorDockWidgetVisible = show;
    }
    if (dockWidgetArea(toolboxDockWidget) == Qt::LeftDockWidgetArea) {
        toolboxDockWidget->setVisible(show);
        toolboxDockWidgetVisible = show;
    }
}

void MainWindow::showRightPanes(bool show)
{
    if (dockWidgetArea(globalDockWidget) == Qt::RightDockWidgetArea) {
        globalDockWidget->setVisible(show);
        globalDockWidgetVisible = show;
    }
    if (dockWidgetArea(propertiesDockWidget) == Qt::RightDockWidgetArea) {
        propertiesDockWidget->setVisible(show);
        propertiesDockWidgetVisible = show;
    }
    if (dockWidgetArea(formsDockWidget) == Qt::RightDockWidgetArea) {
        formsDockWidget->setVisible(show);
        formsDockWidgetVisible = show;
    }
    if (dockWidgetArea(inspectorDockWidget) == Qt::RightDockWidgetArea) {
        inspectorDockWidget->setVisible(show);
        inspectorDockWidgetVisible = show;
    }
    if (dockWidgetArea(toolboxDockWidget) == Qt::RightDockWidgetArea) {
        toolboxDockWidget->setVisible(show);
        toolboxDockWidgetVisible = show;
    }
}

void MainWindow::setDockWidgetTitleBarsHidden(bool yes)
{
    static auto w1 = new QWidget(this);
    static auto w2 = new QWidget(this);
    static auto w3 = new QWidget(this);
    static auto w4 = new QWidget(this);
    static auto w5 = new QWidget(this);
    static bool set = false;

    if (!set) {
        set = true;
        w1->setMinimumSize(0, 0);
        w2->setMinimumSize(0, 0);
        w3->setMinimumSize(0, 0);
        w4->setMinimumSize(0, 0);
        w5->setMinimumSize(0, 0);

        w1->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
        w2->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
        w3->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
        w4->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
        w5->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    }

    if (yes) {
        formsDockWidget->setTitleBarWidget(w1);
        toolboxDockWidget->setTitleBarWidget(w2);
        inspectorDockWidget->setTitleBarWidget(w3);
        propertiesDockWidget->setTitleBarWidget(w4);
        globalDockWidget->setTitleBarWidget(w5);
    } else {
        formsDockWidget->setTitleBarWidget(formsTitleBar);
        toolboxDockWidget->setTitleBarWidget(toolboxTitleBar);
        inspectorDockWidget->setTitleBarWidget(inspectorTitleBar);
        propertiesDockWidget->setTitleBarWidget(propertiesTitleBar);
        globalDockWidget->setTitleBarWidget(globalTitleBar);
    }
}

void MainWindow::hideDocks()
{
    globalDockWidget->hide();
    propertiesDockWidget->hide();
    formsDockWidget->hide();
    inspectorDockWidget->hide();
    toolboxDockWidget->hide();
}

void MainWindow::showDocks()
{
    globalDockWidget->show();
    propertiesDockWidget->show();
    formsDockWidget->show();
    inspectorDockWidget->show();
    toolboxDockWidget->show();
}

void MainWindow::restoreDocks()
{
    globalDockWidget->setVisible(globalDockWidgetVisible);
    propertiesDockWidget->setVisible(propertiesDockWidgetVisible);
    formsDockWidget->setVisible(formsDockWidgetVisible);
    inspectorDockWidget->setVisible(inspectorDockWidgetVisible);
    toolboxDockWidget->setVisible(toolboxDockWidgetVisible);
}

CentralWidget* MainWindow::centralWidget() const
{
    return m_centralWidget;
}

GlobalResourcesPane* MainWindow::globalResourcesPane() const
{
    return m_globalResourcesPane;
}

PropertiesPane* MainWindow::propertiesPane() const
{
    return m_propertiesPane;
}

InspectorPane* MainWindow::inspectorPane() const
{
    return m_inspectorPane;
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
    event->accept();
}

QSize MainWindow::sizeHint() const
{
    return {1260, 700};
}
