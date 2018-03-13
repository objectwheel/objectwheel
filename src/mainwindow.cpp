#include <mainwindow.h>
#include <fit.h>
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
#include <toolsbackend.h>
#include <designerwidget.h>

#include <QToolBar>
#include <QLabel>
#include <QToolButton>
#include <QDockWidget>

namespace {
    QDockWidget* propertiesDockWidget;
    QDockWidget* formsDockWidget;
    QDockWidget* toolboxDockWidget;
    QDockWidget* inspectorDockWidget;
}

RunPane* m_runPane;
FormsPane* m_formsPane;
OutputPane* m_outputPane;
ToolboxPane* m_toolboxPane;
InspectorPane* m_inspectorPane;
PropertiesPane* m_propertiesPane;
PageSwitcherPane* m_pageSwitcherPane;

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent)
  , m_centralWidget(new CentralWidget)
  , m_runPane(new RunPane(m_outputPane->consoleBox()))
  , m_formsPane(new FormsPane(m_centralWidget->designerWidget()->designerScene()))
  , m_outputPane(new OutputPane)
  , m_toolboxPane(new ToolboxPane)
  , m_inspectorPane(new InspectorPane(m_centralWidget->designerWidget()->designerScene()))
  , m_propertiesPane(new PropertiesPane(m_centralWidget->designerWidget()->designerScene()))
  , m_pageSwitcherPane(new PageSwitcherPane)

