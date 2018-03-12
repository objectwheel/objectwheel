#include <mainwindow.h>
#include <fit.h>
#include <runpane.h>
#include <outputpane.h>
#include <toolboxpane.h>
#include <propertiespane.h>
#include <formspane.h>
#include <inspectorpane.h>
#include <pageswitcherpane.h>
#include <centralwidget.h>

#include <QToolBar>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent)
  , m_outputPane(new OutputPane)
  , m_runPane(new RunPane(m_outputPane->consoleBox()))
  , m_pageSwitcherPane(new PageSwitcherPane)
  , m_toolboxPane(new ToolboxPane)
  , m_propertiesPane(new PropertiesPane)
  , m_formsPane(new FormsPane)
  , m_inspectorPage(new InspectorPane)
  , m_centralWidget(new CentralWidget)

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
    auto propertiesLabel = new QLabel;
    propertiesLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    propertiesLabel->setText(tr("   Properties"));
    propertiesLabel->setFont(dockTitleFont);

    QToolButton* pinButton = new QToolButton;
    pinButton->setToolTip("Pin/Unpin pane.");
    pinButton->setCursor(Qt::PointingHandCursor);
    pinButton->setIcon(QIcon(":/resources/images/unpin.png"));
    connect(pinButton, &QToolButton::clicked, this, [=]{
        _propertiesDockwidget->setFloating(!_propertiesDockwidget->isFloating());
    });

    QToolBar* toolbar = new QToolBar;
    toolbar->addWidget(label);
    toolbar->addWidget(pinButton);
    toolbar->setStyleSheet(CSS::DesignerPinbar);
    toolbar->setIconSize(QSize(fit::fx(11), fit::fx(11)));
    toolbar->setFixedHeight(fit::fx(24));

    _propertiesDockwidget->setTitleBarWidget(toolbar);
    _propertiesDockwidget->setWidget(_propertiesPane);
    _propertiesDockwidget->setWindowTitle("Properties");
    _propertiesDockwidget->setFeatures(QDockWidget::DockWidgetMovable |
                                      QDockWidget::DockWidgetFloatable);

    /*** FORMS DOCK WIDGET ***/
    QLabel* label2 = new QLabel;
    label2->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    label2->setText("   Form Navigator");
    label2->setFont(f);

    QToolButton* pinButton2 = new QToolButton;
    pinButton2->setToolTip("Pin/Unpin pane.");
    pinButton2->setCursor(Qt::PointingHandCursor);
    pinButton2->setIcon(QIcon(":/resources/images/unpin.png"));
    connect(pinButton2, &QToolButton::clicked, this, [=]{
        _formsDockwidget->setFloating(!_formsDockwidget->isFloating());
    });

    QToolBar* toolbar2 = new QToolBar;
    toolbar2->addWidget(label2);
    toolbar2->addWidget(pinButton2);
    toolbar2->setStyleSheet(CSS::DesignerPinbar);
    toolbar2->setIconSize(QSize(fit::fx(11), fit::fx(11)));
    toolbar2->setFixedHeight(fit::fx(24));

    _formsDockwidget->setTitleBarWidget(toolbar2);
    _formsDockwidget->setWidget(_formsPane);
    _formsDockwidget->setWindowTitle("Forms");
    _formsDockwidget->setFeatures(QDockWidget::DockWidgetMovable |
                                 QDockWidget::DockWidgetFloatable);

    /*** TOOLBOX DOCK WIDGET ***/
    QLabel* label3 = new QLabel;
    label3->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    label3->setText("   Toolbox");
    label3->setFont(f);

    QToolButton* pinButton3 = new QToolButton;
    pinButton3->setToolTip("Pin/Unpin pane.");
    pinButton3->setCursor(Qt::PointingHandCursor);
    pinButton3->setIcon(QIcon(":/resources/images/unpin.png"));
    connect(pinButton3, &QToolButton::clicked, this, [=]{
        _toolboxDockwidget->setFloating(!_toolboxDockwidget->isFloating());
    });

    QToolButton* toolboxSettingsButton = new QToolButton;
    toolboxSettingsButton->setToolTip("Toolbox settings.");
    toolboxSettingsButton->setCursor(Qt::PointingHandCursor);
    toolboxSettingsButton->setIcon(QIcon(":/resources/images/settings.png"));
    connect(toolboxSettingsButton, &QToolButton::clicked, this, [=] {
        WindowManager::instance()->show(WindowManager::ToolboxSettings);
    });

    QToolBar* toolbar3 = new QToolBar;
    toolbar3->addWidget(label3);
    toolbar3->addWidget(toolboxSettingsButton);
    toolbar3->addWidget(pinButton3);
    toolbar3->setStyleSheet(CSS::DesignerPinbar);
    toolbar3->setIconSize(QSize(fit::fx(11), fit::fx(11)));
    toolbar3->setFixedHeight(fit::fx(24));

    _toolboxDockwidget->setTitleBarWidget(toolbar3);
    _toolboxDockwidget->setWidget(_toolboxPane);
    _toolboxDockwidget->setWindowTitle("Toolbox");
    _toolboxDockwidget->setFeatures(
        QDockWidget::DockWidgetMovable |
        QDockWidget::DockWidgetFloatable
    );

    ToolsBackend::instance()->addToolboxTree(_toolboxPane->toolboxTree());

    /*** INSPECTOR DOCK WIDGET ***/
    QLabel* label4 = new QLabel;
    label4->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    label4->setText("   Control Inspector");
    label4->setFont(f);

    QToolButton* pinButton4 = new QToolButton;
    pinButton4->setToolTip("Pin/Unpin pane.");
    pinButton4->setCursor(Qt::PointingHandCursor);
    pinButton4->setIcon(QIcon(":/resources/images/unpin.png"));
    connect(pinButton4, &QToolButton::clicked, this, [=]{
        _inspectorDockwidget->setFloating(!_inspectorDockwidget->isFloating());
    });

    QToolBar* toolbar4 = new QToolBar;
    toolbar4->addWidget(label4);
    toolbar4->addWidget(pinButton4);
    toolbar4->setStyleSheet(CSS::DesignerPinbar);
    toolbar4->setIconSize(QSize(fit::fx(11), fit::fx(11)));
    toolbar4->setFixedHeight(fit::fx(24));

    connect(_inspectorPage, SIGNAL(controlClicked(Control*)),
      _centralWidget, SLOT(onControlClick(Control*)));
    connect(_inspectorPage, SIGNAL(controlDoubleClicked(Control*)),
      _centralWidget, SLOT(onControlDoubleClick(Control*)));

    _inspectorDockwidget->setTitleBarWidget(toolbar4);
    _inspectorDockwidget->setWidget(_inspectorPage);
    _inspectorDockwidget->setWindowTitle("Control Inspector");
    _inspectorDockwidget->setFeatures(QDockWidget::DockWidgetMovable |
                                     QDockWidget::DockWidgetFloatable);

    connect(InterpreterBackend::instance(),
    QOverload<int, QProcess::ExitStatus>::of(&InterpreterBackend::finished),
    [=] (int exitCode, QProcess::ExitStatus exitStatus)
    {
        auto pane = _centralWidget->outputPane();
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

        _runButton->setEnabled(true);
    });

    addDockWidget(Qt::LeftDockWidgetArea, _toolboxDockwidget);
    addDockWidget(Qt::LeftDockWidgetArea, _formsDockwidget);
    addDockWidget(Qt::RightDockWidgetArea, _inspectorDockwidget);
    addDockWidget(Qt::RightDockWidgetArea, _propertiesDockwidget);
}

