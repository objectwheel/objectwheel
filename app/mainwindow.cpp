#include <mainwindow.h>
#include <runpane.h>
#include <outputpane.h>
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
#include <consolebox.h>
#include <control.h>
#include <toolboxsettingswindow.h>
#include <qmlcodeeditorwidget.h>

#include <QProcess>
#include <QToolBar>
#include <QLabel>
#include <QToolButton>
#include <QDockWidget>
#include <QLayout>

#include <qmlcodedocument.h>
#include <QTimer>
#include <qmlcodeeditor.h>

#define CSS_DESIGNER_PINBAR "\
QToolBar { \
    border-top: 1px solid  #c6c6c6;\
    border-bottom: 1px solid #c6c6c6;\
    border-right: 1px solid #c6c6c6;\
    border-left: 3px solid #0D74C8;\
    margin-left: 1px; margin-right: 1px;\
    background: qlineargradient(spread:pad, x1:0.5, y1:0, x2:0.5, y2:1, stop:0 #ffffff, stop:1 #e3e3e3); \
    spacing: 5px; \
}"

namespace {
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
  , m_runPane(new RunPane(m_centralWidget->outputPane()->consoleBox()))
  , m_formsPane(new FormsPane(m_centralWidget->designerWidget()->designerScene()))
  , m_toolboxPane(new ToolboxPane)
  , m_inspectorPane(new InspectorPane(m_centralWidget->designerWidget()->designerScene()))
  , m_propertiesPane(new PropertiesPane(m_centralWidget->designerWidget()->designerScene()))
  , m_globalResourcesPane(new GlobalResourcesPane)
  , m_pageSwitcherPane(new PageSwitcherPane)

{
    QPalette p(palette());
    p.setColor(backgroundRole(), "#f0f0f0");
    setPalette(p);

    setWindowTitle(APP_NAME);
    setAutoFillBackground(true);
    setCentralWidget(m_centralWidget);
    setContextMenuPolicy(Qt::NoContextMenu);
    setStyleSheet("QMainWindow::separator{ height: 1px; }");

    /** Set Tool Bars **/
    /* Add Run Pane */
    auto runBar = new QToolBar;
    runBar->setOrientation(Qt::Horizontal);
    runBar->setFixedHeight(38);
    runBar->setFloatable(false);
    runBar->setMovable(false);
    runBar->setWindowTitle(tr("Run Bar"));
    runBar->setStyleSheet("border: none; spacing: 0;"
                          "QToolBar {"
                          "    background: qlineargradient(spread:pad, x1:0.5, y1:0,"
                          "    x2:0.5, y2:1, stop:0 #2784E3, stop:1 #0e5bad);"
                          "}");
    runBar->setContentsMargins(0, 0, 0, 0);
    runBar->layout()->setContentsMargins(0, 0, 0, 0);
    runBar->layout()->setSpacing(0);
    runBar->addWidget(m_runPane);
    m_runPane->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    addToolBar(Qt::TopToolBarArea, runBar);

    /* Add Page Switcher Pane */
    auto pageSwitcherBar = new QToolBar;
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
    runBar->layout()->setSpacing(0);
    pageSwitcherBar->addWidget(m_pageSwitcherPane);
    m_pageSwitcherPane->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    addToolBar(Qt::LeftToolBarArea , pageSwitcherBar);

    /** Set Dock Widgets **/
    QFont dockTitleFont;
    dockTitleFont.setWeight(QFont::Medium);

    /* Add Inspector Pane */
    auto inspectorTitleLabel = new QLabel;
    inspectorTitleLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    inspectorTitleLabel->setText(tr("   Control Inspector"));
    inspectorTitleLabel->setStyleSheet("color: black");
    inspectorTitleLabel->setFont(dockTitleFont);

    auto inspectorTitlePinButton = new QToolButton;
    inspectorTitlePinButton->setToolTip(tr("Pin/Unpin pane."));
    inspectorTitlePinButton->setCursor(Qt::PointingHandCursor);
    inspectorTitlePinButton->setIcon(QIcon(":/images/unpin.png"));
    connect(inspectorTitlePinButton, &QToolButton::clicked, this, [] {
        inspectorDockWidget->setFloating(!inspectorDockWidget->isFloating());
    });

    inspectorTitleBar = new QToolBar;
    inspectorTitleBar->addWidget(inspectorTitleLabel);
    inspectorTitleBar->addWidget(inspectorTitlePinButton);
    inspectorTitleBar->setStyleSheet(CSS_DESIGNER_PINBAR);
    inspectorTitleBar->setIconSize(QSize(11, 11));
    inspectorTitleBar->setFixedHeight(24);

    inspectorDockWidget = new QDockWidget;
    inspectorDockWidget->setStyleSheet("QDockWidget { border: none }");
    inspectorDockWidget->setTitleBarWidget(inspectorTitleBar);
    inspectorDockWidget->setWidget(m_inspectorPane);
    inspectorDockWidget->setWindowTitle(tr("Control Inspector"));
    inspectorDockWidget->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    addDockWidget(Qt::RightDockWidgetArea, inspectorDockWidget);

    /* Add Properties Pane */
    auto propertiesTitleLabel = new QLabel;
    propertiesTitleLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    propertiesTitleLabel->setText(tr("   Properties"));
    propertiesTitleLabel->setStyleSheet("color: black");
    propertiesTitleLabel->setFont(dockTitleFont);

    auto propertiesTitlePinButton = new QToolButton;
    propertiesTitlePinButton->setToolTip(tr("Pin/Unpin pane."));
    propertiesTitlePinButton->setCursor(Qt::PointingHandCursor);
    propertiesTitlePinButton->setIcon(QIcon(":/images/unpin.png"));
    connect(propertiesTitlePinButton, &QToolButton::clicked, this, [] {
        propertiesDockWidget->setFloating(!propertiesDockWidget->isFloating());
    });

    propertiesTitleBar = new QToolBar;
    propertiesTitleBar->addWidget(propertiesTitleLabel);
    propertiesTitleBar->addWidget(propertiesTitlePinButton);
    propertiesTitleBar->setStyleSheet(CSS_DESIGNER_PINBAR);
    propertiesTitleBar->setIconSize(QSize(11, 11));
    propertiesTitleBar->setFixedHeight(24);

    propertiesDockWidget = new QDockWidget;
    propertiesDockWidget->setStyleSheet("QDockWidget { border: none }");
    propertiesDockWidget->setTitleBarWidget(propertiesTitleBar);
    propertiesDockWidget->setWidget(m_propertiesPane);
    propertiesDockWidget->setWindowTitle(tr("Properties"));
    propertiesDockWidget->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    addDockWidget(Qt::RightDockWidgetArea, propertiesDockWidget);

    /* Add Global Resources Pane */
    auto globalTitleLabel = new QLabel;
    globalTitleLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    globalTitleLabel->setText(tr("   Global Resources"));
    globalTitleLabel->setStyleSheet("color: black");
    globalTitleLabel->setFont(dockTitleFont);

    auto globalTitlePinButton = new QToolButton;
    globalTitlePinButton->setToolTip(tr("Pin/Unpin pane."));
    globalTitlePinButton->setCursor(Qt::PointingHandCursor);
    globalTitlePinButton->setIcon(QIcon(":/images/unpin.png"));
    connect(globalTitlePinButton, &QToolButton::clicked, this, [] {
        globalDockWidget->setFloating(!globalDockWidget->isFloating());
    });

    globalTitleBar = new QToolBar;
    globalTitleBar->addWidget(globalTitleLabel);
    globalTitleBar->addWidget(globalTitlePinButton);
    globalTitleBar->setStyleSheet(CSS_DESIGNER_PINBAR);
    globalTitleBar->setIconSize(QSize(11, 11));
    globalTitleBar->setFixedHeight(24);

    globalDockWidget = new QDockWidget;
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
    toolboxTitleLabel->setText(tr("   Toolbox"));
    toolboxTitleLabel->setStyleSheet("color: black");
    toolboxTitleLabel->setFont(dockTitleFont);

    auto toolboxTitlePinButton = new QToolButton;
    toolboxTitlePinButton->setToolTip(tr("Pin/Unpin pane."));
    toolboxTitlePinButton->setCursor(Qt::PointingHandCursor);
    toolboxTitlePinButton->setIcon(QIcon(":/images/unpin.png"));
    connect(toolboxTitlePinButton, &QToolButton::clicked, this, [] {
        toolboxDockWidget->setFloating(!toolboxDockWidget->isFloating());
    });

    auto toolboxSettingsButton = new QToolButton;
    toolboxSettingsButton->setToolTip(tr("Toolbox settings."));
    toolboxSettingsButton->setCursor(Qt::PointingHandCursor);
    toolboxSettingsButton->setIcon(QIcon(":/images/settings.png"));
    connect(toolboxSettingsButton, &QToolButton::clicked, this, [=] {
        WindowManager::toolboxSettingsWindow()->show();
    });

    toolboxTitleBar = new QToolBar;
    toolboxTitleBar->addWidget(toolboxTitleLabel);
    toolboxTitleBar->addWidget(toolboxSettingsButton);
    toolboxTitleBar->addWidget(toolboxTitlePinButton);
    toolboxTitleBar->setStyleSheet(CSS_DESIGNER_PINBAR);
    toolboxTitleBar->setIconSize(QSize(11, 11));
    toolboxTitleBar->setFixedHeight(24);

    toolboxDockWidget = new QDockWidget;
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
    formsTitleLabel->setText(tr("   Form Navigator"));
    formsTitleLabel->setStyleSheet("color: black");
    formsTitleLabel->setFont(dockTitleFont);

    auto formsTitlePinButton = new QToolButton;
    formsTitlePinButton->setToolTip(tr("Pin/Unpin pane."));
    formsTitlePinButton->setCursor(Qt::PointingHandCursor);
    formsTitlePinButton->setIcon(QIcon(":/images/unpin.png"));
    connect(formsTitlePinButton, &QToolButton::clicked, this, [] {
        formsDockWidget->setFloating(!formsDockWidget->isFloating());
    });

    formsTitleBar = new QToolBar;
    formsTitleBar->addWidget(formsTitleLabel);
    formsTitleBar->addWidget(formsTitlePinButton);
    formsTitleBar->setStyleSheet(CSS_DESIGNER_PINBAR);
    formsTitleBar->setIconSize(QSize(11, 11));
    formsTitleBar->setFixedHeight(24);

    formsDockWidget = new QDockWidget;
    formsDockWidget->setStyleSheet("QDockWidget { border: none }");
    formsDockWidget->setTitleBarWidget(formsTitleBar);
    formsDockWidget->setWidget(m_formsPane);
    formsDockWidget->setWindowTitle(tr("Form Navigator"));
    formsDockWidget->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    addDockWidget(Qt::LeftDockWidgetArea, formsDockWidget);

    connect(m_centralWidget->designerWidget(), SIGNAL(hideDockWidgetTitleBars(bool)), SLOT(setDockWidgetTitleBarsHidden(bool)));
    connect(m_pageSwitcherPane, SIGNAL(buildsActivated()), SLOT(hideDocks()));
    connect(m_pageSwitcherPane, SIGNAL(designerActivated()), SLOT(restoreDocks()));
    connect(m_pageSwitcherPane, SIGNAL(splitViewActivated()), SLOT(restoreDocks()));
    connect(m_pageSwitcherPane, SIGNAL(helpActivated()), SLOT(hideDocks()));
    connect(m_pageSwitcherPane, SIGNAL(qmlCodeEditorActivated()), SLOT(hideDocks()));
    connect(m_pageSwitcherPane, SIGNAL(projectOptionsActivated()), SLOT(hideDocks()));
    connect(m_pageSwitcherPane, SIGNAL(currentPageChanged(Pages)), m_centralWidget, SLOT(setCurrentPage(Pages)));
    connect(m_pageSwitcherPane, SIGNAL(leftPanesShowChanged(bool)), this, SLOT(showLeftPanes(bool)));
    connect(m_pageSwitcherPane, SIGNAL(rightPanesShowChanged(bool)), this, SLOT(showRightPanes(bool)));

    connect(m_centralWidget->qmlCodeEditorWidget(), &QmlCodeEditorWidget::opened, [=] {
        if (m_centralWidget->qmlCodeEditorWidget()->count() <= 0
                && m_pageSwitcherPane->currentPage() != Page_SplitView) {
            m_pageSwitcherPane->setCurrentPage(Page_Designer);
        }
        if (m_centralWidget->qmlCodeEditorWidget()->count() > 0
                && m_pageSwitcherPane->currentPage() != Page_QmlCodeEditor) {
            m_pageSwitcherPane->setCurrentPage(Page_SplitView);
        }
    });

    connect(m_inspectorPane, SIGNAL(controlSelectionChanged(const QList<Control*>&)),
            m_centralWidget->designerWidget(), SLOT(onControlSelectionChange(const QList<Control*>&)));
    connect(m_inspectorPane, SIGNAL(controlDoubleClicked(Control*)),
            m_centralWidget->designerWidget(), SLOT(onInspectorItemDoubleClick(Control*)));

    connect(RunManager::instance(), qOverload<int, QProcess::ExitStatus>(&RunManager::finished), [=]
            (int exitCode, QProcess::ExitStatus)
    {
        auto pane = m_centralWidget->outputPane();
        auto console = pane->consoleBox();

        if (exitCode == EXIT_FAILURE) {
            console->printFormatted(tr("The process was ended forcefully.\n"), "#b34b46",
                                    QFont::DemiBold);
        }

        console->printFormatted(ProjectManager::name() + tr(" exited with code %1.\n").arg(exitCode),
                                "#025dbf", QFont::DemiBold);
    });

    sweep();
}

void MainWindow::sweep()
{
    m_runPane->sweep();
    m_centralWidget->sweep();
    m_formsPane->sweep();
    m_toolboxPane->sweep();
    m_inspectorPane->sweep();
    m_propertiesPane->sweep();
    m_globalResourcesPane->sweep();
    m_pageSwitcherPane->sweep();
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
    m_pageSwitcherPane->setLeftPanesShow(show);
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
    m_pageSwitcherPane->setRightPanesShow(show);
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

QSize MainWindow::sizeHint() const
{
    return {1260, 700};
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
