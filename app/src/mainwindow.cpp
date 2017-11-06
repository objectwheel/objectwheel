#include <fit.h>
#include <zipper.h>
#include <string.h>
#include <toolboxtree.h>
#include <mainwindow.h>
#include <css.h>
#include <filemanager.h>
#include <projectmanager.h>
#include <usermanager.h>
#include <toolsmanager.h>
#include <savemanager.h>
#include <delayer.h>
#include <formscene.h>
#include <qmlpreviewer.h>
#include <formview.h>
#include <controlview.h>
#include <loadingindicator.h>
#include <global.h>
#include <outputwidget.h>

#include <QtConcurrent>
#include <QtNetwork>

using namespace Fit;

MainWindow* MainWindow::_instance = nullptr;
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , _centralWidget(this)
    , _progressWidget(&_centralWidget)
{
    if (_instance) {
        deleteLater();
        return;
    }

    _instance = this;
    setupGui();
    QTimer::singleShot(300, [=] { setupManagers(); });
}

MainWindow* MainWindow::instance()
{
    return _instance;
}

void MainWindow::setupGui()
{
    setWindowTitle(NAME_APP);
    setAutoFillBackground(true);

    QPalette p(palette());
    p.setColor(QPalette::Window, QColor("#E0E4E7"));
    setPalette(p);

    _designManager.setSettleWidget(&_settleWidget);

    _centralWidget.addWidget(Screen::Studio, &_settleWidget);
    _centralWidget.addWidget(Screen::Projects, &_projectsScreen);
    _centralWidget.addWidget(Screen::Login, &_loginScreen);
    _centralWidget.addWidget(Screen::About, &_aboutWidget);
    _centralWidget.addWidget(Screen::Builds, &_buildsScreen);
    _centralWidget.showWidget(Screen::Login);
    _progressWidget.showProgress("Loading");
    connect(&_centralWidget, &CentralWidget::visibleWidgetChanged, this, [=]
    {
        if (_centralWidget.visibleUid() == Screen::Studio) {
            _titleBar.show();
            _formsDockwidget.show();
            _propertiesDockwidget.show();
            _toolboxDockwidget.show();
            _inspectorDockwidget.show();
        } else {
            _titleBar.hide();
            _formsDockwidget.hide();
            _propertiesDockwidget.hide();
            _toolboxDockwidget.hide();
            _inspectorDockwidget.hide();
        }
    });

    // Toolbar settings
    QLabel* titleText = new QLabel;
    titleText->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    titleText->setText("Objectwheel Studio");
    titleText->setAlignment(Qt::AlignCenter);
    titleText->setStyleSheet("background: transparent; color:white;");
    QFont f;
    f.setWeight(QFont::Medium);
    titleText->setFont(f);

    /* Add Title Bar */
    addToolBar(Qt::TopToolBarArea, &_titleBar);
    _titleBar.setVisible(false);
    _titleBar.setFixedHeight(fit(34));
    _titleBar.setFloatable(false);
    _titleBar.setMovable(false);
    _titleBar.addWidget(titleText);
    _titleBar.setStyleSheet(QString("border: none; background:qlineargradient(spread:pad, x1:0.5, y1:0, x2:0.5, y2:1, stop:0 %1, stop:1 %2);")
                            .arg(QColor("#0D74C8").name()).arg(QColor("#0D74C8").darker(115).name()));

    /*** PROPERTIES DOCK WIDGET ***/
    QLabel* label = new QLabel;
    label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    label->setText(" Properties");
    label->setFont(f);

    QToolButton* pinButton = new QToolButton;
    pinButton->setToolTip("Pin/Unpin pane.");
    pinButton->setCursor(Qt::PointingHandCursor);
    pinButton->setIcon(QIcon(":/resources/images/unpin.png"));
    connect(pinButton, &QToolButton::clicked, this, [=]{
        _propertiesDockwidget.setFloating(!_propertiesDockwidget.isFloating());
    });

    QToolBar* toolbar = new QToolBar;
    toolbar->addWidget(label);
    toolbar->addWidget(pinButton);
    toolbar->setStyleSheet(CSS::DesignerPinbar);
    toolbar->setIconSize(QSize(fit(11), fit(11)));
    toolbar->setFixedHeight(fit(21));

    _propertiesDockwidget.setTitleBarWidget(toolbar);
    _propertiesDockwidget.setWidget(&_propertiesWidget);
    _propertiesDockwidget.setWindowTitle("Properties");
    _propertiesDockwidget.setVisible(false);
    _propertiesDockwidget.setFeatures(QDockWidget::DockWidgetMovable |
                                      QDockWidget::DockWidgetFloatable);

    /*** FORMS DOCK WIDGET ***/
    QLabel* label2 = new QLabel;
    label2->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    label2->setText(" Forms");
    label2->setFont(f);

    QToolButton* pinButton2 = new QToolButton;
    pinButton2->setToolTip("Pin/Unpin pane.");
    pinButton2->setCursor(Qt::PointingHandCursor);
    pinButton2->setIcon(QIcon(":/resources/images/unpin.png"));
    connect(pinButton2, &QToolButton::clicked, this, [=]{
        _formsDockwidget.setFloating(!_formsDockwidget.isFloating());
    });

    QToolBar* toolbar2 = new QToolBar;
    toolbar2->addWidget(label2);
    toolbar2->addWidget(pinButton2);
    toolbar2->setStyleSheet(CSS::DesignerPinbar);
    toolbar2->setIconSize(QSize(fit(11), fit(11)));
    toolbar2->setFixedHeight(fit(21));

    _formsDockwidget.setTitleBarWidget(toolbar2);
    _formsDockwidget.setWidget(&_formsWidget);
    _formsDockwidget.setWindowTitle("Forms");
    _formsDockwidget.setVisible(false);
    _formsDockwidget.setFeatures(QDockWidget::DockWidgetMovable |
                                 QDockWidget::DockWidgetFloatable);

    /*** TOOLBOX DOCK WIDGET ***/
    QLabel* label3 = new QLabel;
    label3->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    label3->setText(" Toolbox");
    label3->setFont(f);

    QToolButton* pinButton3 = new QToolButton;
    pinButton3->setToolTip("Pin/Unpin pane.");
    pinButton3->setCursor(Qt::PointingHandCursor);
    pinButton3->setIcon(QIcon(":/resources/images/unpin.png"));
    connect(pinButton3, &QToolButton::clicked, this, [=]{
        _toolboxDockwidget.setFloating(!_toolboxDockwidget.isFloating());
    });

    QToolBar* toolbar3 = new QToolBar;
    toolbar3->addWidget(label3);
    toolbar3->addWidget(pinButton3);
    toolbar3->setStyleSheet(CSS::DesignerPinbar);
    toolbar3->setIconSize(QSize(fit(11), fit(11)));
    toolbar3->setFixedHeight(fit(21));

    _toolboxDockwidget.setTitleBarWidget(toolbar3);
    _toolboxDockwidget.setWidget(&_toolbox);
    _toolboxDockwidget.setWindowTitle("Toolbox");
    _toolboxDockwidget.setVisible(false);
    _toolboxDockwidget.setFeatures(QDockWidget::DockWidgetMovable |
                                   QDockWidget::DockWidgetFloatable);

    connect(_toolbox.toolboxTree()->indicatorButton(),
      &FlatButton::clicked, this, [=] {
        auto splitter = DesignManager::splitter();
        auto controlView = DesignManager::controlView();
        auto formView = DesignManager::formView();
        auto qmlEditorView = DesignManager::qmlEditorView();
        auto sizes = splitter->sizes();
        QSize size;
        if (formView->isVisible())
            size = formView->size();
        else if (qmlEditorView->isVisible())
            size = qmlEditorView->size();
        else
            size = controlView->size();
        sizes[splitter->indexOf(controlView)] = size.height();
        auto previousControl = DesignManager::controlScene()->mainControl();
        if (previousControl)
            previousControl->deleteLater();
        auto url = _toolbox.toolboxTree()->urls(_toolbox.toolboxTree()->currentItem())[0];
        auto control = SaveManager::exposeControl(dname(dname(url.toLocalFile())), ControlGui);
        DesignManager::controlScene()->setMainControl(control);
        DesignManager::setMode(ControlGui);
        control->refresh();
        for (auto childControl : control->childControls())
            childControl->refresh();
        splitter->setSizes(sizes);
    });

    /*** INSPECTOR DOCK WIDGET ***/
    QLabel* label4 = new QLabel;
    label4->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    label4->setText(" Control Inspector");
    label4->setFont(f);

    QToolButton* pinButton4 = new QToolButton;
    pinButton4->setToolTip("Pin/Unpin pane.");
    pinButton4->setCursor(Qt::PointingHandCursor);
    pinButton4->setIcon(QIcon(":/resources/images/unpin.png"));
    connect(pinButton4, &QToolButton::clicked, this, [=]{
        _inspectorDockwidget.setFloating(!_inspectorDockwidget.isFloating());
    });

    QToolBar* toolbar4 = new QToolBar;
    toolbar4->addWidget(label4);
    toolbar4->addWidget(pinButton4);
    toolbar4->setStyleSheet(CSS::DesignerPinbar);
    toolbar4->setIconSize(QSize(fit(11), fit(11)));
    toolbar4->setFixedHeight(fit(21));

    connect(&_inspectorWidget, SIGNAL(controlClicked(Control*)),
      &_designManager, SLOT(controlClicked(Control*)));
    connect(&_inspectorWidget, SIGNAL(controlDoubleClicked(Control*)),
      &_designManager, SLOT(controlDoubleClicked(Control*)));

    _inspectorDockwidget.setTitleBarWidget(toolbar4);
    _inspectorDockwidget.setWidget(&_inspectorWidget);
    _inspectorDockwidget.setWindowTitle("Control Inspector");
    _inspectorDockwidget.setVisible(false);
    _inspectorDockwidget.setFeatures(QDockWidget::DockWidgetMovable |
                                     QDockWidget::DockWidgetFloatable);

    addDockWidget(Qt::LeftDockWidgetArea, &_toolboxDockwidget);
    addDockWidget(Qt::LeftDockWidgetArea, &_formsDockwidget);
    addDockWidget(Qt::RightDockWidgetArea, &_inspectorDockwidget);
    addDockWidget(Qt::RightDockWidgetArea, &_propertiesDockwidget);
}

