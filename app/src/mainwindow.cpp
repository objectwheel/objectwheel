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
#include <QtQml>
#include <QtCore>
#include <QtQuick>
#include <QtWidgets>
#include <QtNetwork>
#include <filemanager.h>
#include <QFileInfo>
#include <projectmanager.h>
#include <usermanager.h>
#include <toolsmanager.h>
#include <savemanager.h>
#include <splashscreen.h>
#include <scenemanager.h>
#include <QtConcurrent>
#include <delayer.h>
#include <control.h>
#include <formscene.h>

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
MainWindowPrivate* MainWindow::m_d = nullptr;

MainWindow::MainWindow(QWidget *parent)
	: QWidget(parent)
    , m_RightMenu(new CoverMenu)
    , m_LeftMenu(new CoverMenu)
{
	if (m_d) return;
	m_d = new MainWindowPrivate(this);
    m_d->setupUi();
	SetupGui();
	QTimer::singleShot(300, [=] { SetupManagers(); });
}

void MainWindow::SetupGui()
{
	/* Add Tool Menu */
    m_RightMenu->setCoverWidget(m_d->centralWidget);
    m_RightMenu->setCoverSide(CoverMenu::FromRight);
    connect(this,SIGNAL(resized()),m_RightMenu,SLOT(hide()));
    connect(this,&MainWindow::resized, [this] { m_d->titleBar->setMenuChecked(false); });

	/* Add Properties Menu */
    m_LeftMenu->setCoverWidget(m_d->centralWidget);
    m_LeftMenu->setCoverSide(CoverMenu::FromLeft);
    connect(this,SIGNAL(resized()),m_LeftMenu,SLOT(hide()));
    connect(this,&MainWindow::resized, [this] { m_d->titleBar->setSettingsChecked(false); });

	/* Add Title Bar */
	fit(m_d->titleBar, Fit::Height, true);
	m_d->titleBar->setText("Objectwheel Studio");
    m_d->titleBar->setColor("#0D74C8");
    m_d->titleBar->setShadowColor("#EAEEF1");
	connect(m_d->titleBar, SIGNAL(MenuToggled(bool)), m_RightMenu, SLOT(setCovered(bool)));
	connect(m_d->titleBar, SIGNAL(SettingsToggled(bool)), m_LeftMenu, SLOT(setCovered(bool)));
	connect(m_RightMenu, SIGNAL(toggled(bool)), m_d->titleBar, SLOT(setMenuChecked(bool)));
	connect(m_LeftMenu, SIGNAL(toggled(bool)), m_d->titleBar, SLOT(setSettingsChecked(bool)));

	/* Prepare Properties Widget */
//	connect(this, SIGNAL(selectionShowed(QObject*const)), m_d->propertiesWidget, SLOT(refreshList(QObject*const)));
//	connect(this, &MainWindow::selectionHided, [this] { m_d->propertiesWidget->setDisabled(true); });

	/* Init Left Container */
	QVariant toolboxVariant;
	toolboxVariant.setValue<QWidget*>(m_d->toolboxWidget);
	QVariant propertiesVariant;
	propertiesVariant.setValue<QWidget*>(m_d->propertiesWidget);
    QVariant formsVariant;
    formsVariant.setValue<QWidget*>(m_d->formsWidget);
	Container* leftContainer = new Container;
	leftContainer->addWidget(m_d->toolboxWidget);
	leftContainer->addWidget(m_d->propertiesWidget);
    leftContainer->addWidget(m_d->formsWidget);
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

//	m_d->propertiesWidget->setRootContext(m_d->designWidget->rootContext());
//	m_d->propertiesWidget->setItemSource(&m_d->m_Items);
//    m_d->propertiesWidget->setUrlList(&m_d->m_ItemUrls);

	m_d->qmlEditor = new QmlEditor(this);
	m_d->qmlEditor->setHidden(true);
//	m_d->qmlEditor->setItems(&m_d->m_Items, &m_d->m_ItemUrls);
//	m_d->qmlEditor->setRootContext(m_d->designWidget->rootContext());
//	connect(this, SIGNAL(selectionShowed(QObject*const)), m_d->qmlEditor, SLOT(selectItem(QObject*const)));
//	connect(m_d->bubbleHead, SIGNAL(moved(QPoint)), m_d->qmlEditor, SLOT(setShowCenter(QPoint)));

    QObject::connect(m_d->toolboxList,(void(ListWidget::*)(int))(&ListWidget::currentRowChanged),[=](int i){
        if (i>=0) {
           DesignManager::formScene()->clearSelection();
           DesignManager::controlScene()->clearSelection();
        }
    });

	m_d->aboutWidget = new About(this);
    m_d->buildsScreen = new BuildsScreen(this);

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
    sceneListWidgetLayout->addWidget(m_d->sceneList);
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
    ToolsManager::setListWidget(m_d->toolboxList);
	auto userManager = new UserManager(this); //create new user manager
    auto* projectManager = new ProjectManager(this); //create new project manager
	projectManager->setMainWindow(this);
    new SaveManager(this);
	auto sceneManager = new SceneManager;
    sceneManager->setMainWindow(this);
	sceneManager->setSceneListWidget(m_d->sceneList);
	sceneManager->addScene("studioScene", m_d->centralWidget);
    sceneManager->addScene("projectsScene", m_d->projectsScreen);
    sceneManager->addScene("loginScene", m_d->loginScreen);
    sceneManager->addScene("aboutScene", m_d->aboutWidget);
    sceneManager->addScene("buildsScene", m_d->buildsScreen);
    sceneManager->setCurrent("loginScene", false);

    SplashScreen::raise();
	connect(sceneManager, (void(SceneManager::*)(const QString&))(&SceneManager::currentSceneChanged),
			[=](const QString& key){
		m_RightMenu->hide();
		m_d->titleBar->setMenuChecked(false);
		m_LeftMenu->hide();
		m_d->titleBar->setSettingsChecked(false);
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
		m_d->sceneList->insertItem(0, item);
		m_d->sceneList->AddUrls(item, urls);
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
	m_d->qmlEditor->setGeometry(0, 0, width(), height());
	QWidget::resizeEvent(event);
	emit resized();
}

void MainWindow::handleCurrentFormChanges(const QVariant& CurrentForm, const QVariant& index)
{
//	m_CurrentForm = qobject_cast<QQuickItem*>(CurrentForm.value<QObject*>());
//	if (!m_CurrentForm) qFatal("MainForm : Error occurred");
//    m_d->formsWidget->setCurrentForm(index.toInt());
}

void MainWindow::handleEditorOpenButtonClicked()
{
//    if(m_ResizerTick->isVisible()) {
//        m_d->qmlEditor->show();
//    } else {
//        if (m_d->toolboxList->currentRow() < 0) return;
//        auto cItem = m_d->toolboxList->currentItem();
//		m_d->qmlEditor->setRootFolder(ToolsManager::toolsDir() + separator() + m_d->toolboxList->currentItem()->text());
//        m_d->qmlEditor->show(m_d->toolboxList->GetUrls(cItem)[0].toLocalFile());
//	}
//    m_d->qmlEditor->raise();
}

void MainWindow::cleanupObjectwheel()
{
    while(SaveManager::inprogress())
        Delayer::delay(100);

    UserManager::stopUserSession();

    qApp->processEvents();
}

void MainWindow::clearStudio()
{
//	m_d->toolboxList->ClearUrls();
//	m_d->toolboxList->clear();
//    m_d->qmlEditor->clearEditor();

//	for (auto item : m_d->m_Items) {
//		m_d->designWidget->rootContext()->setContextProperty(
//					m_d->designWidget->rootContext()->nameForObject(item), 0);
//		item->deleteLater();
//	}

//	auto forms = m_d->formsWidget->forms();
//	for (int i = forms.count(); i--;) {
//		if (forms[i] != m_CurrentWindow) {
//			m_d->formsWidget->removeWindowWithoutSave(m_d->designWidget->rootContext()->nameForObject(forms[i]));
//		}
//	}
//	QString page1Name = "page1";
//	m_d->formsWidget->changeWindowWithoutSave(m_d->designWidget->rootContext()->nameForObject(m_CurrentWindow), page1Name);

//	m_d->m_Items.clear();
//	m_d->m_ItemUrls.clear();
//	HideSelectionTools();
//    if (m_d->bubbleHead->isChecked()) m_d->bubbleHead->click();
//    QTimer::singleShot(450, [=]{ m_d->bubbleHead->move(width() - fit(72), height() - fit(72)); });
//    m_d->toolboxList->setCurrentRow(-1);
//    Delayer::delay(100);
}

void MainWindow::on_buildsButton_clicked()
{
    SceneManager::show("buildsScene", SceneManager::ToLeft);
}

void MainWindow::on_clearButton_clicked()
{
    if (DesignManager::currentScene()->mainControl()->childControls().size() < 1)
        return;

    QMessageBox msgBox;
    msgBox.setText("<b>This will clear the current form's content.</b>");
    msgBox.setInformativeText("Do you want to continue?");
    msgBox.setStandardButtons(QMessageBox::No | QMessageBox::Yes);
    msgBox.setDefaultButton(QMessageBox::No);
    msgBox.setIcon(QMessageBox::Warning);

    switch (msgBox.exec()) {
        case QMessageBox::Yes:
            DesignManager::currentScene()->mainControl()->cleanContent();
            break;
        default:
            break;
    }
}

void MainWindow::on_playButton_clicked()
{
//    static bool editModeState;
//    editModeState = m_d->editMode;
//    if (m_d->editMode) { // Edit mode on
//        for (auto item : m_d->m_Items) {
//            item->setEnabled(true);
//        }
//        HideSelectionTools();
//        m_d->editMode = false;
//        m_d->emIndicator->setOn(false);
//    }
//    m_d->designWidget->setParent(this);
//    SceneManager::addScene("playScene", m_d->designWidget);
//    SceneManager::setCurrent("playScene");

//    FlatButton* exitButton = new FlatButton;
//    exitButton->setParent(m_d->designWidget);
//    exitButton->setIconButton(true);
//    exitButton->setIcon(QIcon(":/resources/images/delete-icon.png"));
//#if defined(Q_OS_IOS) || defined(Q_OS_ANDROID) || defined(Q_OS_WINPHONE)
//    exitButton->setGeometry(width() - fit(26), fit(8), fit(18), fit(18));
//#else
//    exitButton->setGeometry(width() - fit(15), fit(5), fit(8), fit(8));
//#endif
//    auto connection = connect(this, &MainWindow::resized, [=]{
//#if defined(Q_OS_IOS) || defined(Q_OS_ANDROID) || defined(Q_OS_WINPHONE)
//    exitButton->setGeometry(width() - fit(26), fit(8), fit(18), fit(18));
//#else
//    exitButton->setGeometry(width() - fit(15), fit(5), fit(8), fit(8));
//#endif
//    });
//    exitButton->show();

//    connect(exitButton, &FlatButton::clicked, [=]{
//        disconnect(connection);
//        exitButton->deleteLater();
//        m_d->verticalLayout->insertWidget(1, m_d->designWidget);
//        SceneManager::removeScene("playScene");
//        SceneManager::setCurrent("studioScene");
//        m_d->editMode = editModeState;
//        m_d->emIndicator->setOn(m_d->editMode);
//    });
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
	auto icon = dname(m_d->toolboxList->GetUrls(m_d->toolboxList->currentItem())[0].toLocalFile()) + "/icon.png";
	QByteArray bArray;
	QBuffer buffer(&bArray);
	buffer.open(QIODevice::WriteOnly);
	if (!pixmap.save(&buffer,"PNG")) return;
	buffer.close();
	if (!wrfile(icon, bArray)) return;
	m_d->toolboxList->currentItem()->setIcon(QIcon(icon));
}

void MainWindow::handleToolboxNameboxChanges(QString name)
{
//	if (name == m_d->toolboxList->currentItem()->text() || name == "") return;

//	int count = 1;
//	for (int i = 0; i < m_d->toolboxList->count(); i++) {
//		if (m_d->toolboxList->item(i)->text() == name) {
//			if (count > 1) {
//				name.remove(name.size() - 1, 1);
//			}
//			i = -1;
//			count++;
//			name += QString::number(count);
//		}
//	}

//	auto from = ToolsManager::toolsDir() + "/" + m_d->toolboxList->currentItem()->text();
//	auto to = ToolsManager::toolsDir() + "/" + name;
//	if (!rn(from, to)) qFatal("MainWindow : Error occurred");

//	m_d->toolboxList->currentItem()->setText(name);

//	QList<QUrl> urls;
//	urls << QUrl::fromLocalFile(to + "/main.qml");
//	m_d->toolboxList->RemoveUrls(m_d->toolboxList->currentItem());
//	m_d->toolboxList->AddUrls(m_d->toolboxList->currentItem(),urls);

//	for (int i = 0; i < m_d->m_ItemUrls.size(); i++) {
//		if (m_d->m_ItemUrls[i].toLocalFile() == (from+"/main.qml")) {
//			m_d->m_ItemUrls[i] = QUrl::fromLocalFile(to+"/main.qml");
//		}
//	}

//	m_d->qmlEditor->updateCacheForRenamedEntry(from, to, true);
}

void MainWindow::toolboxEditButtonToggled(bool checked)
{
	if (checked) {
		m_d->showAdderArea();
	} else {
		m_d->hideAdderArea();
	}
}

void MainWindow::toolboxRemoveButtonClicked()
{
//	if (m_d->toolboxList->currentRow() < 0) return;
//	auto name = m_d->toolboxList->currentItem()->text();
//	QMessageBox msgBox;
//	msgBox.setText(QString("<b>This will remove %1 from Tool Library and Dashboard.</b>").arg(name));
//	msgBox.setInformativeText("Do you want to continue?");
//	msgBox.setStandardButtons(QMessageBox::No | QMessageBox::Yes);
//	msgBox.setDefaultButton(QMessageBox::No);
//	msgBox.setIcon(QMessageBox::Warning);
//	const int ret = msgBox.exec();
//	switch (ret) {
//		case QMessageBox::Yes: {
//			m_d->qmlEditor->clearCacheFor(ToolsManager::toolsDir() + separator() + name, true);
//			rm(ToolsManager::toolsDir() + separator() + name);
//			m_d->toolboxList->RemoveUrls(m_d->toolboxList->currentItem());
//			delete m_d->toolboxList->takeItem(m_d->toolboxList->currentRow());

//			for (int i = 0; i < m_d->m_ItemUrls.size(); i++) {
//				if (m_d->m_ItemUrls[i].toLocalFile() == (ToolsManager::toolsDir() + separator() + name + "/main.qml")) {
//					auto items = GetAllChildren(m_d->m_Items[i]);
//					for (auto item : items) {
//						if (m_d->m_Items.contains(item)) {
//							SaveManager::removeSave(m_d->designWidget->rootContext()->nameForObject(item));
//							SaveManager::removeParentalRelationship(m_d->designWidget->rootContext()->nameForObject(item));
//							m_d->designWidget->rootContext()->setContextProperty(
//										m_d->designWidget->rootContext()->nameForObject(item), 0);
//							int j = m_d->m_Items.indexOf(item);
//							m_d->m_Items.removeOne(item);
//							m_d->m_ItemUrls.removeAt(j);
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
	for (int i = 0; i < m_d->toolboxList->count(); i++) {
		if (m_d->toolboxList->item(i)->text() == name) {
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
	m_d->toolboxList->insertItem(0, item);
	m_d->toolboxList->AddUrls(item,urls);
	m_d->toolboxList->setCurrentRow(0);
	m_d->toolboxEditButton->setChecked(true);
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
//			m_d->toolboxList->ClearUrls();
//			m_d->toolboxList->clear();
//			m_d->qmlEditor->clearCache();
//			ToolsManager::resetTools();

//			for (auto item : m_d->m_Items) {
//				SaveManager::removeSave(m_d->designWidget->rootContext()->nameForObject(item));
//				SaveManager::removeParentalRelationship(m_d->designWidget->rootContext()->nameForObject(item));
//				m_d->designWidget->rootContext()->setContextProperty(
//							m_d->designWidget->rootContext()->nameForObject(item), 0);
//				item->deleteLater();
//			}
//			m_d->m_Items.clear();
//			m_d->m_ItemUrls.clear();
//			HideSelectionTools();
//			m_d->toolboxList->setCurrentRow(-1);
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
		auto dir = dname(m_d->toolboxList->GetUrls(m_d->toolboxList->currentItem())[0].toLocalFile());
		auto toolName = m_d->toolboxList->currentItem()->text();
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
		for (int i = 0; i < m_d->toolboxList->count(); i++) {
			if (m_d->toolboxList->item(i)->text() == name) {
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
    delete m_d;
}
