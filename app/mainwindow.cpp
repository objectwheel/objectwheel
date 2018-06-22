#include <mainwindow.h>
#include <css.h>
#include <runpane.h>
#include <outputpane.h>
#include <toolboxpane.h>
#include <propertiespane.h>
#include <formspane.h>
#include <inspectorpane.h>
#include <pageswitcherpane.h>
#include <centralwidget.h>
#include <windowmanager.h>
#include <toolmanager.h>
#include <designerwidget.h>
#include <qmlcodeeditorwidget.h>
#include <controlcreationmanager.h>
#include <runmanager.h>
#include <projectmanager.h>
#include <consolebox.h>
#include <controlmonitoringmanager.h>
#include <control.h>
#include <toolboxsettingswindow.h>

#include <QProcess>
#include <QToolBar>
#include <QLabel>
#include <QToolButton>
#include <QDockWidget>
#include <QLayout>

#include <qmlcodedocument.h>
#include <QTimer>
#include <qmlcodeeditor.h>

namespace {
    QDockWidget* propertiesDockWidget;
    QDockWidget* formsDockWidget;
    QDockWidget* toolboxDockWidget;
    QDockWidget* inspectorDockWidget;
}

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent)
  , m_centralWidget(new CentralWidget)
  , m_runPane(new RunPane(m_centralWidget->outputPane()->consoleBox()))
  , m_formsPane(new FormsPane(m_centralWidget->designerWidget()->designerScene()))
  , m_toolboxPane(new ToolboxPane)
  , m_inspectorPane(new InspectorPane(m_centralWidget->designerWidget()->designerScene()))
  , m_propertiesPane(new PropertiesPane(m_centralWidget->designerWidget()->designerScene()))
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
    runBar->setStyleSheet("border: none; QToolBar { background: qlineargradient(spread:pad, x1:0.5, y1:0, "
                          "x2:0.5, y2:1, stop:0 #2784E3, stop:1 #0e5bad); }");
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
    pageSwitcherBar->setStyleSheet("QToolBar { background: #3b444c; border: none }");
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

    auto inspectorTitleBar = new QToolBar;
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

    auto propertiesTitleBar = new QToolBar;
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

    auto toolboxTitleBar = new QToolBar;
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

    auto formsTitleBar = new QToolBar;
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

    connect(m_pageSwitcherPane, SIGNAL(buildsActivated()), SLOT(hideDocks()));
    connect(m_pageSwitcherPane, SIGNAL(designerActivated()), SLOT(showDocks()));
    connect(m_pageSwitcherPane, SIGNAL(splitViewActivated()), SLOT(showDocks()));
    connect(m_pageSwitcherPane, SIGNAL(helpActivated()), SLOT(hideDocks()));
    connect(m_pageSwitcherPane, SIGNAL(qmlCodeEditorActivated()), SLOT(hideDocks()));
    connect(m_pageSwitcherPane, SIGNAL(projectOptionsActivated()), SLOT(hideDocks()));
    connect(m_pageSwitcherPane, SIGNAL(currentPageChanged(Pages)),
            m_centralWidget, SLOT(setCurrentPage(Pages)));

    connect(m_centralWidget->qmlCodeEditorWidget(), &QmlCodeEditorWidget::openControlCountChanged, [=] {
        if (m_centralWidget->qmlCodeEditorWidget()->openControlCount() <= 0
            && m_pageSwitcherPane->currentPage() != Page_SplitView)
            m_pageSwitcherPane->setCurrentPage(Page_Designer);

        if (m_centralWidget->qmlCodeEditorWidget()->openControlCount() > 0
            && m_pageSwitcherPane->currentPage() != Page_QmlCodeEditor)
            m_pageSwitcherPane->setCurrentPage(Page_SplitView);
    });

    connect(m_inspectorPane, SIGNAL(controlSelectionChanged(const QList<Control*>&)),
            m_centralWidget->designerWidget(), SLOT(onControlSelectionChange(const QList<Control*>&)));
    connect(m_inspectorPane, SIGNAL(controlDoubleClicked(Control*)),
            m_centralWidget->designerWidget(), SLOT(onControlDoubleClick(Control*)));
    connect(m_centralWidget->qmlCodeEditorWidget(), SIGNAL(documentSaved()),
            m_propertiesPane, SLOT(refreshList()));
    connect(ControlMonitoringManager::instance(), &ControlMonitoringManager::previewChanged,
            [=] (Control* control) { if (control->isSelected()) m_propertiesPane->refreshList(); });

    connect(RunManager::instance(),
    QOverload<int, QProcess::ExitStatus>::of(&RunManager::finished),
    [=] (int exitCode, QProcess::ExitStatus exitStatus)
    {
        auto pane = m_centralWidget->outputPane();
        auto console = pane->consoleBox();

        if (exitStatus == QProcess::CrashExit) {
            console->printFormatted(
                tr("The process was ended forcefully.\n"),
                "#b34b46",
                QFont::DemiBold
            );
        }

        console->printFormatted(
            ProjectManager::name() + tr(" exited with code %1.\n").arg(exitCode),
            "#025dbf",
            QFont::DemiBold
        );
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
    m_pageSwitcherPane->sweep();
}

void MainWindow::hideDocks()
{
    propertiesDockWidget->hide();
    formsDockWidget->hide();
    inspectorDockWidget->hide();
    toolboxDockWidget->hide();
}

void MainWindow::showDocks()
{
    propertiesDockWidget->show();
    formsDockWidget->show();
    inspectorDockWidget->show();
    toolboxDockWidget->show();
}

CentralWidget* MainWindow::centralWidget() const
{
    return m_centralWidget;
}

QSize MainWindow::sizeHint() const
{
    return {1260, 700};
}