void MainWindow::setupManagers()
{
    ToolsManager::setToolboxTree(_toolbox.toolboxTree());
    auto userManager = new UserManager(this);
    Q_UNUSED(userManager);
    auto* projectManager = new ProjectManager(this);
    projectManager->setMainWindow(this);
    new SaveManager(this);
    new QmlPreviewer(this);

    connect(SaveManager::instance(), SIGNAL(parserRunningChanged(bool)),
      SLOT(handleIndicatorChanges()));
    connect(QmlPreviewer::instance(), SIGNAL(workingChanged(bool)),
      SLOT(handleIndicatorChanges()));
    connect(qApp, SIGNAL(aboutToQuit()),
      SLOT(cleanupObjectwheel()));

    auto ret = QtConcurrent::run(&UserManager::tryAutoLogin);
    Delayer::delay(&ret, &QFuture<bool>::isRunning);
    if (ret.result()) {
        ProjectsScreen::refreshProjectList();
        _progressWidget.hideProgress();
        _centralWidget.showWidget(Screen::Projects);
    } else {
        _progressWidget.hideProgress();
    }
}

void MainWindow::handleIndicatorChanges()
{
    DesignManager::loadingIndicator()->setRunning(SaveManager::parserWorking() || QmlPreviewer::working());
}

