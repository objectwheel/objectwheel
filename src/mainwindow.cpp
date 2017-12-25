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
#include <loadingindicator.h>
#include <global.h>
#include <outputwidget.h>

#include <QtConcurrent>
#include <QtNetwork>

#define wM (WindowManager::instance())

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent)
{
    _settleWidget = new QFrame;
    _titleBar = new QToolBar;
    _toolboxDockwidget = new QDockWidget;
    _propertiesDockwidget = new QDockWidget;
    _formsDockwidget = new QDockWidget;
    _inspectorDockwidget = new QDockWidget;
    _designerWidget = new DesignerWidget;
    _toolboxPane = new ToolboxPane;
    _propertiesPane = new PropertiesPane;
    _formsPane = new FormsPane;
    _inspectorPage = new InspectorPane;

    QPalette p(palette());
    p.setColor(backgroundRole(), "#E0E4E7");
    setStyleSheet("QMainWindow::separator{height: 0px;}");
    setCentralWidget(_settleWidget);
    setAutoFillBackground(true);
    setWindowTitle(APP_NAME);
    setPalette(p);

    _settleWidget->setFrameShape(QFrame::StyledPanel);
    _settleWidget->setFrameShadow(QFrame::Plain);
    _designerWidget->setSettleWidget(_settleWidget);

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
    addToolBar(Qt::TopToolBarArea, _titleBar);
    _titleBar->setFixedHeight(fit::fx(34));
    _titleBar->setFloatable(false);
    _titleBar->setMovable(false);
    _titleBar->addWidget(titleText);
    _titleBar->setStyleSheet(QString("border: none; background:qlineargradient(spread:pad, x1:0.5, y1:0, x2:0.5, y2:1, stop:0 %1, stop:1 %2);")
      .arg(QColor("#0D74C8").name()).arg(QColor("#0D74C8").darker(115).name()));

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

    connect(_toolboxPane->toolboxTree()->indicatorButton(),
      &FlatButton::clicked, this, [=] {
        auto splitter = DesignerWidget::splitter();
        auto controlView = DesignerWidget::controlView();
        auto formView = DesignerWidget::formView();
        auto qmlEditorView = DesignerWidget::qmlEditorView();
        auto sizes = splitter->sizes();
        QSize size;
        if (formView->isVisible())
            size = formView->size();
        else if (qmlEditorView->isVisible())
            size = qmlEditorView->size();
        else
            size = controlView->size();
        sizes[splitter->indexOf(controlView)] = size.height();
        auto previousControl = DesignerWidget::controlScene()->mainControl();
        if (previousControl)
            previousControl->deleteLater();
        auto url = _toolboxPane->toolboxTree()->urls(_toolboxPane->toolboxTree()->currentItem())[0];
        auto control = SaveBackend::exposeControl(dname(dname(url.toLocalFile())), ControlGui);
        DesignerWidget::controlScene()->setMainControl(control);
        DesignerWidget::setMode(ControlGui);
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

    QToolBar* toolbar4 = new QToolBar;
    toolbar4->addWidget(label4);
    toolbar4->addWidget(pinButton4);
    toolbar4->setStyleSheet(CSS::DesignerPinbar);
    toolbar4->setIconSize(QSize(fit::fx(11), fit::fx(11)));
    toolbar4->setFixedHeight(fit::fx(22.8));

    connect(_inspectorPage, SIGNAL(controlClicked(Control*)),
      _designerWidget, SLOT(controlClicked(Control*)));
    connect(_inspectorPage, SIGNAL(controlDoubleClicked(Control*)),
      _designerWidget, SLOT(controlDoubleClicked(Control*)));

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

void MainWindow::handleIndicatorChanges()
{
    DesignerWidget::loadingIndicator()->setRunning(SaveBackend::parserWorking() || PreviewBackend::working());
}

void MainWindow::cleanupObjectwheel()
{
    while(SaveBackend::parserWorking())
        Delayer::delay(100);

    UserBackend::stopUserSession();

    qApp->processEvents();
}

//void MainWindow::setupManagers()
//{
//    ToolsManager::instance()->addToolboxTree(_toolboxPane->toolboxTree());
//    auto userManager = new UserManager(this);
//    Q_UNUSED(userManager);
//    new ProjectManager(this);
//    ProjectManager::setMainWindow(this);
//    new SaveBackend(this);
//    new QmlPreviewer(this);

//    connect(SaveBackend::instance(), SIGNAL(parserRunningChanged(bool)),
//      SLOT(handleIndicatorChanges()));
//    connect(QmlPreviewer::instance(), SIGNAL(workingChanged(bool)),
//      SLOT(handleIndicatorChanges()));
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
//    auto ret = QtConcurrent::run(&UserManager::stopUserSession);
//    Delayer::delay(&ret, &QFuture<void>::isRunning);
//    SplashScreen::hide();
//    SplashScreen::setText("Loading");
//    SceneManager::show("loginScene", SceneManager::ToLeft);
//}