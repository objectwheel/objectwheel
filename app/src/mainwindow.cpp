#include <fit.h>
#include <zipper.h>
#include <string.h>
#include <titleBar.h>
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
    setStyleSheet(QLatin1String("#_centralWidget, #MainWindow{\n"
                                "background:\"#e0e4e7\";\n }"));

    _centralWidget.setObjectName(QStringLiteral("_centralWidget"));
    setCentralWidget(&_centralWidget);
    _verticalLayout.setSpacing(0);
    _verticalLayout.setObjectName(QStringLiteral("_verticalLayout"));
    _verticalLayout.setContentsMargins(0, 0, 0, 0);
    _centralWidget.setLayout(&_verticalLayout);

    _titleBar.setObjectName(QStringLiteral("_titleBar"));
    _titleBar.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    _titleBar.setMinimumSize(QSize(0, 45));
    _titleBar.setMaximumSize(QSize(16777215, 45));
    _verticalLayout.addWidget(&_titleBar);

    _settleWidget.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    _verticalLayout.addWidget(&_settleWidget);

    _designManager.setSettleWidget(&_settleWidget);

    _propertiesWidget.setObjectName(QStringLiteral("_propertiesWidget"));
    _propertiesWidget.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    _formsWidget.setObjectName(QStringLiteral("_formsWidget"));
    _formsWidget.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    _formsWidget.setParent(this);

    /* Add Title Bar */
    fit(&_titleBar, Fit::Height, true);
    _titleBar.setText("Objectwheel Studio");
    _titleBar.setColor("#0D74C8");
    _titleBar.setShadowColor("#EAEEF1");

    _toolboxDockwidget.setWidget(&_toolbox);
    _toolboxDockwidget.setWindowTitle("Toolbox");
    _toolboxDockwidget.setFloating(true);
    addDockWidget(Qt::RightDockWidgetArea, &_toolboxDockwidget);

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