void MainWindow::cleanupObjectwheel()
{
    while(SaveManager::parserWorking())
        Delayer::delay(100);

    UserManager::stopUserSession();

    qApp->processEvents();
}

void MainWindow::showDockWidgets()
{
    _formsDockwidget.show();
    _propertiesDockwidget.show();
    _toolboxDockwidget.show();
    _inspectorDockwidget.show();
}

void MainWindow::hideDockWidgets()
{
    _formsDockwidget.hide();
    _propertiesDockwidget.hide();
    _toolboxDockwidget.hide();
    _inspectorDockwidget.hide();
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    QMainWindow::closeEvent(event);
    emit quitting();
}

InspectorWidget* MainWindow::inspectorWidget()
{
    return &_inspectorWidget;
}

void MainWindow::clearStudio()
{
    
}

CentralWidget* MainWindow::centralWidget()
{
    return &_centralWidget;
}

ProgressWidget* MainWindow::progressWidget()
{
    return &_progressWidget;
}

//void MainWindow::on_secureExitButton_clicked()
//{
//    SplashScreen::setText("Stopping user session");
//    SplashScreen::show(true);
//    UserManager::clearAutoLogin();
//    auto ret = QtConcurrent::run(&UserManager::stopUserSession);
//    Delayer::delay(&ret, &QFuture<void>::isRunning);
//    SplashScreen::hide();
//    SplashScreen::setText("Loading");
//    SceneManager::show("loginScene", SceneManager::ToLeft);
//}
