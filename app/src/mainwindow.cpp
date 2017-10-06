#include <fit.h>
#include <zipper.h>
#include <string.h>
#include <toolboxtree.h>
#include <mainwindow.h>
#include <css.h>
#include <splashscreen.h>
#include <filemanager.h>
#include <projectmanager.h>
#include <usermanager.h>
#include <toolsmanager.h>
#include <savemanager.h>
#include <splashscreen.h>
#include <scenemanager.h>
#include <delayer.h>
#include <control.h>
#include <formscene.h>
#include <qmlpreviewer.h>
#include <loadingindicator.h>

#include <QtConcurrent>
#include <QtNetwork>

using namespace Fit;

MainWindow* MainWindow::_instance = nullptr;
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
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
    setWindowTitle(QApplication::translate("MainWindow", "Objectwheel", 0));
    setObjectName(QStringLiteral("MainWindow"));
    _centralWidget.setObjectName(QStringLiteral("_centralWidget"));
    setStyleSheet(QLatin1String("#_centralWidget, #MainWindow{\n"
                                "background:\"#e0e4e7\";\n }"));

    setCentralWidget(&_centralWidget);
    _designManager.setSettleWidget(&_centralWidget);

    // Toolbar settings
    QLabel* titleText = new QLabel;
    titleText->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    titleText->setText("Objectwheel Studio");
    titleText->setAlignment(Qt::AlignCenter);
    titleText->setStyleSheet("background: transparent; color:white;");

    /* Add Title Bar */
    addToolBar(Qt::TopToolBarArea, &_titleBar);
    _titleBar.setFixedHeight(fit(42));
    _titleBar.setFloatable(false);
    _titleBar.setMovable(false);
    _titleBar.addWidget(titleText);
    _titleBar.setStyleSheet(QString("border: none; background:qlineargradient(spread:pad, x1:0.5, y1:0, x2:0.5, y2:1, stop:0 %1, stop:1 %2);")
                            .arg(QColor("#0D74C8").name()).arg(QColor("#0D74C8").darker(115).name()));

    /*** PROPERTIES DOCK WIDGET ***/
    QLabel* label = new QLabel;
    label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    label->setText(" Properties");

    QToolButton* pinButton = new QToolButton;
    pinButton->setToolTip("Pin/Unpin pane.");
    pinButton->setCursor(Qt::PointingHandCursor);
    pinButton->setIcon(QIcon(":/resources/images/unpin.png"));
    connect(pinButton, &QToolButton::clicked, [=]{
        _propertiesDockwidget.setFloating(!_propertiesDockwidget.isFloating());
    });

    QToolBar* toolbar = new QToolBar;
    toolbar->addWidget(label);
    toolbar->addWidget(pinButton);
    toolbar->setStyleSheet(CSS::DesignerToolbar);
    toolbar->setIconSize(QSize(fit(11), fit(11)));
    toolbar->setFixedHeight(fit(21));

    _propertiesDockwidget.setTitleBarWidget(toolbar);
    _propertiesDockwidget.setWidget(&_propertiesWidget);
    _propertiesDockwidget.setWindowTitle("Properties");
    _propertiesDockwidget.setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    addDockWidget(Qt::RightDockWidgetArea, &_propertiesDockwidget);

    /*** FORMS DOCK WIDGET ***/
    QLabel* label2 = new QLabel;
    label2->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    label2->setText(" Forms");

    QToolButton* pinButton2 = new QToolButton;
    pinButton2->setToolTip("Pin/Unpin pane.");
    pinButton2->setCursor(Qt::PointingHandCursor);
    pinButton2->setIcon(QIcon(":/resources/images/unpin.png"));
    connect(pinButton2, &QToolButton::clicked, [=]{
        _formsDockwidget.setFloating(!_formsDockwidget.isFloating());
    });

    QToolBar* toolbar2 = new QToolBar;
    toolbar2->addWidget(label2);
    toolbar2->addWidget(pinButton2);
    toolbar2->setStyleSheet(CSS::DesignerToolbar);
    toolbar2->setIconSize(QSize(fit(11), fit(11)));
    toolbar2->setFixedHeight(fit(21));

    _formsDockwidget.setTitleBarWidget(toolbar2);
    _formsDockwidget.setWidget(&_formsWidget);
    _formsDockwidget.setWindowTitle("Forms");
    _formsDockwidget.setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    addDockWidget(Qt::RightDockWidgetArea, &_formsDockwidget);

    /*** TOOLBOX DOCK WIDGET ***/
    QLabel* label3 = new QLabel;
    label3->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    label3->setText(" Toolbox");

    QToolButton* pinButton3 = new QToolButton;
    pinButton3->setToolTip("Pin/Unpin pane.");
    pinButton3->setCursor(Qt::PointingHandCursor);
    pinButton3->setIcon(QIcon(":/resources/images/unpin.png"));
    connect(pinButton3, &QToolButton::clicked, [=]{
        _toolboxDockwidget.setFloating(!_toolboxDockwidget.isFloating());
    });

    QToolBar* toolbar3 = new QToolBar;
    toolbar3->addWidget(label3);
    toolbar3->addWidget(pinButton3);
    toolbar3->setStyleSheet(CSS::DesignerToolbar);
    toolbar3->setIconSize(QSize(fit(11), fit(11)));
    toolbar3->setFixedHeight(fit(21));

    _toolboxDockwidget.setTitleBarWidget(toolbar3);
    _toolboxDockwidget.setWidget(&_toolbox);
    _toolboxDockwidget.setWindowTitle("Toolbox");
    _toolboxDockwidget.setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    addDockWidget(Qt::LeftDockWidgetArea, &_toolboxDockwidget);

    connect(_toolbox.toolboxTree()->indicatorButton(), &FlatButton::clicked, [=] {
        auto previousControl = DesignManager::controlScene()->mainControl();
        if (previousControl)
            previousControl->deleteLater();
        auto url = _toolbox.toolboxTree()->urls(_toolbox.toolboxTree()->currentItem())[0];
        auto control = SaveManager::exposeControl(dname(dname(url.toLocalFile())));
        DesignManager::controlScene()->setMainControl(control);
        DesignManager::setMode(DesignManager::ControlGUI);
        control->refresh();
        connect(control, &Control::initialized, [=] {
            control->controlTransaction()->setTransactionsEnabled(true);
        });
        for (auto childControl : control->childControls())
            childControl->refresh();
    });

    // Init Splash Screen
    SplashScreen::init(this);
    SplashScreen::setText("Loading");
    SplashScreen::setTextColor("#2e3a41");
    SplashScreen::setBackgroundBrush(QColor("#e0e4e7"));
    SplashScreen::setIcon(QIcon(":/resources/images/logo.png"));
    SplashScreen::setIconSize(Fit::fit(160), Fit::fit(80));
    SplashScreen::setLoadingSize(Fit::fit(24), Fit::fit(24));
    SplashScreen::setLoadingImageFilename("qrc:///resources/images/loading.png");
    SplashScreen::show(false);
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
    auto sceneManager = new SceneManager;
    sceneManager->setMainWindow(this);
