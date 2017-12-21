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

#define wM (WindowManager::instance())

MainWindow* MainWindow::_instance = nullptr;
MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
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
    setAutoFillBackground(true);
    setCentralWidget(&_settleWidget);

    QPalette p(palette());
    p.setColor(QPalette::Window, QColor("#E0E4E7"));
    setPalette(p);
    setStyleSheet("QMainWindow::separator{height: 0px;}");

    _settleWidget.setFrameShape(QFrame::StyledPanel);
    _settleWidget.setFrameShadow(QFrame::Plain);
    _designManager.setSettleWidget(&_settleWidget);

    wM->add(WindowManager::Studio, this);
    wM->add(WindowManager::Projects, &_projectsScreen);
    wM->add(WindowManager::Login, &_loginScreen);
    wM->show(WindowManager::Login);
    wM->busy("Loading");
    connect(&_centralWidget, &View::visibleChanged, this, [&]
    {
        if (_centralWidget.current() == Screen::Studio) {
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
    _titleBar.setFixedHeight(fit::fx(34));
    _titleBar.setFloatable(false);
    _titleBar.setMovable(false);
    _titleBar.addWidget(titleText);
    _titleBar.setStyleSheet(QString("border: none; background:qlineargradient(spread:pad, x1:0.5, y1:0, x2:0.5, y2:1, stop:0 %1, stop:1 %2);")
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
    connect(pinButton, &QToolButton::clicked, this, [&]{
        _propertiesDockwidget.setFloating(!_propertiesDockwidget.isFloating());
    });

    QToolBar* toolbar = new QToolBar;
    toolbar->addWidget(label);
    toolbar->addWidget(pinButton);
    toolbar->setStyleSheet(CSS::DesignerPinbar);
    toolbar->setIconSize(QSize(fit::fx(11), fit::fx(11)));
    toolbar->setFixedHeight(fit::fx(22.8));

    _propertiesDockwidget.setTitleBarWidget(toolbar);
    _propertiesDockwidget.setWidget(&_propertiesWidget);
    _propertiesDockwidget.setWindowTitle("Properties");
    _propertiesDockwidget.setVisible(false);
    _propertiesDockwidget.setAttribute(Qt::WA_TranslucentBackground);
    _propertiesDockwidget.setFeatures(QDockWidget::DockWidgetMovable |
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
    connect(pinButton2, &QToolButton::clicked, this, [&]{
        _formsDockwidget.setFloating(!_formsDockwidget.isFloating());
    });

    QToolBar* toolbar2 = new QToolBar;
    toolbar2->addWidget(label2);
    toolbar2->addWidget(pinButton2);
    toolbar2->setStyleSheet(CSS::DesignerPinbar);
    toolbar2->setIconSize(QSize(fit::fx(11), fit::fx(11)));
    toolbar2->setFixedHeight(fit::fx(22.8));

    _formsDockwidget.setTitleBarWidget(toolbar2);
    _formsDockwidget.setWidget(&_formsWidget);
    _formsDockwidget.setWindowTitle("Forms");
    _formsDockwidget.setVisible(false);
    _formsDockwidget.setAttribute(Qt::WA_TranslucentBackground);
    _formsDockwidget.setFeatures(QDockWidget::DockWidgetMovable |
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
    connect(pinButton3, &QToolButton::clicked, this, [&]{
        _toolboxDockwidget.setFloating(!_toolboxDockwidget.isFloating());
    });

    QToolButton* toolboxSettingsButton = new QToolButton;
    toolboxSettingsButton->setToolTip("Toolbox settings.");
    toolboxSettingsButton->setCursor(Qt::PointingHandCursor);
    toolboxSettingsButton->setIcon(QIcon(":/resources/images/settings.png"));
    connect(toolboxSettingsButton, &QToolButton::clicked, this, [&] {
        _toolbox.showSettings();
    });

    QToolBar* toolbar3 = new QToolBar;
    toolbar3->addWidget(label3);
    toolbar3->addWidget(toolboxSettingsButton);
    toolbar3->addWidget(pinButton3);
    toolbar3->setStyleSheet(CSS::DesignerPinbar);
    toolbar3->setIconSize(QSize(fit::fx(11), fit::fx(11)));
    toolbar3->setFixedHeight(fit::fx(22.8));

    _toolboxDockwidget.setTitleBarWidget(toolbar3);
    _toolboxDockwidget.setWidget(&_toolbox);
    _toolboxDockwidget.setWindowTitle("Toolbox");
    _toolboxDockwidget.setVisible(false);
    _toolboxDockwidget.setAttribute(Qt::WA_TranslucentBackground);
    _toolboxDockwidget.setFeatures(QDockWidget::DockWidgetMovable |
                                   QDockWidget::DockWidgetFloatable);

    connect(_toolbox.toolboxTree()->indicatorButton(),
      &FlatButton::clicked, this, [&] {
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
    connect(pinButton4, &QToolButton::clicked, this, [&]{
        _inspectorDockwidget.setFloating(!_inspectorDockwidget.isFloating());
    });

    QToolBar* toolbar4 = new QToolBar;
    toolbar4->addWidget(label4);
    toolbar4->addWidget(pinButton4);
    toolbar4->setStyleSheet(CSS::DesignerPinbar);
    toolbar4->setIconSize(QSize(fit::fx(11), fit::fx(11)));
    toolbar4->setFixedHeight(fit::fx(22.8));

    connect(&_inspectorWidget, SIGNAL(controlClicked(Control*)),
      &_designManager, SLOT(controlClicked(Control*)));
    connect(&_inspectorWidget, SIGNAL(controlDoubleClicked(Control*)),
      &_designManager, SLOT(controlDoubleClicked(Control*)));

    _inspectorDockwidget.setTitleBarWidget(toolbar4);
    _inspectorDockwidget.setWidget(&_inspectorWidget);
    _inspectorDockwidget.setWindowTitle("Control Inspector");
    _inspectorDockwidget.setVisible(false);
    _inspectorDockwidget.setAttribute(Qt::WA_TranslucentBackground);
    _inspectorDockwidget.setFeatures(QDockWidget::DockWidgetMovable |
                                     QDockWidget::DockWidgetFloatable);

    addDockWidget(Qt::LeftDockWidgetArea, &_toolboxDockwidget);
    addDockWidget(Qt::LeftDockWidgetArea, &_formsDockwidget);
    addDockWidget(Qt::RightDockWidgetArea, &_inspectorDockwidget);
    addDockWidget(Qt::RightDockWidgetArea, &_propertiesDockwidget);

    createActions();
    createMenus();
}

void MainWindow::setupManagers()
{
    ToolsManager::instance()->addToolboxTree(_toolbox.toolboxTree());
    auto userManager = new UserManager(this);
    Q_UNUSED(userManager);
    new ProjectManager(this);
    ProjectManager::setMainWindow(this);
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
        _centralWidget.show(Screen::Projects);
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

BuildsDialog* MainWindow::buildsDialog()
{
    return &_buildsDialog;
}

InspectorWidget* MainWindow::inspectorWidget()
{
    return &_inspectorWidget;
}

void MainWindow::clearStudio()
{
    
}

ProgressWidget* MainWindow::progressWidget()
{
    return &_progressWidget;
}

void MainWindow::newFile()
{
    //TODO
}

void MainWindow::open()
{
    //TODO
}

void MainWindow::save()
{
    //TODO
}

void MainWindow::print()
{
    //TODO
}

void MainWindow::undo()
{
    //TODO
}

void MainWindow::redo()
{
    //TODO
}

void MainWindow::cut()
{
    //TODO
}

void MainWindow::copy()
{
    //TODO
}

void MainWindow::paste()
{
    //TODO
}

void MainWindow::bold()
{
    //TODO
}

void MainWindow::italic()
{
    //TODO
}

void MainWindow::leftAlign()
{
    //TODO
}

void MainWindow::rightAlign()
{
    //TODO
}

void MainWindow::justify()
{
    //TODO
}

void MainWindow::center()
{
    //TODO
}

void MainWindow::setLineSpacing()
{
    //TODO
}

void MainWindow::setParagraphSpacing()
{
    //TODO
}

void MainWindow::preferences()
{
    _preferencesWidget.show();
}

void MainWindow::about()
{
    _aboutWidget.show();
}

void MainWindow::createActions()
{
    newAct = new QAction(tr("&New"), this);
    newAct->setShortcuts(QKeySequence::New);
    newAct->setStatusTip(tr("Create a new file"));
    connect(newAct, &QAction::triggered, this, &MainWindow::newFile);

    openAct = new QAction(tr("&Open..."), this);
    openAct->setShortcuts(QKeySequence::Open);
    openAct->setStatusTip(tr("Open an existing file"));
    connect(openAct, &QAction::triggered, this, &MainWindow::open);

    saveAct = new QAction(tr("&Save"), this);
    saveAct->setShortcuts(QKeySequence::Save);
    saveAct->setStatusTip(tr("Save the document to disk"));
    connect(saveAct, &QAction::triggered, this, &MainWindow::save);

    printAct = new QAction(tr("&Print..."), this);
    printAct->setShortcuts(QKeySequence::Print);
    printAct->setStatusTip(tr("Print the document"));
    connect(printAct, &QAction::triggered, this, &MainWindow::print);

    exitAct = new QAction(tr("E&xit"), this);
    exitAct->setShortcuts(QKeySequence::Quit);
    exitAct->setStatusTip(tr("Exit the application"));
    connect(exitAct, &QAction::triggered, this, &QWidget::close);

    undoAct = new QAction(tr("&Undo"), this);
    undoAct->setShortcuts(QKeySequence::Undo);
    undoAct->setStatusTip(tr("Undo the last operation"));
    connect(undoAct, &QAction::triggered, this, &MainWindow::undo);

    redoAct = new QAction(tr("&Redo"), this);
    redoAct->setShortcuts(QKeySequence::Redo);
    redoAct->setStatusTip(tr("Redo the last operation"));
    connect(redoAct, &QAction::triggered, this, &MainWindow::redo);

    cutAct = new QAction(tr("Cu&t"), this);
    cutAct->setShortcuts(QKeySequence::Cut);
    cutAct->setStatusTip(tr("Cut the current selection's contents to the "
                            "clipboard"));
    connect(cutAct, &QAction::triggered, this, &MainWindow::cut);

    copyAct = new QAction(tr("&Copy"), this);
    copyAct->setShortcuts(QKeySequence::Copy);
    copyAct->setStatusTip(tr("Copy the current selection's contents to the "
                             "clipboard"));
    connect(copyAct, &QAction::triggered, this, &MainWindow::copy);

    pasteAct = new QAction(tr("&Paste"), this);
    pasteAct->setShortcuts(QKeySequence::Paste);
    pasteAct->setStatusTip(tr("Paste the clipboard's contents into the current "
                              "selection"));
    connect(pasteAct, &QAction::triggered, this, &MainWindow::paste);

    boldAct = new QAction(tr("&Bold"), this);
    boldAct->setCheckable(true);
    boldAct->setShortcut(QKeySequence::Bold);
    boldAct->setStatusTip(tr("Make the text bold"));
    connect(boldAct, &QAction::triggered, this, &MainWindow::bold);

    QFont boldFont = boldAct->font();
    boldFont.setBold(true);
    boldAct->setFont(boldFont);

    italicAct = new QAction(tr("&Italic"), this);
    italicAct->setCheckable(true);
    italicAct->setShortcut(QKeySequence::Italic);
    italicAct->setStatusTip(tr("Make the text italic"));
    connect(italicAct, &QAction::triggered, this, &MainWindow::italic);

    QFont italicFont = italicAct->font();
    italicFont.setItalic(true);
    italicAct->setFont(italicFont);

    setLineSpacingAct = new QAction(tr("Set &Line Spacing..."), this);
    setLineSpacingAct->setStatusTip(tr("Change the gap between the lines of a "
                                       "paragraph"));
    connect(setLineSpacingAct, &QAction::triggered, this, &MainWindow::setLineSpacing);

    setParagraphSpacingAct = new QAction(tr("Set &Paragraph Spacing..."), this);
    setParagraphSpacingAct->setStatusTip(tr("Change the gap between paragraphs"));
    connect(setParagraphSpacingAct, &QAction::triggered,
            this, &MainWindow::setParagraphSpacing);

    aboutAct = new QAction(tr("&About"), this);
    aboutAct->setStatusTip(tr("Show the application's About box"));
    connect(aboutAct, &QAction::triggered, this, &MainWindow::about);

    aboutQtAct = new QAction(tr("About &Qt"), this);
    aboutQtAct->setStatusTip(tr("Show the Qt library's About box"));
    connect(aboutQtAct, &QAction::triggered, qApp, &QApplication::aboutQt);

    preferencesAct = new QAction(tr("&Preferences"), this);
    preferencesAct->setStatusTip(tr("Show Objectwheel Preferences"));
    connect(preferencesAct, &QAction::triggered, this, &MainWindow::preferences);

    leftAlignAct = new QAction(tr("&Left Align"), this);
    leftAlignAct->setCheckable(true);
    leftAlignAct->setShortcut(tr("Ctrl+L"));
    leftAlignAct->setStatusTip(tr("Left align the selected text"));
    connect(leftAlignAct, &QAction::triggered, this, &MainWindow::leftAlign);

    rightAlignAct = new QAction(tr("&Right Align"), this);
    rightAlignAct->setCheckable(true);
    rightAlignAct->setShortcut(tr("Ctrl+R"));
    rightAlignAct->setStatusTip(tr("Right align the selected text"));
    connect(rightAlignAct, &QAction::triggered, this, &MainWindow::rightAlign);

    justifyAct = new QAction(tr("&Justify"), this);
    justifyAct->setCheckable(true);
    justifyAct->setShortcut(tr("Ctrl+J"));
    justifyAct->setStatusTip(tr("Justify the selected text"));
    connect(justifyAct, &QAction::triggered, this, &MainWindow::justify);

    centerAct = new QAction(tr("&Center"), this);
    centerAct->setCheckable(true);
    centerAct->setShortcut(tr("Ctrl+E"));
    centerAct->setStatusTip(tr("Center the selected text"));
    connect(centerAct, &QAction::triggered, this, &MainWindow::center);

    alignmentGroup = new QActionGroup(this);
    alignmentGroup->addAction(leftAlignAct);
    alignmentGroup->addAction(rightAlignAct);
    alignmentGroup->addAction(justifyAct);
    alignmentGroup->addAction(centerAct);
    leftAlignAct->setChecked(true);
}

void MainWindow::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(newAct);
    fileMenu->addAction(openAct);
    fileMenu->addAction(saveAct);
    fileMenu->addAction(printAct);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAct);

    editMenu = menuBar()->addMenu(tr("&Edit"));
    editMenu->addAction(undoAct);
    editMenu->addAction(redoAct);
    editMenu->addSeparator();
    editMenu->addAction(cutAct);
    editMenu->addAction(copyAct);
    editMenu->addAction(pasteAct);
    editMenu->addSeparator();

    helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(aboutAct);
    helpMenu->addAction(aboutQtAct);

    optionsMenu = menuBar()->addMenu(tr("&Options"));

#if defined (Q_OS_DARWIN)
    helpMenu->addSeparator();
    helpMenu->addAction(preferencesAct);
#else
    optionsMenu->addAction(preferencesAct);
#endif

    formatMenu = editMenu->addMenu(tr("&Format"));
    formatMenu->addAction(boldAct);
    formatMenu->addAction(italicAct);
    formatMenu->addSeparator()->setText(tr("Alignment"));
    formatMenu->addAction(leftAlignAct);
    formatMenu->addAction(rightAlignAct);
    formatMenu->addAction(justifyAct);
    formatMenu->addAction(centerAct);
    formatMenu->addSeparator();
    formatMenu->addAction(setLineSpacingAct);
    formatMenu->addAction(setParagraphSpacingAct);
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
