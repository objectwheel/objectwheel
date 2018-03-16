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
#include <qmlcodeeditorwidget.h>
#include <exposerbackend.h>
#include <interpreterbackend.h>
#include <projectbackend.h>
#include <consolebox.h>

#include <QProcess>
#include <QToolBar>
#include <QLabel>
#include <QToolButton>
#include <QDockWidget>
#include <QLayout>

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
    p.setColor(backgroundRole(), "#f0f4f7");
    setPalette(p);

    setWindowTitle(APP_NAME);
    setAutoFillBackground(true);
    setCentralWidget(m_centralWidget);
    setContextMenuPolicy(Qt::NoContextMenu);
    setStyleSheet("QMainWindow::separator{ height: 1px; }");

    ExposerBackend::instance()->init(m_centralWidget->designerWidget()->designerScene());

    /** Set Tool Bars **/
    /* Add Run Pane */
    auto runBar = new QToolBar;
    runBar->setOrientation(Qt::Horizontal);
    runBar->setFixedHeight(fit::fx(38));
    runBar->setFloatable(false);
    runBar->setMovable(false);
    runBar->setWindowTitle(tr("Run Bar"));
    runBar->setStyleSheet("border: none");
    runBar->setContentsMargins(0, 0, 0, 0);
    runBar->layout()->setContentsMargins(0, 0, 0, 0);
    runBar->addWidget(m_runPane);
    m_runPane->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    addToolBar(Qt::TopToolBarArea, runBar);

    /* Add Page Switcher Pane */
    auto pageSwitcherBar = new QToolBar;
    pageSwitcherBar->setOrientation(Qt::Vertical);
    pageSwitcherBar->setFixedWidth(fit::fx(65));
    pageSwitcherBar->setWindowFlags(pageSwitcherBar->windowFlags() | Qt::WindowStaysOnTopHint);
    pageSwitcherBar->setFloatable(false);
    pageSwitcherBar->setMovable(false);
    pageSwitcherBar->setWindowTitle(tr("Page Bar"));
    pageSwitcherBar->setStyleSheet("border: none");
    pageSwitcherBar->setContentsMargins(0, 0, 0, 0);
    pageSwitcherBar->layout()->setContentsMargins(0, 0, 0, 0);
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
    inspectorTitleLabel->setStyleSheet("color: #2E3A41");
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
    propertiesTitleLabel->setStyleSheet("color: #2E3A41");
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
    toolboxTitleLabel->setStyleSheet("color: #2E3A41");
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
    toolboxDockWidget->setStyleSheet("QDockWidget { border: none }");
    toolboxDockWidget->setTitleBarWidget(toolboxTitleBar);
    toolboxDockWidget->setWidget(m_toolboxPane);
    toolboxDockWidget->setWindowTitle(tr("Toolbox"));
    toolboxDockWidget->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    addDockWidget(Qt::LeftDockWidgetArea, toolboxDockWidget);
    ToolsBackend::instance()->addToolboxTree(m_toolboxPane->toolboxTree());

    /* Add Forms Pane */
    auto formsTitleLabel = new QLabel;
    formsTitleLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    formsTitleLabel->setText(tr("   Form Navigator"));
    formsTitleLabel->setStyleSheet("color: #2E3A41");
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
    formsDockWidget->setStyleSheet("QDockWidget { border: none }");
    formsDockWidget->setTitleBarWidget(formsTitleBar);
    formsDockWidget->setWidget(m_formsPane);
    formsDockWidget->setWindowTitle(tr("Form Navigator"));
    formsDockWidget->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    addDockWidget(Qt::LeftDockWidgetArea, formsDockWidget);

    connect(m_pageSwitcherPane, SIGNAL(buildsActivated()), SLOT(hideDocks()));
    connect(m_pageSwitcherPane, SIGNAL(designerActivated()), SLOT(showDocks()));
    connect(m_pageSwitcherPane, SIGNAL(splitViewActivated()), SLOT(showDocks()));
    connect(m_pageSwitcherPane, SIGNAL(documentsActivated()), SLOT(hideDocks()));
    connect(m_pageSwitcherPane, SIGNAL(qmlCodeEditorActivated()), SLOT(hideDocks()));
    connect(m_pageSwitcherPane, SIGNAL(projectOptionsActivated()), SLOT(hideDocks()));
    connect(m_pageSwitcherPane, SIGNAL(currentPageChanged(Pages)), m_centralWidget, SLOT(setCurrentPage(Pages)));

    connect(m_centralWidget->qmlCodeEditorWidget(), &QmlCodeEditorWidget::openControlCountChanged, [=] {
        if (m_centralWidget->qmlCodeEditorWidget()->openControlCount() > 0 && !m_pageSwitcherPane->isPageEnabled(Page_QmlCodeEditor)) {
            m_pageSwitcherPane->setPageEnabled(Page_QmlCodeEditor);
            m_pageSwitcherPane->setPageEnabled(Page_SplitView);
        }else if (m_centralWidget->qmlCodeEditorWidget()->openControlCount() <= 0) {
            m_pageSwitcherPane->setPageDisabled(Page_QmlCodeEditor);
            m_pageSwitcherPane->setPageDisabled(Page_SplitView);
            m_pageSwitcherPane->setCurrentPage(Page_Designer);
        }

        if (m_centralWidget->qmlCodeEditorWidget()->openControlCount() > 0
            && m_pageSwitcherPane->currentPage() != Page_SplitView)
            m_pageSwitcherPane->setCurrentPage(Page_QmlCodeEditor);
    });

    connect(m_inspectorPane, SIGNAL(controlClicked(Control*)), m_centralWidget->designerWidget(), SLOT(onControlClick(Control*)));
    connect(m_inspectorPane, SIGNAL(controlDoubleClicked(Control*)), m_centralWidget->designerWidget(), SLOT(onControlDoubleClick(Control*)));
    connect(m_formsPane, SIGNAL(currentFormChanged()), m_inspectorPane, SLOT(refresh()));

    connect(InterpreterBackend::instance(),
    QOverload<int, QProcess::ExitStatus>::of(&InterpreterBackend::finished),
    [=] (int exitCode, QProcess::ExitStatus exitStatus)
    {
        auto pane = m_centralWidget->outputPane();
        auto console = pane->consoleBox();

        if (exitStatus == QProcess::CrashExit) {
            console->printFormatted(
                tr("The process was ended forcefully.\n"),
                "#B34B46",
                QFont::DemiBold
            );
        }

        console->printFormatted(
            ProjectBackend::instance()->name() + tr(" exited with code %1.\n").arg(exitCode),
            "#1069C7",
            QFont::DemiBold
        );
    });
}

void MainWindow::reset()
{
    ToolsBackend::instance()->clear();
    m_runPane->reset();
    m_centralWidget->reset();
    m_formsPane->reset();
    m_toolboxPane->reset();
    m_inspectorPane->reset();
    m_propertiesPane->reset();
    m_pageSwitcherPane->reset();
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