{
    QPalette p(palette());
    p.setColor(backgroundRole(), "#f0f4f7");
    setStyleSheet("QMainWindow::separator{ height: 1px; }");
    setCentralWidget(m_centralWidget);
    setAutoFillBackground(true);
    setWindowTitle(APP_NAME);
    setPalette(p);

    /** Set Tool Bars **/
    /* Add Output Pane */
    auto outputBar = new QToolBar(this);
    outputBar->setFixedHeight(fit::fx(25));
    outputBar->setFloatable(false);
    outputBar->setMovable(false);
    outputBar->addWidget(m_outputPane);
    addToolBar(Qt::BottomToolBarArea, outputBar);

    /* Add Run Pane */
    auto runBar = new QToolBar(this);
    runBar->setFixedHeight(fit::fx(38));
    runBar->setFloatable(false);
    runBar->setMovable(false);
    runBar->addWidget(m_runPane);
    addToolBar(Qt::TopToolBarArea, runBar);

    /* Add Page Switcher Pane */
    auto pageSwitcherBar = new QToolBar(this);
    pageSwitcherBar->setFixedWidth(fit::fx(50));
    pageSwitcherBar->setFloatable(false);
    pageSwitcherBar->setMovable(false);
    pageSwitcherBar->addWidget(m_pageSwitcherPane);
    addToolBar(Qt::LeftToolBarArea, pageSwitcherBar);

    /** Set Dock Widgets **/
    QFont dockTitleFont;
    dockTitleFont.setWeight(QFont::Medium);

    /* Add Properties Pane */
    auto propertiesTitleLabel = new QLabel;
    propertiesTitleLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    propertiesTitleLabel->setText(tr("   Properties"));
    propertiesTitleLabel->setFont(dockTitleFont);

    auto propertiesTitlePinButton = new QToolButton;
    propertiesTitlePinButton->setToolTip(tr("Pin/Unpin pane."));
    propertiesTitlePinButton->setCursor(Qt::PointingHandCursor);
    propertiesTitlePinButton->setIcon(QIcon(":/resources/images/unpin.png"));
    connect(propertiesTitlePinButton, &QToolButton::clicked, this, [] {
        propertiesDockWidget->setFloating(!propertiesDockWidget->isFloating());
    });

    auto propertiesTitleBar = new QToolBar;
    propertiesTitleBar->addWidget(propertiesTitleLabel);
    propertiesTitleBar->addWidget(propertiesTitlePinButton);
    propertiesTitleBar->setStyleSheet(CSS::DesignerPinbar);
    propertiesTitleBar->setIconSize(QSize(fit::fx(11), fit::fx(11)));
    propertiesTitleBar->setFixedHeight(fit::fx(24));

    propertiesDockWidget = new QDockWidget;
    propertiesDockWidget->setTitleBarWidget(propertiesTitleBar);
    propertiesDockWidget->setWidget(m_propertiesPane);
    propertiesDockWidget->setWindowTitle(tr("Properties"));
    propertiesDockWidget->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    addDockWidget(Qt::RightDockWidgetArea, propertiesDockWidget);

    /* Add Forms Pane */
    auto formsTitleLabel = new QLabel;
    formsTitleLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    formsTitleLabel->setText(tr("   Form Navigator"));
    formsTitleLabel->setFont(dockTitleFont);

    auto formsTitlePinButton = new QToolButton;
    formsTitlePinButton->setToolTip(tr("Pin/Unpin pane."));
    formsTitlePinButton->setCursor(Qt::PointingHandCursor);
    formsTitlePinButton->setIcon(QIcon(":/resources/images/unpin.png"));
    connect(formsTitlePinButton, &QToolButton::clicked, this, [] {
        formsDockWidget->setFloating(!formsDockWidget->isFloating());
    });

    auto formsTitleBar = new QToolBar;
    formsTitleBar->addWidget(formsTitleLabel);
    formsTitleBar->addWidget(formsTitlePinButton);
    formsTitleBar->setStyleSheet(CSS::DesignerPinbar);
    formsTitleBar->setIconSize(QSize(fit::fx(11), fit::fx(11)));
    formsTitleBar->setFixedHeight(fit::fx(24));

    formsDockWidget = new QDockWidget;
    formsDockWidget->setTitleBarWidget(formsTitleBar);
    formsDockWidget->setWidget(m_formsPane);
    formsDockWidget->setWindowTitle(tr("Form Navigator"));
    formsDockWidget->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    addDockWidget(Qt::RightDockWidgetArea, formsDockWidget);

    /* Add Toolbox Pane */
    auto toolboxTitleLabel = new QLabel;
    toolboxTitleLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    toolboxTitleLabel->setText(tr("   Toolbox"));
    toolboxTitleLabel->setFont(dockTitleFont);

    auto toolboxTitlePinButton = new QToolButton;
    toolboxTitlePinButton->setToolTip(tr("Pin/Unpin pane."));
    toolboxTitlePinButton->setCursor(Qt::PointingHandCursor);
    toolboxTitlePinButton->setIcon(QIcon(":/resources/images/unpin.png"));
    connect(toolboxTitlePinButton, &QToolButton::clicked, this, [] {
        toolboxDockWidget->setFloating(!toolboxDockWidget->isFloating());
    });

    auto toolboxSettingsButton = new QToolButton;
    toolboxSettingsButton->setToolTip(tr("Toolbox settings."));
    toolboxSettingsButton->setCursor(Qt::PointingHandCursor);
    toolboxSettingsButton->setIcon(QIcon(":/resources/images/settings.png"));
    connect(toolboxSettingsButton, &QToolButton::clicked, this, [=] {
        WindowManager::instance()->show(WindowManager::ToolboxSettings);
    });

    auto toolboxTitleBar = new QToolBar;
    toolboxTitleBar->addWidget(toolboxTitleLabel);
    toolboxTitleBar->addWidget(toolboxSettingsButton);
    toolboxTitleBar->addWidget(toolboxTitlePinButton);
    toolboxTitleBar->setStyleSheet(CSS::DesignerPinbar);
    toolboxTitleBar->setIconSize(QSize(fit::fx(11), fit::fx(11)));
    toolboxTitleBar->setFixedHeight(fit::fx(24));

    toolboxDockWidget = new QDockWidget;
    toolboxDockWidget->setTitleBarWidget(toolboxTitleBar);
    toolboxDockWidget->setWidget(m_toolboxPane);
    toolboxDockWidget->setWindowTitle(tr("Toolbox"));
    toolboxDockWidget->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    addDockWidget(Qt::RightDockWidgetArea, toolboxDockWidget);
    ToolsBackend::instance()->addToolboxTree(m_toolboxPane->toolboxTree());

    /* Add Inspector Pane */
    auto inspectorTitleLabel = new QLabel;
    inspectorTitleLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    inspectorTitleLabel->setText(tr("   Control Inspector"));
    inspectorTitleLabel->setFont(dockTitleFont);

    auto inspectorTitlePinButton = new QToolButton;
    inspectorTitlePinButton->setToolTip(tr("Pin/Unpin pane."));
    inspectorTitlePinButton->setCursor(Qt::PointingHandCursor);
    inspectorTitlePinButton->setIcon(QIcon(":/resources/images/unpin.png"));
    connect(inspectorTitlePinButton, &QToolButton::clicked, this, [] {
        inspectorDockWidget->setFloating(!inspectorDockWidget->isFloating());
    });

    auto inspectorTitleBar = new QToolBar;
    inspectorTitleBar->addWidget(inspectorTitleLabel);
    inspectorTitleBar->addWidget(inspectorTitlePinButton);
    inspectorTitleBar->setStyleSheet(CSS::DesignerPinbar);
    inspectorTitleBar->setIconSize(QSize(fit::fx(11), fit::fx(11)));
    inspectorTitleBar->setFixedHeight(fit::fx(24));

    inspectorDockWidget = new QDockWidget;
    inspectorDockWidget->setTitleBarWidget(inspectorTitleBar);
    inspectorDockWidget->setWidget(m_inspectorPane);
    inspectorDockWidget->setWindowTitle(tr("Control Inspector"));
    inspectorDockWidget->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    addDockWidget(Qt::RightDockWidgetArea, inspectorDockWidget);

    // FIXME
    //    connect(_inspectorPage, SIGNAL(controlClicked(Control*)), _centralWidget, SLOT(onControlClick(Control*)));
    //    connect(_inspectorPage, SIGNAL(controlDoubleClicked(Control*)), _centralWidget, SLOT(onControlDoubleClick(Control*)));

    //    connect(InterpreterBackend::instance(),
    //    QOverload<int, QProcess::ExitStatus>::of(&InterpreterBackend::finished),
    //    [=] (int exitCode, QProcess::ExitStatus exitStatus)
    //    {
    //        auto pane = _centralWidget->outputPane();
    //        auto console = pane->consoleBox();

    //        if (exitStatus == QProcess::CrashExit) {
    //            console->printFormatted(
    //                tr("The process was ended forcefully.\n"),
    //                "#B34B46",
    //                QFont::DemiBold
    //            );
    //        }

    //        console->printFormatted(
    //            ProjectBackend::instance()->name() + tr(" exited with code %1.\n").arg(exitCode),
    //            "#1069C7",
    //            QFont::DemiBold
    //        );

    //        _runButton->setEnabled(true);
    //    });
}

void MainWindow::reset()
{
    // FIXME
    //    m_runPane->reset();
    //    m_centralWidget->qmlEditorView()->clear();

    //    ToolsBackend::instance()->clear();

    //    m_centralWidget->controlScene()->clearSelection();
    //    m_centralWidget->designerScene()->clearSelection();

    //    m_centralWidget->reset();
    //    m_centralWidget->controlScene()->clearScene();
    //    m_centralWidget->designerScene()->clearScene();

    //    m_formsPane->reset();
    //    m_toolboxPane->reset();
    //    m_inspectorPane->reset();
    //    m_propertiesPane->reset();
}