//    sceneManager->setSceneListWidget(&sceneList);
    sceneManager->addScene("studioScene", &_centralWidget);
    sceneManager->addScene("projectsScene", &_projectsScreen);
    sceneManager->addScene("loginScene", &_loginScreen);
    sceneManager->addScene("aboutScene", &_aboutWidget);
    sceneManager->addScene("buildsScene", &_buildsScreen);
    sceneManager->setCurrent("loginScene", false);

    connect(SaveManager::instance(), SIGNAL(parserRunningChanged(bool)), SLOT(handleIndicatorChanges()));
    connect(QmlPreviewer::instance(), SIGNAL(workingChanged(bool)), SLOT(handleIndicatorChanges()));

    SplashScreen::raise();

    for (auto scene : sceneManager->scenes()) {
        QList<QUrl> urls;
        QString sceneName;
        QString iconPath = ":/resources/images/" + scene + "Icon.png";
        if (scene == "projectsScene") {
            sceneName = "Projects";
        } else if (scene == "studioScene") {
            sceneName = "Studio";
        } else if (scene == "aboutScene") {
            sceneName = "About";
        } else if (scene == "loginScene") {
            continue;
        } else if (scene == "buildsScene") {
            continue;
        }
        urls << scene;
//        QListWidgetItem* item = new QListWidgetItem(QIcon(iconPath), sceneName);
//        sceneList.insertItem(0, item);
//        sceneList.addUrls(item, urls);
    }
    connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(cleanupObjectwheel()));

    auto ret = QtConcurrent::run(&UserManager::tryAutoLogin);
    Delayer::delay(&ret, &QFuture<bool>::isRunning);
    if (ret.result()) {
        ProjectsScreen::refreshProjectList();
        SplashScreen::hide();
        SceneManager::show("projectsScene", SceneManager::ToLeft);
    } else {
        SplashScreen::hide();
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
}

void MainWindow::hideDockWidgets()
{
    _formsDockwidget.hide();
    _propertiesDockwidget.hide();
    _toolboxDockwidget.hide();
}

void MainWindow::clearStudio()
{

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
