#include <fit.h>
#include <zipper.h>
#include <string.h>
#include <listwidget.h>
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

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    setupGui();
    QTimer::singleShot(300, [=] { setupManagers(); });
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

    _propertiesDockwidget.setWidget(&_propertiesWidget);
    _propertiesDockwidget.setWindowTitle("Properties");
    _propertiesDockwidget.setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    addDockWidget(Qt::RightDockWidgetArea, &_propertiesDockwidget);

    _propertiesWidget.setObjectName(QStringLiteral("_propertiesWidget"));

    _formsDockwidget.setWidget(&_formsWidget);
    _formsDockwidget.setWindowTitle("Forms");
    _formsDockwidget.setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    addDockWidget(Qt::RightDockWidgetArea, &_formsDockwidget);

    _formsWidget.setObjectName(QStringLiteral("_formsWidget"));

    _toolboxDockwidget.setWidget(&_toolbox);
    _toolboxDockwidget.setWindowTitle("Toolbox");
    _toolboxDockwidget.setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    addDockWidget(Qt::LeftDockWidgetArea, &_toolboxDockwidget);

    connect(_toolbox.toolboxList(),(void(ListWidget::*)(int))(&ListWidget::currentRowChanged),[=](int i){
        if (i>=0) {
            DesignManager::formScene()->clearSelection();
            DesignManager::controlScene()->clearSelection();
        }
    });

    connect(_toolbox.toolboxList()->indicatorButton(), &FlatButton::clicked, [=] {
        auto previousControl = DesignManager::controlScene()->mainControl();
        if (previousControl)
            previousControl->deleteLater();
        auto url = _toolbox.toolboxList()->GetUrls(_toolbox.toolboxList()->currentItem())[0];
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
    ToolsManager::setListWidget(_toolbox.toolboxList());
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
//        sceneList.AddUrls(item, urls);
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
