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
#include <previewbackend.h>
#include <formview.h>
#include <controlview.h>
#include <global.h>
#include <outputwidget.h>
#include <qmleditorview.h>
#include <loadingbar.h>
#include <designerwidget.h>
#include <toolboxpane.h>
#include <propertiespane.h>
#include <formspane.h>
#include <inspectorpane.h>
#include <windowmanager.h>
#include <executivewidget.h>

#include <QSplitter>
#include <QMessageBox>
#include <QToolButton>
#include <QDockWidget>
#include <QtConcurrent>
#include <QtNetwork>
#include <QApplication>
#include <QScreen>

#if defined(Q_OS_MAC)
#include <mactoolbar.h>
#endif

#define pS (QApplication::primaryScreen())

static bool stopper = false;

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent)
{
    _titleBar = new QToolBar;
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
    _executiveWidget = nullptr;

    QPalette p(palette());
    p.setColor(backgroundRole(), "#E0E4E7");
    setStyleSheet("QMainWindow::separator{height: 0px;}");
    setCentralWidget(_designerWidget);
    setAutoFillBackground(true);
    setWindowTitle(APP_NAME);
    setPalette(p);

    /* Add Title Bar */
    addToolBar(Qt::TopToolBarArea, _titleBar);
    _titleBar->setFixedHeight(fit::fx(38));
    _titleBar->setFloatable(false);
    _titleBar->setMovable(false);
    _titleBar->setStyleSheet(QString(
    "QToolBar{border: none; spacing:5px; background:qlineargradient(spread:pad, "
    "x1:0.5, y1:0, x2:0.5, y2:1, stop:0 #2784E3, stop:1 #1068C6);}"));

    int lspace = 0, btnwidth = 48 / pS->devicePixelRatio();
    #if defined(Q_OS_MAC)
    auto macToolbar = new MacToolbar(this);
    _titleBar->setFixedHeight(macToolbar->toolbarHeight());
    lspace = fit::fx(72);
    #endif

    _runButton->setCursor(Qt::PointingHandCursor);
    _runButton->setToolTip("Run");
    _runButton->setIcon(QIcon(":/resources/images/run.png"));
    _runButton->setFixedSize(QSize(76, 48) / pS->devicePixelRatio());
    _runButton->setIconButton(true);
    connect(_runButton, SIGNAL(clicked(bool)),
        SLOT(handleRunButtonClick()));

    _stopButton->setToolTip("Stop");
    _stopButton->setDisabled(true);
    _stopButton->setCursor(Qt::PointingHandCursor);
    _stopButton->setIcon(QIcon(":/resources/images/stop.png"));
    _stopButton->setFixedSize(QSize(76, 48) / pS->devicePixelRatio());
    _stopButton->setIconButton(true);
    connect(_stopButton, SIGNAL(clicked(bool)),
        SLOT(handleStopButtonClick()));

    _buildsButton->setToolTip("Get Cloud Build");
    _buildsButton->setCursor(Qt::PointingHandCursor);
    _buildsButton->setIcon(QIcon(":/resources/images/build.png"));
    _buildsButton->setFixedSize(QSize(76, 48) / pS->devicePixelRatio());
    _buildsButton->setIconButton(true);
    connect(_buildsButton, SIGNAL(clicked(bool)),
       SLOT(handleBuildsButtonClick()));

    _loadingBar->setFixedSize(QSize(962, 48) / pS->devicePixelRatio());

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
    spc6->setFixedWidth(5 + btnwidth + lspace); // lspace + 2*btnwidth + 3*5 = 2*5 + btnwidth + x

    _titleBar->insertWidget(_titleBar->actions().first(), spc5);
    _titleBar->insertWidget(_titleBar->actions().first(), _buildsButton);
    _titleBar->insertWidget(_titleBar->actions().first(), spc6);
    _titleBar->insertWidget(_titleBar->actions().first(), spc4);
    _titleBar->insertWidget(_titleBar->actions().first(), _loadingBar);
    _titleBar->insertWidget(_titleBar->actions().first(), spc3);
    _titleBar->insertWidget(_titleBar->actions().first(), _stopButton);
    _titleBar->insertWidget(_titleBar->actions().first(), spc2);
    _titleBar->insertWidget(_titleBar->actions().first(), _runButton);
    _titleBar->insertWidget(_titleBar->actions().first(), spc);

//    QTimer* cct = new QTimer;
//    connect(cct, &QTimer::timeout, [=] {
//        static int i = 0;
//        if (i == 0) {
//            _loadingBar->busy(30, "Loading..");
//        } else if (i == 1) {
//            _loadingBar->busy(60, "Loading..2323...");
//        } else if (i == 2) {
//            _loadingBar->busy(80, "Loading..23232323..");
//        } if (i == 3) {
//            _loadingBar->error("Kraal. Bitti.");
//        }
//        i++;
//    });
//    QTimer::singleShot(5000, [=]{cct->start(2000);});

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
    toolbar->setFixedHeight(fit::fx(22.8));

    _propertiesDockwidget->setTitleBarWidget(toolbar);
    _propertiesDockwidget->setWidget(_propertiesPane);
    _propertiesDockwidget->setWindowTitle("Properties");
    _propertiesDockwidget->setAttribute(Qt::WA_TranslucentBackground);
    _propertiesDockwidget->setFeatures(QDockWidget::DockWidgetMovable |
                                      QDockWidget::DockWidgetFloatable);

    /*** FORMS DOCK WIDGET ***/
    QLabel* label2 = new QLabel;
    label2->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    label2->setText("   Forms");
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
    toolbar2->setFixedHeight(fit::fx(22.8));

    _formsDockwidget->setTitleBarWidget(toolbar2);
    _formsDockwidget->setWidget(_formsPane);
    _formsDockwidget->setWindowTitle("Forms");
    _formsDockwidget->setAttribute(Qt::WA_TranslucentBackground);
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
    toolbar3->setFixedHeight(fit::fx(22.8));

    _toolboxDockwidget->setTitleBarWidget(toolbar3);
    _toolboxDockwidget->setWidget(_toolboxPane);
    _toolboxDockwidget->setWindowTitle("Toolbox");
    _toolboxDockwidget->setAttribute(Qt::WA_TranslucentBackground);
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
        auto control = SaveBackend::exposeControl(dname(dname(url.toLocalFile())), ControlGui);
        designerWidget()->controlScene()->setMainControl(control);
        designerWidget()->setMode(ControlGui);
        control->refresh();
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

    connect(SaveBackend::instance(), SIGNAL(doneExecuter(QString)), _loadingBar, SLOT(done(QString)));
    connect(SaveBackend::instance(), SIGNAL(busyExecuter(int, QString)), _loadingBar, SLOT(busy(int,QString)));

    QToolBar* toolbar4 = new QToolBar;
    toolbar4->addWidget(label4);
    toolbar4->addWidget(pinButton4);
    toolbar4->setStyleSheet(CSS::DesignerPinbar);
    toolbar4->setIconSize(QSize(fit::fx(11), fit::fx(11)));
    toolbar4->setFixedHeight(fit::fx(22.8));

    connect(_inspectorPage, SIGNAL(controlClicked(Control*)),
      _designerWidget, SLOT(handleControlClick(Control*)));
    connect(_inspectorPage, SIGNAL(controlDoubleClicked(Control*)),
      _designerWidget, SLOT(handleControlDoubleClick(Control*)));

    _inspectorDockwidget->setTitleBarWidget(toolbar4);
    _inspectorDockwidget->setWidget(_inspectorPage);
    _inspectorDockwidget->setWindowTitle("Control Inspector");
    _inspectorDockwidget->setAttribute(Qt::WA_TranslucentBackground);
    _inspectorDockwidget->setFeatures(QDockWidget::DockWidgetMovable |
                                     QDockWidget::DockWidgetFloatable);

    addDockWidget(Qt::LeftDockWidgetArea, _toolboxDockwidget);
    addDockWidget(Qt::LeftDockWidgetArea, _formsDockwidget);
    addDockWidget(Qt::RightDockWidgetArea, _inspectorDockwidget);
    addDockWidget(Qt::RightDockWidgetArea, _propertiesDockwidget);
}

