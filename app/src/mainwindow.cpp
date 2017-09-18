#include <fit.h>
#include <zipper.h>
#include <string.h>
#include <titlebar.h>
#include <covermenu.h>
#include <listwidget.h>
#include <mainwindow.h>
#include <mainwindow_p.h>
#include <container.h>
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
#include <QtWidgets>
#include <QtNetwork>

#define CUSTOM_ITEM "\
import QtQuick 2.0\n\
\n\
Rectangle {\n\
   id: customItem\n\
   width: 54\n\
   height: 54\n\
   radius: 5 * dpi\n\
   color: \"#c7cbc4\"\n\
}\n"

#if defined(Q_OS_IOS)
#include <ios.h>
#endif

using namespace Fit;
MainWindowPrivate* MainWindow::_d = nullptr;

MainWindow::MainWindow(QWidget *parent)
	: QWidget(parent)
    , m_RightMenu(new CoverMenu)
    , m_LeftMenu(new CoverMenu)
{
    if (_d) return;
    _d = new MainWindowPrivate(this);
    _d->setupUi();
	SetupGui();
	QTimer::singleShot(300, [=] { SetupManagers(); });
}

void MainWindow::SetupGui()
{
	/* Add Tool Menu */
    m_RightMenu->setCoverWidget(_d->centralWidget);
    m_RightMenu->setCoverSide(CoverMenu::FromRight);
    connect(this,SIGNAL(resized()),m_RightMenu,SLOT(hide()));
    connect(this,&MainWindow::resized, [this] { _d->titleBar->setMenuChecked(false); });

	/* Add Properties Menu */
    m_LeftMenu->setCoverWidget(_d->centralWidget);
    m_LeftMenu->setCoverSide(CoverMenu::FromLeft);
    connect(this,SIGNAL(resized()),m_LeftMenu,SLOT(hide()));
    connect(this,&MainWindow::resized, [this] { _d->titleBar->setSettingsChecked(false); });

	/* Add Title Bar */
    fit(_d->titleBar, Fit::Height, true);
    _d->titleBar->setText("Objectwheel Studio");
    _d->titleBar->setColor("#0D74C8");
    _d->titleBar->setShadowColor("#EAEEF1");
    connect(_d->titleBar, SIGNAL(MenuToggled(bool)), m_RightMenu, SLOT(setCovered(bool)));
    connect(_d->titleBar, SIGNAL(SettingsToggled(bool)), m_LeftMenu, SLOT(setCovered(bool)));
    connect(m_RightMenu, SIGNAL(toggled(bool)), _d->titleBar, SLOT(setMenuChecked(bool)));
    connect(m_LeftMenu, SIGNAL(toggled(bool)), _d->titleBar, SLOT(setSettingsChecked(bool)));

	/* Init Left Container */
	QVariant toolboxVariant;
    toolboxVariant.setValue<QWidget*>(_d->toolboxWidget);
	QVariant propertiesVariant;
    propertiesVariant.setValue<QWidget*>(_d->propertiesWidget);
    QVariant formsVariant;
    formsVariant.setValue<QWidget*>(_d->formsWidget);
	Container* leftContainer = new Container;
    leftContainer->addWidget(_d->toolboxWidget);
    leftContainer->addWidget(_d->propertiesWidget);
    leftContainer->addWidget(_d->formsWidget);
	leftContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	QToolBar* leftToolbar = new QToolBar;
	leftToolbar->setStyleSheet(CSS::Toolbar);
	leftToolbar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
	leftToolbar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    leftToolbar->resize(leftToolbar->width(), 43);
	fit(leftToolbar, Fit::Height, true);
	QGraphicsDropShadowEffect* toolbarShadowEffect = new QGraphicsDropShadowEffect;
    toolbarShadowEffect->setBlurRadius(fit(4));
    toolbarShadowEffect->setOffset(0, fit(1));
    toolbarShadowEffect->setColor(QColor(0, 0, 0, 50));
	leftToolbar->setGraphicsEffect(toolbarShadowEffect);

	QRadioButton* toolboxButton = new QRadioButton;
	toolboxButton->setStyleSheet(CSS::ToolboxButton);
	toolboxButton->setCheckable(true);
	toolboxButton->setChecked(true);
	toolboxButton->setCursor(Qt::PointingHandCursor);
	QWidgetAction* toolboxButtonAction = new QWidgetAction(this);
	toolboxButtonAction->setDefaultWidget(toolboxButton);
	toolboxButtonAction->setData(toolboxVariant);
	toolboxButtonAction->setCheckable(true);
	leftToolbar->addAction(toolboxButtonAction);
	connect(toolboxButton, SIGNAL(clicked(bool)), toolboxButtonAction, SLOT(trigger()));
	connect(toolboxButtonAction, SIGNAL(triggered(bool)), leftContainer, SLOT(handleAction()));

	QRadioButton* propertiesButton = new QRadioButton;
	propertiesButton->setCursor(Qt::PointingHandCursor);
	propertiesButton->setStyleSheet(CSS::PropertiesButton);
	propertiesButton->setCheckable(true);
	QWidgetAction* propertiesButtonAction = new QWidgetAction(this);
	propertiesButtonAction->setDefaultWidget(propertiesButton);
	propertiesButtonAction->setData(propertiesVariant);
	propertiesButtonAction->setCheckable(true);
	leftToolbar->addAction(propertiesButtonAction);
	connect(propertiesButton, SIGNAL(clicked(bool)), propertiesButtonAction, SLOT(trigger()));
	connect(propertiesButtonAction, SIGNAL(triggered(bool)), leftContainer, SLOT(handleAction()));

    QRadioButton* formsButton = new QRadioButton;
    formsButton->setCursor(Qt::PointingHandCursor);
    formsButton->setStyleSheet(CSS::FormsButton);
    formsButton->setCheckable(true);
    QWidgetAction* formsButtonAction = new QWidgetAction(this);
    formsButtonAction->setDefaultWidget(formsButton);
    formsButtonAction->setData(formsVariant);
    formsButtonAction->setCheckable(true);
    leftToolbar->addAction(formsButtonAction);
    connect(formsButton, SIGNAL(clicked(bool)), formsButtonAction, SLOT(trigger()));
    connect(formsButtonAction, SIGNAL(triggered(bool)), leftContainer, SLOT(handleAction()));

	QWidget* leftMenuWidget = new QWidget;
	leftMenuWidget->setObjectName("leftMenuWidget");
    leftMenuWidget->setStyleSheet("#leftMenuWidget{background:#52616D;}");

	QVBoxLayout* leftMenuLayout = new QVBoxLayout(leftMenuWidget);
	leftMenuLayout->setContentsMargins(0, 0, 0, 0);
	leftMenuLayout->setSpacing(fit(8));
	leftMenuLayout->addWidget(leftToolbar);
	leftMenuLayout->addWidget(leftContainer);
	m_LeftMenu->attachWidget(leftMenuWidget);

    _d->qmlEditor = new QmlEditor(this);
    _d->qmlEditor->setHidden(true);

    QObject::connect(_d->toolboxList,(void(ListWidget::*)(int))(&ListWidget::currentRowChanged),[=](int i){
        if (i>=0) {
           DesignManager::formScene()->clearSelection();
           DesignManager::controlScene()->clearSelection();
        }
    });

    _d->aboutWidget = new About(this);
    _d->buildsScreen = new BuildsScreen(this);

    QWidget* sceneListWidget = new QWidget;
    sceneListWidget->setStyleSheet("background:#52616D; border:none;");
    m_RightMenu->attachWidget(sceneListWidget);

	QVBoxLayout* sceneListWidgetLayout = new QVBoxLayout(sceneListWidget);
    sceneListWidgetLayout->setSpacing(fit(10));
    sceneListWidgetLayout->setContentsMargins(0, 0, 0, fit(8));

    QGraphicsDropShadowEffect* sceneListTitleShadowEffect = new QGraphicsDropShadowEffect;
    sceneListTitleShadowEffect->setBlurRadius(fit(3));
    sceneListTitleShadowEffect->setOffset(0, fit(1));
    sceneListTitleShadowEffect->setColor(QColor(0, 0, 0, 40));

    QLabel* sceneListTitle = new QLabel;
	sceneListTitle->setText("◉ Menu");
	sceneListTitle->setAlignment(Qt::AlignCenter);
	sceneListTitle->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    sceneListTitle->setFixedHeight(fit(43));
    sceneListTitle->setGraphicsEffect(sceneListTitleShadowEffect);
    sceneListTitle->setStyleSheet(QString("color: white; background:qlineargradient(spread:pad, x1:0.5, y1:0, x2:0.5, y2:1, stop:0 %1, stop:1 %2);").arg(QColor("#C61717").name()).arg(QColor("#C61717").darker(115).name()));

    _d->toolboxList->indicatorButton()->setIcon(QIcon(":/resources/images/right-arrow.png"));
    _d->toolboxList->indicatorButton()->setColor(QColor("#0D74C8"));
    _d->toolboxList->indicatorButton()->setRadius(fit(7));
    _d->toolboxList->indicatorButton()->setIconSize(QSize(fit(10), fit(10)));
    _d->toolboxList->indicatorButton()->resize(fit(15), fit(15));
    connect(_d->toolboxList->indicatorButton(), &FlatButton::clicked, [=] {
        auto previousControl = DesignManager::controlScene()->mainControl();
        if (previousControl)
            previousControl->deleteLater();
        auto url = _d->toolboxList->GetUrls(_d->toolboxList->currentItem())[0];
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

    auto secureExitButton = new FlatButton;
    secureExitButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    secureExitButton->setMinimumSize(fit(150), fit(35));
    secureExitButton->setMaximumSize(fit(150), fit(35));
    secureExitButton->setText("Secure Exit");
    secureExitButton->setObjectName(QStringLiteral("secureExitButton"));
    secureExitButton->setIcon(QIcon(":/resources/images/exit.png"));
    secureExitButton->setColor(QColor("#0078D7"));
    secureExitButton->setTextColor(Qt::white);
    connect(secureExitButton, SIGNAL(clicked(bool)), this, SLOT(on_secureExitButton_clicked()));

    sceneListWidgetLayout->addWidget(sceneListTitle);
    sceneListWidgetLayout->addWidget(_d->sceneList);
    sceneListWidgetLayout->addWidget(secureExitButton);
    sceneListWidgetLayout->setAlignment(secureExitButton, Qt::AlignHCenter);

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

void MainWindow::SetupManagers()
{
	//Let's add some custom controls to that project
    ToolsManager::setListWidget(_d->toolboxList);
    auto userManager = new UserManager(this); //create new user manager
    Q_UNUSED(userManager);
    auto* projectManager = new ProjectManager(this); //create new project manager
	projectManager->setMainWindow(this);
    new SaveManager(this);
    new QmlPreviewer(this);
	auto sceneManager = new SceneManager;
    sceneManager->setMainWindow(this);
    sceneManager->setSceneListWidget(_d->sceneList);
    sceneManager->addScene("studioScene", _d->centralWidget);
    sceneManager->addScene("projectsScene", _d->projectsScreen);
    sceneManager->addScene("loginScene", _d->loginScreen);
    sceneManager->addScene("aboutScene", _d->aboutWidget);
    sceneManager->addScene("buildsScene", _d->buildsScreen);
    sceneManager->setCurrent("loginScene", false);

    connect(SaveManager::instance(), SIGNAL(parserRunningChanged(bool)), SLOT(handleIndicatorChanges()));
    connect(QmlPreviewer::instance(), SIGNAL(workingChanged(bool)), SLOT(handleIndicatorChanges()));

    SplashScreen::raise();
	connect(sceneManager, (void(SceneManager::*)(const QString&))(&SceneManager::currentSceneChanged),
            [=](const QString& /*key*/){
		m_RightMenu->hide();
        _d->titleBar->setMenuChecked(false);
		m_LeftMenu->hide();
        _d->titleBar->setSettingsChecked(false);
	});
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
		QListWidgetItem* item = new QListWidgetItem(QIcon(iconPath), sceneName);
        _d->sceneList->insertItem(0, item);
        _d->sceneList->AddUrls(item, urls);
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
void MainWindow::resizeEvent(QResizeEvent* event)
{
    _d->qmlEditor->setGeometry(0, 0, width(), height());
    QWidget::resizeEvent(event);
	emit resized();
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

void MainWindow::on_secureExitButton_clicked()
{
    SplashScreen::setText("Stopping user session");
    SplashScreen::show(true);
    UserManager::clearAutoLogin();
    auto ret = QtConcurrent::run(&UserManager::stopUserSession);
    Delayer::delay(&ret, &QFuture<void>::isRunning);
    SplashScreen::hide();
    SplashScreen::setText("Loading");
    SceneManager::show("loginScene", SceneManager::ToLeft);
}

void MainWindow::handleToolboxUrlboxChanges(const QString& text)
{
	auto url = QUrl::fromUserInput(text);
	auto pixmap = DownloadPixmap(url);
	if (pixmap.isNull()) return;
    auto icon = dname(_d->toolboxList->GetUrls(_d->toolboxList->currentItem())[0].toLocalFile()) + "/icon.png";
	QByteArray bArray;
	QBuffer buffer(&bArray);
	buffer.open(QIODevice::WriteOnly);
	if (!pixmap.save(&buffer,"PNG")) return;
	buffer.close();
	if (!wrfile(icon, bArray)) return;
    _d->toolboxList->currentItem()->setIcon(QIcon(icon));
}

void MainWindow::handleToolboxNameboxChanges(QString /*name*/)
{
//	if (name == _d->toolboxList->currentItem()->text() || name == "") return;

//	int count = 1;
//	for (int i = 0; i < _d->toolboxList->count(); i++) {
//		if (_d->toolboxList->item(i)->text() == name) {
//			if (count > 1) {
//				name.remove(name.size() - 1, 1);
//			}
//			i = -1;
//			count++;
//			name += QString::number(count);
//		}
//	}

//	auto from = ToolsManager::toolsDir() + "/" + _d->toolboxList->currentItem()->text();
//	auto to = ToolsManager::toolsDir() + "/" + name;
//	if (!rn(from, to)) qFatal("MainWindow : Error occurred");

//	_d->toolboxList->currentItem()->setText(name);

//	QList<QUrl> urls;
//	urls << QUrl::fromLocalFile(to + "/main.qml");
//	_d->toolboxList->RemoveUrls(_d->toolboxList->currentItem());
//	_d->toolboxList->AddUrls(_d->toolboxList->currentItem(),urls);

//	for (int i = 0; i < _d->m_ItemUrls.size(); i++) {
//		if (_d->m_ItemUrls[i].toLocalFile() == (from+"/main.qml")) {
//			_d->m_ItemUrls[i] = QUrl::fromLocalFile(to+"/main.qml");
//		}
//	}

//	_d->qmlEditor->updateCacheForRenamedEntry(from, to, true);
}

void MainWindow::toolboxEditButtonToggled(bool checked)
{
	if (checked) {
        _d->showAdderArea();
	} else {
        _d->hideAdderArea();
	}
}

void MainWindow::toolboxRemoveButtonClicked()
{
//	if (_d->toolboxList->currentRow() < 0) return;
//	auto name = _d->toolboxList->currentItem()->text();
//	QMessageBox msgBox;
//	msgBox.setText(QString("<b>This will remove %1 from Tool Library and Dashboard.</b>").arg(name));
//	msgBox.setInformativeText("Do you want to continue?");
//	msgBox.setStandardButtons(QMessageBox::No | QMessageBox::Yes);
//	msgBox.setDefaultButton(QMessageBox::No);
//	msgBox.setIcon(QMessageBox::Warning);
//	const int ret = msgBox.exec();
//	switch (ret) {
//		case QMessageBox::Yes: {
//			_d->qmlEditor->clearCacheFor(ToolsManager::toolsDir() + separator() + name, true);
//			rm(ToolsManager::toolsDir() + separator() + name);
//			_d->toolboxList->RemoveUrls(_d->toolboxList->currentItem());
//			delete _d->toolboxList->takeItem(_d->toolboxList->currentRow());

//			for (int i = 0; i < _d->m_ItemUrls.size(); i++) {
//				if (_d->m_ItemUrls[i].toLocalFile() == (ToolsManager::toolsDir() + separator() + name + "/main.qml")) {
//					auto items = GetAllChildren(_d->m_Items[i]);
//					for (auto item : items) {
//						if (_d->m_Items.contains(item)) {
//							SaveManager::removeSave(_d->designWidget->rootContext()->nameForObject(item));
//							SaveManager::removeParentalRelationship(_d->designWidget->rootContext()->nameForObject(item));
//							_d->designWidget->rootContext()->setContextProperty(
//										_d->designWidget->rootContext()->nameForObject(item), 0);
//							int j = _d->m_Items.indexOf(item);
//							_d->m_Items.removeOne(item);
//							_d->m_ItemUrls.removeAt(j);
//							item->deleteLater();
//						}
//					}
//				}
//			}
//			HideSelectionTools();
//			break;
//		} default: {
//			// Do nothing
//			break;
//		}
//	}
}

void MainWindow::toolboxAddButtonClicked()
{
	int count = 1;
	auto name = QString("Item%1").arg(count);
    for (int i = 0; i < _d->toolboxList->count(); i++) {
        if (_d->toolboxList->item(i)->text() == name) {
			name.remove(name.size() - 1, 1);
			i = -1;
			count++;
			name += QString::number(count);
		}
	}

	auto filePath = ToolsManager::toolsDir() + "/" + name + "/main.qml";
	int ret = wrfile(filePath, CUSTOM_ITEM);
	if (ret < 0) return;

	QPixmap pixmap;
	QString iconPath = ToolsManager::toolsDir() + "/" + name + "/icon.png";
	pixmap.load(":/resources/images/item.png");
	if (!pixmap.save(iconPath)) {
		return;
	}

	QList<QUrl> urls;
	QListWidgetItem* item = new QListWidgetItem(QIcon(iconPath), name);
	urls << QUrl::fromLocalFile(filePath);
    _d->toolboxList->insertItem(0, item);
    _d->toolboxList->AddUrls(item,urls);
    _d->toolboxList->setCurrentRow(0);
    _d->toolboxEditButton->setChecked(true);
}

void MainWindow::toolboxResetButtonClicked()
{
//	QMessageBox msgBox;
//	msgBox.setText("<b>This will clear Dashboard and reset Tool Library.</b>");
//	msgBox.setInformativeText("Do you want to continue?");
//	msgBox.setStandardButtons(QMessageBox::No | QMessageBox::Yes);
//	msgBox.setDefaultButton(QMessageBox::No);
//	msgBox.setIcon(QMessageBox::Warning);
//	const int ret = msgBox.exec();
//	switch (ret) {
//		case QMessageBox::Yes: {
//			_d->toolboxList->ClearUrls();
//			_d->toolboxList->clear();
//			_d->qmlEditor->clearCache();
//			ToolsManager::resetTools();

//			for (auto item : _d->m_Items) {
//				SaveManager::removeSave(_d->designWidget->rootContext()->nameForObject(item));
//				SaveManager::removeParentalRelationship(_d->designWidget->rootContext()->nameForObject(item));
//				_d->designWidget->rootContext()->setContextProperty(
//							_d->designWidget->rootContext()->nameForObject(item), 0);
//				item->deleteLater();
//			}
//			_d->m_Items.clear();
//			_d->m_ItemUrls.clear();
//			HideSelectionTools();
//			_d->toolboxList->setCurrentRow(-1);
//			break;
//		} default: {
//			// Do nothing
//			break;
//		}
//	}
}

void MainWindow::toolboxImportButtonClicked()
{
	QFileDialog dialog(this);
	dialog.setFileMode(QFileDialog::ExistingFiles);
	dialog.setNameFilter(tr("Zip files (*.zip)"));
	dialog.setViewMode(QFileDialog::Detail);
	if (dialog.exec()) {
        handleImports(dialog.selectedFiles());
        QMessageBox::information(this, "Finished", "Tool import has successfully finished.");
	}
}

void MainWindow::toolboxExportButtonClicked()
{
	QFileDialog dialog(this);
	dialog.setFileMode(QFileDialog::Directory);
	dialog.setViewMode(QFileDialog::Detail);
    dialog.setOption(QFileDialog::ShowDirsOnly, true);
	if (dialog.exec()) {
        auto dir = dname(_d->toolboxList->GetUrls(_d->toolboxList->currentItem())[0].toLocalFile());
        auto toolName = _d->toolboxList->currentItem()->text();
        if (!rm(dialog.selectedFiles().at(0) + separator() + toolName + ".zip")) return;
		Zipper::compressDir(dir, dialog.selectedFiles().at(0) + separator() + toolName + ".zip");
        QMessageBox::information(this, "Finished", "Tool export has successfully finished.");
	}
}

void MainWindow::handleImports(const QStringList& fileNames)
{
	for (auto fileName : fileNames) {
		auto name = fname(fileName.remove(fileName.size() - 4, 4));
		int count = 1;
        for (int i = 0; i < _d->toolboxList->count(); i++) {
            if (_d->toolboxList->item(i)->text() == name) {
				if (count > 1) {
					name.remove(name.size() - 1, 1);
				}
				i = -1;
				count++;
				name += QString::number(count);
			}
		}

		QFile file(fileName + ".zip");
		if (!file.open(QFile::ReadOnly)) return;
		if (!QDir(ToolsManager::toolsDir()).mkpath(name)) return;
		Zipper::extractZip(file.readAll(), ToolsManager::toolsDir() + "/" + name);
		file.close();
		ToolsManager::addTool(name);
	}
}

const QPixmap MainWindow::DownloadPixmap(const QUrl& url)
{
	QPixmap pixmap;
	QEventLoop loop;
	QNetworkAccessManager manager;

	QNetworkReply *reply = manager.get(QNetworkRequest(url));
	QObject::connect(reply, &QNetworkReply::finished, &loop, [&reply, &pixmap, &loop](){
		if (reply->error() == QNetworkReply::NoError) {
			pixmap.loadFromData(reply->readAll());
		}
		loop.quit();
	});

	loop.exec();
	return pixmap;
}

MainWindow::~MainWindow()
{
    delete _d;
}