CentralWidget* MainWindow::centralWidget()
{
    return _centralWidget;
}

void MainWindow::clear()
{
    runPange()->reset();
    centralWidget()->qmlEditorView()->clear();

    ToolsBackend::instance()->clear();

    centralWidget()->controlScene()->clearSelection();
    centralWidget()->formScene()->clearSelection();

    centralWidget()->clear();
    centralWidget()->controlScene()->clearScene();
    centralWidget()->formScene()->clearScene();

    _formsPane->clear();
    _toolboxPane->clear();
    _inspectorPage->clear();
    _propertiesPane->clear();
}


//void MainWindow::setupManagers()
//{
//    auto userManager = new UserManager(this);
//    Q_UNUSED(userManager);
//    new ProjectManager(this);
//    ProjectManager::setMainWindow(this);
//    new SaveBackend(this);
//    new QmlPreviewer(this);

//    connect(qApp, SIGNAL(aboutToQuit()),
//      SLOT(cleanupObjectwheel()));

//    auto ret = QtConcurrent::run(&UserManager::tryAutoLogin);
//    Delayer::delay(&ret, &QFuture<bool>::isRunning);
//    if (ret.result()) {
//        ProjectsWidget::refreshProjectList();
////        wM->progress()->hide();
////        wM->show(WindowManager::Projects);
//    } else {
////        wM->progress()->hide();
////        wM->show(WindowManager::Login);
//    }
//}

//void MainWindow::on_secureExitButton_clicked()
//{
//    SplashScreen::setText("Stopping user session");
//    SplashScreen::show(true);
//    UserManager::clearAutoLogin();
//    auto ret = QtConcurrent::run(&UserManager::stop);
//    Delayer::delay(&ret, &QFuture<void>::isRunning);
//    SplashScreen::hide();
//    SplashScreen::setText("Loading");
//    SceneManager::show("loginScene", SceneManager::ToLeft);
//}