DesignerWidget* MainWindow::designerWidget()
{
    return _designerWidget;
}

void MainWindow::cleanupObjectwheel()
{
    while(SaveBackend::parserWorking())
        Delayer::delay(100);

    UserBackend::instance()->stop();

    qApp->processEvents();
}

void MainWindow::handleStopButtonClick()
{
    stopper = true;
    if (_executiveWidget) {
        _executiveWidget->stop();
        _loadingBar->busy(0, ProjectBackend::instance()->name() + ": <b>Stopped</b>  |  Finished at " + QTime::currentTime().toString());
    }
}

void MainWindow::handleRunButtonClick()
{
    if (_executiveWidget)
        handleStopButtonClick();

    _stopButton->setEnabled(true);
    _stopButton->setEnabled(true);
    stopper = false;
    _executiveWidget = new ExecutiveWidget;
    ExecError error = SaveBackend::execProject(&stopper, _executiveWidget);

    QMessageBox box;
    box.setText("<b>Some went wrong.</b>");
    box.setStandardButtons(QMessageBox::Ok);
    box.setDefaultButton(QMessageBox::Ok);
    box.setIcon(QMessageBox::Warning);
    switch (error.type) {
        case CommonError:
            box.setInformativeText("Database corrupted, change your application skin. "
                                   "If it doesn't work, contact to support for further help.");
            _loadingBar->error(ProjectBackend::instance()->name() + ": <b>Error</b>  |  Stopped at " + QTime::currentTime().toString());
            box.exec();
            break;

        case ChildIsWindowError:
            box.setInformativeText("Child controls can not be a 'Window' (or derived) type."
                                   " Only forms could be 'Window' type.");
            _loadingBar->error(ProjectBackend::instance()->name() + ": <b>Error</b>  |  Stopped at " + QTime::currentTime().toString());
            box.exec();
            break;

        case MasterIsNonGui:
            box.setInformativeText("Master controls can not be a non-ui control (such as Timer or QtObject).");
            _loadingBar->error(ProjectBackend::instance()->name() + ": <b>Error</b>  |  Stopped at " + QTime::currentTime().toString());
            box.exec();
            break;

        case FormIsNonGui:
            box.setInformativeText("Forms can not be a non-ui control (such as Timer or QtObject)."
                                   "Check your forms and make sure they are some 'Window' or 'Item' derived type.");
            _loadingBar->error(ProjectBackend::instance()->name() + ": <b>Error</b>  |  Stopped at " + QTime::currentTime().toString());
            box.exec();
            break;

        case MainFormIsntWindowError:
            box.setInformativeText("Main form has to be a 'Window' derived type. "
                                   "Please change its type to a 'Window' derived class.");
            _loadingBar->error(ProjectBackend::instance()->name() + ": <b>Error</b>  |  Stopped at " + QTime::currentTime().toString());
            box.exec();
            break;

        case MultipleWindowsForMobileError:
            box.setInformativeText("Mobile applications can not contain multiple windows. "
                                   "Please either change the type of secondary windows' type to a non 'Window' derived class, "
                                   "or change your application skin to something else (Desktop for instance) by changing the skin of main form.");
            _loadingBar->error(ProjectBackend::instance()->name() + ": <b>Error</b>  |  Stopped at " + QTime::currentTime().toString());
            box.exec();
            break;

        case NoMainForm:
            box.setInformativeText("There is no main application window. Probably database has corrupted, "
                                   "please contact to support, or start a new project over.");
            _loadingBar->error(ProjectBackend::instance()->name() + ": <b>Error</b>  |  Stopped at " + QTime::currentTime().toString());
            box.exec();
            break;

        case CodeError: {
            box.setInformativeText(QString("Following control has some errors: <b>%1</b>").
                                   arg(error.id));
            QString detailedText;
            for (auto err : error.errors)
                detailedText += QString("Line %1, column %2: %3").
                                arg(err.line()).arg(err.column()).arg(err.description());
            box.setDetailedText(detailedText);
            _loadingBar->error(ProjectBackend::instance()->name() + ": <b>Error</b>  |  Stopped at " + QTime::currentTime().toString());
            box.exec();
            break;
        }

        case Stopped: {
            _loadingBar->error(ProjectBackend::instance()->name() + ": <b>Stopped</b>  |  Finished at " + QTime::currentTime().toString());
        }

        default:
            break;
    }

    delete _executiveWidget;
    _executiveWidget = nullptr;
    _stopButton->setDisabled(true);
}

void MainWindow::handleBuildsButtonClick()
{
    WindowManager::instance()->show(WindowManager::Builds);
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