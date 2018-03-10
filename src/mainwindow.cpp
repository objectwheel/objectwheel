#include <fit.h>
#include <zipper.h>
#include <string.h>
#include <toolboxtree.h>
#include <mainwindow.h>
#include <css.h>
#include <filemanager.h>
#include <projectbackend.h>
#include <userbackend.h>
#include <toolsbackend.h>
#include <savebackend.h>
#include <delayer.h>
#include <formscene.h>
#include <formview.h>
#include <controlview.h>
#include <global.h>
#include <outputpane.h>
#include <qmleditorview.h>
#include <loadingbar.h>
#include <designerwidget.h>
#include <toolboxpane.h>
#include <propertiespane.h>
#include <formspane.h>
#include <inspectorpane.h>
#include <windowmanager.h>
#include <interpreterbackend.h>
#include <flatbutton.h>
#include <consolebox.h>

#include <QSplitter>
#include <QMessageBox>
#include <QToolButton>
#include <QDockWidget>
#include <QtConcurrent>
#include <QtNetwork>
#include <QScreen>
#include <QToolBar>
#include <QLabel>

#if defined(Q_OS_MAC)
#include <mactoolbar.h>
#endif

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent)
{
    _toolBar = new QToolBar;
    _toolboxDockwidget = new QDockWidget;
    _propertiesDockwidget = new QDockWidget;
    _formsDockwidget = new QDockWidget;
    _inspectorDockwidget = new QDockWidget;
    _designerWidget = new DesignerWidget;
    _toolboxPane = new ToolboxPane(this);
    _propertiesPane = new PropertiesPane(this);
    _formsPane = new FormsPane(this);
    _inspectorPage = new InspectorPane(this);
    _loadingBar = new LoadingBar;
    _runButton = new FlatButton;
    _stopButton = new FlatButton;
    _buildsButton = new FlatButton;
    _projectsButton = new FlatButton;

    QPalette p(palette());
    p.setColor(backgroundRole(), "#f0f4f7");
    setStyleSheet("QMainWindow::separator{height: 1px;}");
    setCentralWidget(_designerWidget);
    setAutoFillBackground(true);
    setWindowTitle(APP_NAME);
    setPalette(p);

    /* Add Title Bar */
    _toolBar->setFixedHeight(fit::fx(38));
    _toolBar->setFloatable(false);
    _toolBar->setMovable(false);
    _toolBar->setStyleSheet(QString(
    "QToolBar { border: none; spacing: %1px; background:qlineargradient(spread:pad, "
    "x1:0.5, y1:0, x2:0.5, y2:1, stop:0 #2784E3, stop:1 #1068C6);}").arg(fit::fx(3.5)));
    addToolBar(Qt::TopToolBarArea, _toolBar);

    int lspace = 0;
    #if defined(Q_OS_MAC)
//    auto macToolbar = new MacToolbar(this);
//    _toolBar->setFixedHeight(macToolbar->toolbarHeight());
//    lspace = fit::fx(72);
    #endif

    _runButton->setCursor(Qt::PointingHandCursor);
    _runButton->setToolTip("Run");
    _runButton->setIcon(QIcon(":/resources/images/run.png"));
    _runButton->setFixedSize(fit::fx(QSizeF(38, 24)).toSize());
    _runButton->settings().iconButton = true;
    connect(_runButton, SIGNAL(clicked(bool)),
        SLOT(handleRunButtonClick()));

    _stopButton->setToolTip("Stop");
    // _stopButton->setDisabled(true);
    _stopButton->setCursor(Qt::PointingHandCursor);
    _stopButton->setIcon(QIcon(":/resources/images/stop.png"));
    _stopButton->setFixedSize(fit::fx(QSizeF(38, 24)).toSize());
    _stopButton->settings().iconButton = true;
    connect(_stopButton, SIGNAL(clicked(bool)),
        SLOT(handleStopButtonClick()));
    connect(_stopButton, SIGNAL(doubleClick()),
        SLOT(handleStopButtonDoubleClick()));

    _buildsButton->setToolTip("Get Cloud Build");
    _buildsButton->setCursor(Qt::PointingHandCursor);
    _buildsButton->setIcon(QIcon(":/resources/images/build.png"));
    _buildsButton->setFixedSize(fit::fx(QSizeF(38, 24)).toSize());
    _buildsButton->settings().iconButton = true;
    connect(_buildsButton, SIGNAL(clicked(bool)),
       SLOT(handleBuildsButtonClick()));

    _projectsButton->setToolTip("Show Projects");
    _projectsButton->setCursor(Qt::PointingHandCursor);
    _projectsButton->setIcon(QIcon(":/resources/images/projects.png"));
    _projectsButton->setFixedSize(fit::fx(QSizeF(38, 24)).toSize());
    _projectsButton->settings().iconButton = true;
    connect(_projectsButton, SIGNAL(clicked(bool)),
       SLOT(handleProjectsButtonClick()));

    _loadingBar->setFixedSize(fit::fx(QSizeF(481, 24)).toSize());

    auto spc = new QWidget;
    spc->setFixedWidth(lspace);
    auto spc2 = new QWidget;
    spc2->setFixedWidth(0);
    auto spc3 = new QWidget;
    spc3->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    auto spc4 = new QWidget;
    spc4->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    auto spc5 = new QWidget;
    spc5->setFixedWidth(0);
    auto spc6 = new QWidget;
    spc6->setFixedWidth(lspace); // lspace + 2*btnwidth + 3*5 = 3*5 + 2*btnwidth + x
    auto spc7 = new QWidget;
    spc7->setFixedWidth(0);

    _toolBar->addWidget(spc5);
    _toolBar->insertWidget(_toolBar->actions().first(), _buildsButton);
    _toolBar->insertWidget(_toolBar->actions().first(), spc7);
    _toolBar->insertWidget(_toolBar->actions().first(), _projectsButton);
    _toolBar->insertWidget(_toolBar->actions().first(), spc6);
    _toolBar->insertWidget(_toolBar->actions().first(), spc4);
    _toolBar->insertWidget(_toolBar->actions().first(), _loadingBar);
    _toolBar->insertWidget(_toolBar->actions().first(), spc3);
    _toolBar->insertWidget(_toolBar->actions().first(), _stopButton);
    _toolBar->insertWidget(_toolBar->actions().first(), spc2);
    _toolBar->insertWidget(_toolBar->actions().first(), _runButton);
    _toolBar->insertWidget(_toolBar->actions().first(), spc);

    QFont f;
    f.setWeight(QFont::Medium);
    /*** PROPERTIES DOCK WIDGET ***/
    QLabel* label = new QLabel;
    label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    label->setText("   Properties");
    label->setFont(f);

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

    connect(_toolboxPane->toolboxTree()->indicatorButton(),
      &FlatButton::clicked, this, [=] {
        auto splitter = designerWidget()->splitter();
        auto controlView = designerWidget()->controlView();
        auto formView = designerWidget()->formView();
        auto qmlEditorView = designerWidget()->qmlEditorView();
        auto sizes = splitter->sizes();
        QSize size;
        if (formView->isVisible())
            size = formView->size();
        else if (qmlEditorView->isVisible())
            size = qmlEditorView->size();
        else
            size = controlView->size();
        sizes[splitter->indexOf(controlView)] = size.height();
        auto previousControl = designerWidget()->controlScene()->mainControl();
        if (previousControl)
            previousControl->deleteLater();
        auto url = _toolboxPane->toolboxTree()->urls(_toolboxPane->toolboxTree()->currentItem())[0];
        auto control = SaveBackend::instance()->exposeControl(dname(dname(url.toLocalFile())), ControlGui);
        designerWidget()->controlScene()->setMainControl(control);
        designerWidget()->setMode(ControlGui);

        for (auto childControl : control->childControls())
            childControl->refresh();
        splitter->setSizes(sizes);
        // FIXME: Close docs on qml editor whenever a Control GUI Editor subject changed
    });

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

    _loadingBar->setText(ProjectBackend::instance()->name() + ": <b>Ready</b>  |  Welcome to Objectwheel");
    connect(ProjectBackend::instance(), &ProjectBackend::started, [=] {
        _loadingBar->setText(ProjectBackend::instance()->name() + ": <b>Ready</b>  |  Welcome to Objectwheel");
    });

//    connect(SaveBackend::instance(), SIGNAL(doneExecuter(QString)), _loadingBar, SLOT(done(QString))); //TODO
//    connect(SaveBackend::instance(), SIGNAL(busyExecuter(int, QString)), _loadingBar, SLOT(busy(int,QString))); //TODO

    QToolBar* toolbar4 = new QToolBar;
    toolbar4->addWidget(label4);
    toolbar4->addWidget(pinButton4);
    toolbar4->setStyleSheet(CSS::DesignerPinbar);
    toolbar4->setIconSize(QSize(fit::fx(11), fit::fx(11)));
    toolbar4->setFixedHeight(fit::fx(24));

    connect(_inspectorPage, SIGNAL(controlClicked(Control*)),
      _designerWidget, SLOT(handleControlClick(Control*)));
    connect(_inspectorPage, SIGNAL(controlDoubleClicked(Control*)),
      _designerWidget, SLOT(handleControlDoubleClick(Control*)));

    _inspectorDockwidget->setTitleBarWidget(toolbar4);
    _inspectorDockwidget->setWidget(_inspectorPage);
    _inspectorDockwidget->setWindowTitle("Control Inspector");
    _inspectorDockwidget->setFeatures(QDockWidget::DockWidgetMovable |
                                     QDockWidget::DockWidgetFloatable);

    connect(InterpreterBackend::instance(),
    QOverload<int, QProcess::ExitStatus>::of(&InterpreterBackend::finished),
    [=] (int exitCode, QProcess::ExitStatus exitStatus)
    {
        auto pane = _designerWidget->outputPane();
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

    setDockOptions(dockOptions() ^ QMainWindow::AnimatedDocks);
    addDockWidget(Qt::LeftDockWidgetArea, _toolboxDockwidget);
    addDockWidget(Qt::LeftDockWidgetArea, _formsDockwidget);
    addDockWidget(Qt::RightDockWidgetArea, _inspectorDockwidget);
    addDockWidget(Qt::RightDockWidgetArea, _propertiesDockwidget);
}

DesignerWidget* MainWindow::designerWidget()
{
    return _designerWidget;
}

void MainWindow::clear()
{
    handleStopButtonClick();

    designerWidget()->qmlEditorView()->clear();

    ToolsBackend::instance()->clear();

    designerWidget()->controlScene()->clearSelection();
    designerWidget()->formScene()->clearSelection();

    designerWidget()->clear();
    designerWidget()->controlScene()->clearScene();
    designerWidget()->formScene()->clearScene();

    _formsPane->clear();
    _toolboxPane->clear();
    _inspectorPage->clear();
    _propertiesPane->clear();
}

void MainWindow::handleStopButtonClick()
{
    InterpreterBackend::instance()->terminate();
    _loadingBar->busy(0, ProjectBackend::instance()->name() + ": <b>Stopped</b>  |  Finished at " + QTime::currentTime().toString());
}

void MainWindow::handleStopButtonDoubleClick()
{
    InterpreterBackend::instance()->kill();
    _loadingBar->busy(0, ProjectBackend::instance()->name() + ": <b>Stopped forcefully</b>  |  Finished at " + QTime::currentTime().toString());
}

void MainWindow::handleRunButtonClick()
{
    auto pane = _designerWidget->outputPane();
    auto console = pane->consoleBox();

    console->fade();
    if (!console->isClean())
        console->print("\n");
    console->printFormatted(
        tr("Starting ") + ProjectBackend::instance()->name() + "...\n",
        "#1069C7",
        QFont::DemiBold
    );
    console->scrollToEnd();

    InterpreterBackend::instance()->run();

    _runButton->setDisabled(true);

//    switch (error.type) {
//        case CommonError:
//            box.setInformativeText("Database corrupted, change your application skin. "
//                                   "If it doesn't work, contact to support for further help.");
//            _loadingBar->error(ProjectBackend::instance()->name() + ": <b>Error</b>  |  Stopped at " + QTime::currentTime().toString());
//            box.exec();
//            break;

//        case ChildIsWindowError:
//            box.setInformativeText("Child controls can not be a 'Window' (or derived) type."
//                                   " Only forms could be 'Window' type.");
//            _loadingBar->error(ProjectBackend::instance()->name() + ": <b>Error</b>  |  Stopped at " + QTime::currentTime().toString());
//            box.exec();
//            break;

//        case MasterIsNonGui:
//            box.setInformativeText("Master controls can not be a non-ui control (such as Timer or QtObject).");
//            _loadingBar->error(ProjectBackend::instance()->name() + ": <b>Error</b>  |  Stopped at " + QTime::currentTime().toString());
//            box.exec();
//            break;

//        case FormIsNonGui:
//            box.setInformativeText("Forms can not be a non-ui control (such as Timer or QtObject)."
//                                   "Check your forms and make sure they are some 'Window' or 'Item' derived type.");
//            _loadingBar->error(ProjectBackend::instance()->name() + ": <b>Error</b>  |  Stopped at " + QTime::currentTime().toString());
//            box.exec();
//            break;

//        case MainFormIsntWindowError:
//            box.setInformativeText("Main form has to be a 'Window' derived type. "
//                                   "Please change its type to a 'Window' derived class.");
//            _loadingBar->error(ProjectBackend::instance()->name() + ": <b>Error</b>  |  Stopped at " + QTime::currentTime().toString());
//            box.exec();
//            break;

//        case MultipleWindowsForMobileError:
//            box.setInformativeText("Mobile applications can not contain multiple windows. "
//                                   "Please either change the type of secondary windows' type to a non 'Window' derived class, "
//                                   "or change your application skin to something else (Desktop for instance) by changing the skin of main form.");
//            _loadingBar->error(ProjectBackend::instance()->name() + ": <b>Error</b>  |  Stopped at " + QTime::currentTime().toString());
//            box.exec();
//            break;

//        case NoMainForm:
//            box.setInformativeText("There is no main application window. Probably database has corrupted, "
//                                   "please contact to support, or start a new project over.");
//            _loadingBar->error(ProjectBackend::instance()->name() + ": <b>Error</b>  |  Stopped at " + QTime::currentTime().toString());
//            box.exec();
//            break;

//        case CodeError: {
//            box.setInformativeText(QString("Following control has some errors: <b>%1</b>").
//                                   arg(error.id));
//            QString detailedText;
//            for (auto err : error.errors)
//                detailedText += QString("Line %1, column %2: %3").
//                                arg(err.line()).arg(err.column()).arg(err.description());
//            box.setDetailedText(detailedText);
//            _loadingBar->error(ProjectBackend::instance()->name() + ": <b>Error</b>  |  Stopped at " + QTime::currentTime().toString());
//            box.exec();
//            break;
//        }

//        case Stopped: {
//            _loadingBar->error(ProjectBackend::instance()->name() + ": <b>Stopped</b>  |  Finished at " + QTime::currentTime().toString());
//        }

//        default:
//            break;
//    }
}

void MainWindow::handleBuildsButtonClick()
{
    WindowManager::instance()->show(WindowManager::Builds);
}

void MainWindow::handleProjectsButtonClick()
{
    WindowManager::instance()->show(WindowManager::Welcome);
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
