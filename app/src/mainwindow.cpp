#include <fit.h>
#include <zipper.h>
#include <string.h>
#include <titlebar.h>
#include <covermenu.h>
#include <listwidget.h>
#include <mainwindow.h>
#include <resizertick.h>
#include <rotatortick.h>
#include <removertick.h>
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
	, m_ResizerTick(new ResizerTick)
	, m_RotatorTick(new RotatorTick)
	, m_RemoverTick(new RemoverTick)
	, m_RootItem(nullptr)
	, m_CurrentPage(nullptr)
	, m_RightMenu(new CoverMenu)
	, m_LeftMenu(new CoverMenu)
{
	if (m_d) return;
	m_d = new MainWindowPrivate(this);
	m_d->setupUi(this);
	SetupGui();
	QTimer::singleShot(300, [=] { SetupManagers(); });
}

void MainWindow::SetupGui()
{
	/* Scaling things */
	fit(m_d->editButton, Fit::WidthHeight, true);
	fit(m_d->clearButton, Fit::WidthHeight, true);

	m_d->centralWidget->layout()->setContentsMargins(0,0,0,fit(8));
	m_d->buttonsLayout->setSpacing(fit(5));
	m_d->bindingWidget->setRootContext(m_d->designWidget->rootContext());
	m_d->bindingWidget->setItemSource(&m_d->m_Items);
	/* Set ticks' icons */
	m_ResizerTick->setIcon(QIcon(":/resources/images/resize-icon.png"));
	m_RemoverTick->setIcon(QIcon(":/resources/images/delete-icon.png"));
	m_RotatorTick->setIcon(QIcon(":/resources/images/rotate-icon.png"));

	/* Set ticks' root contexts */
	m_ResizerTick->SetRootContext(m_d->designWidget->rootContext());
	m_RotatorTick->SetRootContext(m_d->designWidget->rootContext());

	/* Start filtering design area */
	m_d->designWidget->installEventFilter(this);

	/* Hide ticks when tracked item removed */
	connect(m_RemoverTick, &RemoverTick::ItemRemoved, m_ResizerTick, &ResizerTick::hide);
	connect(m_RemoverTick, &RemoverTick::ItemRemoved, m_RotatorTick, &RotatorTick::hide);
	connect(m_RemoverTick, &RemoverTick::ItemRemoved, m_d->propertiesWidget, &PropertiesWidget::clearList);
	connect(m_RemoverTick, &RemoverTick::ItemRemoved, m_d->bindingWidget, &BindingWidget::clearList);
	connect(m_RemoverTick, &RemoverTick::ItemRemoved, m_d->bindingWidget, &BindingWidget::detachBindingsFor);

	/* Remove deleted items from internal item list */
    connect(m_RemoverTick, static_cast<void (RemoverTick::*)(QQuickItem* const item)const>(&RemoverTick::ItemRemoved), [=]
    (QQuickItem* item) {
		if (m_d->m_Items.contains(item)) {
			auto childs = GetAllChildren(item);
			for (auto child : childs) {
				qDebug() << child;
				if (m_d->m_Items.contains(child)) {
					SaveManager::removeSave(m_d->designWidget->rootContext()->nameForObject(child));
					SaveManager::removeParentalRelationship(m_d->designWidget->rootContext()->nameForObject(child));
					int i = m_d->m_Items.indexOf(child);
					m_d->m_Items.removeAt(i);
					m_d->m_ItemUrls.removeAt(i);
					m_d->designWidget->rootContext()->setContextProperty(m_d->designWidget->rootContext()->nameForObject(child), 0);
				}
			}
		}
	});

	/* Re-move ticks when tracked item resized */
	connect(m_ResizerTick, &ResizerTick::ItemResized, m_RemoverTick, &RemoverTick::FixCoord);
	connect(m_ResizerTick, &ResizerTick::ItemResized, m_RotatorTick, &RotatorTick::FixCoord);

	/* Re-move ticks when tracked item rotated */
	connect(m_RotatorTick, &RotatorTick::ItemRotated, m_RemoverTick, &RemoverTick::FixCoord);
	connect(m_RotatorTick, &RotatorTick::ItemRotated, m_ResizerTick, &ResizerTick::FixCoord);

	/* Hide ticks when editButton clicked */
	connect(m_d->editButton, &QPushButton::clicked, m_ResizerTick, &ResizerTick::hide);
	connect(m_d->editButton, &QPushButton::clicked, m_RemoverTick, &RemoverTick::hide);
	connect(m_d->editButton, &QPushButton::clicked, m_RotatorTick, &RotatorTick::hide);

	/* Enable/Disable other controls when editButton clicked */
	connect(m_d->editButton, &QPushButton::toggled, [this](bool checked) {m_d->propertiesWidget->setEnabled(checked);});
	connect(m_d->editButton, &QPushButton::toggled, [this](bool checked) {m_d->clearButton->setEnabled(!checked);});

	/* Set ticks' Parents and hide ticks */
	m_ResizerTick->setParent(m_d->designWidget);
	m_RemoverTick->setParent(m_d->designWidget);
	m_RotatorTick->setParent(m_d->designWidget);
	m_ResizerTick->hide();
	m_RemoverTick->hide();
	m_RotatorTick->hide();

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
	m_d->titleBar->setColor("#2b5796");
	m_d->titleBar->setShadowColor("#e0e4e7");
	connect(m_d->titleBar, SIGNAL(MenuToggled(bool)), m_RightMenu, SLOT(setCovered(bool)));
	connect(m_d->titleBar, SIGNAL(SettingsToggled(bool)), m_LeftMenu, SLOT(setCovered(bool)));
	connect(m_RightMenu, SIGNAL(toggled(bool)), m_d->titleBar, SLOT(setMenuChecked(bool)));
	connect(m_LeftMenu, SIGNAL(toggled(bool)), m_d->titleBar, SLOT(setSettingsChecked(bool)));

	/* Prepare Properties Widget */
	connect(this, SIGNAL(selectionShowed(QObject*const)), m_d->propertiesWidget, SLOT(refreshList(QObject*const)));
	connect(this, &MainWindow::selectionHided, [this] { m_d->propertiesWidget->setDisabled(true); });
	connect(this, SIGNAL(selectionShowed(QObject*const)), m_d->bindingWidget, SLOT(selectItem(QObject*const)));
	connect(this, SIGNAL(selectionHided()), m_d->bindingWidget, SLOT(clearList()));

	/* Set flat buttons' colors*/
	m_d->editButton->setColor(QColor("#2b5796"));
	m_d->editButton->setCheckedColor(QColor("#1e8145"));
	m_d->editButton->setTextColor(Qt::white);
	m_d->clearButton->setColor(QColor("#c03638"));
	m_d->clearButton->setDisabledColor(Qt::darkGray);
	m_d->clearButton->setDisabledTextColor(QColor("#444444"));
	m_d->clearButton->setTextColor(Qt::white);

	/* Fix Coords of ticks when property changed */
	connect(m_d->propertiesWidget, &PropertiesWidget::propertyChanged, m_RemoverTick, &RemoverTick::FixCoord);
	connect(m_d->propertiesWidget, &PropertiesWidget::propertyChanged, m_RotatorTick, &RotatorTick::FixCoord);
	connect(m_d->propertiesWidget, &PropertiesWidget::propertyChanged, m_ResizerTick, &ResizerTick::FixCoord);

	m_d->designWidget->rootContext()->setContextProperty("dpi", Fit::ratio());

	/* Init Left Container */
	QVariant toolboxVariant;
	toolboxVariant.setValue<QWidget*>(m_d->toolboxWidget);
	QVariant propertiesVariant;
	propertiesVariant.setValue<QWidget*>(m_d->propertiesWidget);
	QVariant bindingVariant;
	bindingVariant.setValue<QWidget*>(m_d->bindingWidget);
	QVariant pagesVariant;
	pagesVariant.setValue<QWidget*>(m_d->pagesWidget);
	Container* leftContainer = new Container;
	leftContainer->addWidget(m_d->toolboxWidget);
	leftContainer->addWidget(m_d->propertiesWidget);
	leftContainer->addWidget(m_d->bindingWidget);
	leftContainer->addWidget(m_d->pagesWidget);
	leftContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	QToolBar* leftToolbar = new QToolBar;
	leftToolbar->setStyleSheet(CSS::Toolbar);
	leftToolbar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
	leftToolbar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    leftToolbar->resize(leftToolbar->width(), 43);
	fit(leftToolbar, Fit::Height, true);
	QGraphicsDropShadowEffect* toolbarShadowEffect = new QGraphicsDropShadowEffect;
	toolbarShadowEffect->setBlurRadius(fit(7));
	toolbarShadowEffect->setOffset(0, fit(4));
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

	QRadioButton* bindingButton = new QRadioButton;
	bindingButton->setCursor(Qt::PointingHandCursor);
	bindingButton->setStyleSheet(CSS::BindingButton);
	bindingButton->setCheckable(true);
	QWidgetAction* bindingButtonAction = new QWidgetAction(this);
	bindingButtonAction->setDefaultWidget(bindingButton);
	bindingButtonAction->setData(bindingVariant);
	bindingButtonAction->setCheckable(true);
	leftToolbar->addAction(bindingButtonAction);
	connect(bindingButton, SIGNAL(clicked(bool)), bindingButtonAction, SLOT(trigger()));
	connect(bindingButtonAction, SIGNAL(triggered(bool)), leftContainer, SLOT(handleAction()));

	QRadioButton* pagesButton = new QRadioButton;
	pagesButton->setCursor(Qt::PointingHandCursor);
	pagesButton->setStyleSheet(CSS::PagesButton);
	pagesButton->setCheckable(true);
	QWidgetAction* pagesButtonAction = new QWidgetAction(this);
	pagesButtonAction->setDefaultWidget(pagesButton);
	pagesButtonAction->setData(pagesVariant);
	pagesButtonAction->setCheckable(true);
	leftToolbar->addAction(pagesButtonAction);
	connect(pagesButton, SIGNAL(clicked(bool)), pagesButtonAction, SLOT(trigger()));
	connect(pagesButtonAction, SIGNAL(triggered(bool)), leftContainer, SLOT(handleAction()));

	QWidget* leftMenuWidget = new QWidget;
	leftMenuWidget->setObjectName("leftMenuWidget");
	leftMenuWidget->setStyleSheet("#leftMenuWidget{background:#566573;}");
	connect(m_d->bindingWidget, &BindingWidget::popupShowed, [=] {
		leftMenuWidget->setStyleSheet("#leftMenuWidget{background:#2b5796;}");
	});
	connect(m_d->bindingWidget, &BindingWidget::popupHid, [=] {
		leftMenuWidget->setStyleSheet("#leftMenuWidget{background:#566573;}");
	});
	connect(toolboxButtonAction, (void(QWidgetAction::*)(bool))(&QWidgetAction::triggered), [=] {
		leftMenuWidget->setStyleSheet("#leftMenuWidget{background:#566573;}");
	});
	connect(propertiesButtonAction, (void(QWidgetAction::*)(bool))(&QWidgetAction::triggered), [=] {
		leftMenuWidget->setStyleSheet("#leftMenuWidget{background:#566573;}");
	});
	connect(pagesButtonAction, (void(QWidgetAction::*)(bool))(&QWidgetAction::triggered), [=] {
		leftMenuWidget->setStyleSheet("#leftMenuWidget{background:#566573;}");
	});
	connect(bindingButtonAction, (void(QWidgetAction::*)(bool))(&QWidgetAction::triggered), [=] {
		if (m_d->bindingWidget->hasPopupOpen())
			leftMenuWidget->setStyleSheet("#leftMenuWidget{background:#2b5796;}");
	});
	QVBoxLayout* leftMenuLayout = new QVBoxLayout(leftMenuWidget);
	leftMenuLayout->setContentsMargins(0, 0, 0, 0);
	leftMenuLayout->setSpacing(fit(8));
	leftMenuLayout->addWidget(leftToolbar);
	leftMenuLayout->addWidget(leftContainer);
	m_LeftMenu->attachWidget(leftMenuWidget);

	m_d->propertiesWidget->setRootContext(m_d->designWidget->rootContext());
	m_d->propertiesWidget->setItemSource(&m_d->m_Items);
	m_d->bindingWidget->setRootContext(m_d->designWidget->rootContext());

	m_d->centralWidget->installEventFilter(this);

	QTimer::singleShot(0, [=] {
#if !defined(Q_OS_DARWIN)
		m_d->designWidget->setSource(QUrl("qrc:/resources/qmls/dashboard.qml"));
#endif
		m_d->designWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
		/* Assign design area's root object */
		m_RootItem = m_d->designWidget->rootObject();
		connect(m_RootItem, SIGNAL(currentPageChanged(QVariant, QVariant)),
				this, SLOT(handleCurrentPageChanges(QVariant, QVariant)));
		auto v = QQmlProperty::read(m_RootItem, "current_page", m_d->designWidget->rootContext());
		m_CurrentPage = qobject_cast<QQuickItem*>(v.value<QObject*>());
		if (!m_CurrentPage) qFatal("MainWindow : Error occurred");
		auto v2 = QQmlProperty::read(m_RootItem, "swipeView", m_d->designWidget->rootContext());
		auto view = qobject_cast<QQuickItem*>(v2.value<QObject*>());
		connect(view, SIGNAL(currentIndexChanged()), this, SLOT(HideSelectionTools()));
		if (!view) qFatal("MainWindow : Error occurred");
		m_d->designWidget->rootContext()->setContextProperty("swipeView", view);
		auto v3 = qmlContext(view)->contextProperty("page1");
		auto item = qobject_cast<QQuickItem*>(v3.value<QObject*>());
		if (!item) qFatal("MainWindow : Error occurred");
		m_d->designWidget->rootContext()->setContextProperty("page1", item);
		m_d->pagesWidget->setSwipeItem(view);
		m_d->pagesWidget->setRootContext(m_d->designWidget->rootContext());
		m_d->pagesWidget->setItemList(&m_d->m_Items);
		m_d->pagesWidget->setUrlList(&m_d->m_ItemUrls);
		m_d->pagesWidget->setBindingWidget(m_d->bindingWidget);
	});

	m_d->bubbleHead = new BubbleHead(this);
	m_d->bubbleHead->setIcon(QIcon(":/resources/images/editor.png"));
	m_d->bubbleHead->setNotificationText("E");
	QTimer::singleShot(200,[this] {
		m_d->bubbleHead->move(fit(20), height()-fit(75));
	});

	m_d->qmlEditor = new QmlEditor(this);
	m_d->qmlEditor->setHidden(true);
	m_d->qmlEditor->setItems(&m_d->m_Items, &m_d->m_ItemUrls);
	m_d->qmlEditor->setRootContext(m_d->designWidget->rootContext());
	m_d->qmlEditor->setBindingWidget(m_d->bindingWidget);
	connect(this, SIGNAL(selectionShowed(QObject*const)), m_d->qmlEditor, SLOT(selectItem(QObject*const)));
    connect(m_d->bubbleHead, SIGNAL(clicked(bool)), this, SLOT(handleBubbleHeadClicked()));
	connect(m_d->bubbleHead, SIGNAL(moved(QPoint)), m_d->qmlEditor, SLOT(setShowCenter(QPoint)));

    QObject::connect(m_d->toolboxList,(void(ListWidget::*)(int))(&ListWidget::currentRowChanged),[=](int i){
        if (i>=0) {
            HideSelectionTools();
        }
    });

	m_d->aboutWidget = new About(this);

	QWidget* sceneListWidget = new QWidget(this);
	sceneListWidget->setStyleSheet("background:#566573;");
	m_RightMenu->attachWidget(sceneListWidget);
	QVBoxLayout* sceneListWidgetLayout = new QVBoxLayout(sceneListWidget);
	sceneListWidgetLayout->setSpacing(fit(15));
	sceneListWidgetLayout->setContentsMargins(0, 0, 0, 0);
	QLabel* sceneListTitle = new QLabel(this);
	sceneListTitle->setText("◉ Menu");
	sceneListTitle->setAlignment(Qt::AlignCenter);
	QGraphicsDropShadowEffect* sceneListTitleShadowEffect = new QGraphicsDropShadowEffect;
	sceneListTitleShadowEffect->setBlurRadius(fit(5));
	sceneListTitleShadowEffect->setOffset(0, fit(4));
	sceneListTitleShadowEffect->setColor(QColor(0, 0, 0, 60));
	sceneListTitle->setGraphicsEffect(sceneListTitleShadowEffect);
	sceneListTitle->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	sceneListTitle->setFixedHeight(fit(43));
	sceneListTitle->setStyleSheet("color: white; background:#C03638;");
	sceneListWidgetLayout->addWidget(sceneListTitle);
	sceneListWidgetLayout->addWidget(m_d->sceneList);

	/* Pop-up toolbox widget's scrollbar */
	connect(m_RightMenu, &CoverMenu::toggled, [this](bool checked) {if (checked) m_d->sceneList->showBar(); });
	connect(m_LeftMenu, &CoverMenu::toggled, [this](bool checked) {if (checked) m_d->toolboxList->showBar(); });
	connect(m_LeftMenu, &CoverMenu::toggled, [this](bool checked) {if (checked) m_d->propertiesWidget->showBar();});
	connect(m_LeftMenu, &CoverMenu::toggled, [this](bool checked) {if (checked) m_d->bindingWidget->showBar(); });
	connect(m_LeftMenu, &CoverMenu::toggled, [this](bool checked) {if (checked) m_d->pagesWidget->showBar(); });

	// Init Splash Screen
	SplashScreen::init(this);
	SplashScreen::setText("Wait");
	QTimer::singleShot(300, [=] { SplashScreen::setText("Decrypting the user's files"); });
	SplashScreen::setTextColor("#2e3a41");
	SplashScreen::setBackgroundBrush(QColor("#e0e4e7"));
	SplashScreen::setIcon(QIcon(":/resources/images/logo.png"));
	SplashScreen::setIconSize(Fit::fit(160), Fit::fit(80));
	SplashScreen::setLoadingSize(Fit::fit(24), Fit::fit(24));
	SplashScreen::setLoadingImageFilename("qrc:///resources/images/loading.png");
	SplashScreen::show();
}

void MainWindow::SetupManagers()
{
	//Let's add some custom controls to that project
	ToolsManager::setListWidget(m_d->toolboxList);
	auto userManager = new UserManager(this); //create new user manager
	auto projectManager = new ProjectManager(this); //create new project manager
	projectManager->setMainWindow(this);
	new SaveManager(this);
	auto sceneManager = new SceneManager;
	sceneManager->setMainWindow(this);
	sceneManager->setSceneListWidget(m_d->sceneList);
	sceneManager->addScene("studioScene", m_d->centralWidget);
	sceneManager->addScene("projectsScene", m_d->projectsScreen);
	sceneManager->addScene("aboutScene", m_d->aboutWidget);
	sceneManager->setCurrent("projectsScene");
	SplashScreen::raise();
	connect(sceneManager, (void(SceneManager::*)(const QString&))(&SceneManager::currentSceneChanged),
			[=](const QString& key){
		m_RightMenu->hide();
		m_d->titleBar->setMenuChecked(false);
		m_LeftMenu->hide();
		m_d->titleBar->setSettingsChecked(false);

		if (key == "studioScene") {
			m_d->bubbleHead->raise();
		}
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
		}
		urls << scene;
		QListWidgetItem* item = new QListWidgetItem(QIcon(iconPath), sceneName);
		m_d->sceneList->insertItem(0, item);
		m_d->sceneList->AddUrls(item, urls);
	}

	userManager->buildNewUser("kozmon@hotmail.com"); //build new user if doesn't exist already
	auto ret = QtConcurrent::run((bool (*)(const QString&,const QString&))(&UserManager::startUserSession),
					  QString("kozmon@hotmail.com"), QString("password123")); //unlock user session
	while(ret.isRunning()) qApp->processEvents(QEventLoop::AllEvents, 50);
	connect(qApp, SIGNAL(aboutToQuit()), userManager, SLOT(stopUserSession()));
	m_d->projectsScreen->refreshProjectList();
	SplashScreen::hide();
}

bool MainWindow::eventFilter(QObject* object, QEvent* event)
{
	/* Handle events which are happening on design area */
	if (object == m_d->designWidget)
	{
		/* Design area's events' shared variables */
		static QPoint dragStartPoint;
		static QQuickItem* pressedItem;

		switch (event->type())
		{
			case QEvent::DragEnter:
			{
				event->accept();
				return true;
			}
			case QEvent::DragLeave:
			{
				event->accept();
				return true;
			}
			case QEvent::DragMove:
			{
				event->accept();
				return true;
			}

			case QEvent::Drop:
			{
				QDropEvent* dropEvent = static_cast<QDropEvent*>(event);
				event->accept();

				if (nullptr != pressedItem)
				{
					/* Handled drops coming from design area itself */
					if (true == dropEvent->mimeData()->hasFormat("dashboard/x-qquickitem"))
					{
						/* Get deepest item under the dropped point */
						QQuickItem* itemAtDroppedPoint = GetDeepestDesignItemOnPoint(dropEvent->pos());

						/* Get all children */
						QQuickItemList childItems = GetAllChildren(pressedItem);

						/* Get fixed dropped point as in desing area coord system */
						QPointF droppedPoint = dropEvent->pos() - dragStartPoint +
											   m_CurrentPage->mapFromItem(pressedItem->parentItem(), pressedItem->position());

						/* Move related item to its new position */
						if (nullptr == itemAtDroppedPoint)
						{
							pressedItem->setParentItem(m_CurrentPage);
							SaveManager::addParentalRelationship(m_d->designWidget->rootContext()
																 ->nameForObject(pressedItem),
																 m_d->designWidget->rootContext()
																 ->nameForObject(m_CurrentPage));
							pressedItem->setPosition(droppedPoint);
						}
						else if (true == childItems.contains(itemAtDroppedPoint))
						{
							pressedItem->setPosition(pressedItem->parentItem()->mapFromItem(m_CurrentPage, droppedPoint));
						}
						else if (pressedItem != itemAtDroppedPoint) // else handled in previous else if
						{
							QPointF mappedPoint = itemAtDroppedPoint->mapFromItem(m_CurrentPage, droppedPoint);
							pressedItem->setParentItem(itemAtDroppedPoint);
							SaveManager::addParentalRelationship(m_d->designWidget->rootContext()
																 ->nameForObject(pressedItem),
																 m_d->designWidget->rootContext()
																 ->nameForObject(itemAtDroppedPoint));
							pressedItem->setPosition(mappedPoint);
						}
						SaveManager::setVariantProperty(m_d->designWidget->rootContext()
														->nameForObject(pressedItem),
														"x",pressedItem->x());
						SaveManager::setVariantProperty(m_d->designWidget->rootContext()
														->nameForObject(pressedItem),
														"y",pressedItem->y());
						fixWebViewPosition(pressedItem);
						ShowSelectionTools(pressedItem);
						return true;
					}
				}

				/* Handled drops coming from toolbox */
				if (dropEvent->mimeData()->hasUrls()) // WARNING: All kind of urls enter!
				{
					auto url = dropEvent->mimeData()->urls().at(0);
					m_d->designWidget->engine()->clearComponentCache(); //WARNING: Performance issues?
					QQmlComponent component(m_d->designWidget->engine()); //TODO: Drop into another item?
					component.loadUrl(url);

					QQmlIncubator incubator;
					component.create(incubator, m_d->designWidget->rootContext());
					while (incubator.isLoading()) {
						QApplication::processEvents(QEventLoop::AllEvents, 50);
					}
					QQuickItem *qml = qobject_cast<QQuickItem*>(incubator.object());

					if (component.isError() || !qml) {
						QMessageBox msgBox;
						msgBox.setText("<b>This tool has some errors, please fix these first.</b>");
						msgBox.setStandardButtons(QMessageBox::Ok);
						msgBox.setDefaultButton(QMessageBox::Ok);
						msgBox.setIcon(QMessageBox::Information);
						msgBox.exec();
						return true;
					}

					int count = 1;
					QString componentName = qmlContext(qml)->nameForObject(qml);
					if (componentName.isEmpty()) componentName = "anonymous";
					for (int i=0; i<m_d->m_Items.size();i++) {
						if (componentName == QString(m_d->designWidget->rootContext()->nameForObject(m_d->m_Items[i])) ||
							componentName == QString("dpi") || componentName == QString("swipeView")) {
							// FIXME: If it's conflict with page names?
                            if (componentName.at(componentName.size() - 1).isNumber()) {
                                componentName.remove(componentName.size() - 1, 1);
                            }
							componentName += QString::number(count);
							count++;
							i = -1;
						}
					}
					m_d->designWidget->rootContext()->setContextProperty(componentName, qml);
					qml->setParentItem(m_CurrentPage);
					qml->setPosition(qml->mapFromItem(m_CurrentPage, dropEvent->pos()));
					qml->setClip(true); // Even if it's not true
					qml->setEnabled(!m_d->editButton->isChecked());
					fit(qml, Fit::WidthHeight);
					m_d->m_Items << qml;
					m_d->m_ItemUrls << url;
					SaveManager::addSave(componentName, url.toLocalFile());
					SaveManager::addParentalRelationship(componentName, m_d->designWidget->rootContext()->nameForObject(m_CurrentPage));
					SaveManager::setVariantProperty(componentName, "x", qml->x());
					SaveManager::setVariantProperty(componentName, "y", qml->y());
					SaveManager::setVariantProperty(componentName, "clip", qml->clip());
					SaveManager::setVariantProperty(componentName, "enabled", qml->isEnabled());

					QTimer::singleShot(200, [qml, this] { fixWebViewPosition(qml); });
					if (m_d->editButton->isChecked()) ShowSelectionTools(qml);
					return true;
				}
				return false;
			}

			case QMouseEvent::MouseButtonPress:
			{
				QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);

				/* Edit mode things */
				if (m_d->editButton->isChecked())
				{
					/* Get deepest item under the pressed point */
					pressedItem = GetDeepestDesignItemOnPoint(mouseEvent->pos());

					/* Show selection tools */
					if (nullptr != pressedItem)
					{
						if (m_ResizerTick->TrackedItem() != pressedItem)
							ShowSelectionTools(pressedItem);
						else if (m_ResizerTick->isHidden())
							ShowSelectionTools(pressedItem);
						else
							HideSelectionTools();
					}
					else
						HideSelectionTools();

					/* Capture drag start point for drags made within design area */
					dragStartPoint = mouseEvent->pos();
				}

				return false;
			}

			case QMouseEvent::MouseMove:
			{
				QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);

				/* Edit mode things */
				if (m_d->editButton->isChecked())
				{
					/* Made Drags from design area */
					if (nullptr != pressedItem)
					{
						if (!(mouseEvent->buttons() & Qt::LeftButton))
							return false;
						if ((mouseEvent->pos() - dragStartPoint).manhattanLength()
							< QApplication::startDragDistance())
							return false;

						QVariant variant;
						variant.setValue<QQuickItem*>(pressedItem);

						QString mimeType = "dashboard/x-qquickitem";
						QMimeData *mimeData = new QMimeData;
						mimeData->setData(mimeType, variant.toByteArray());

						QDrag *drag = new QDrag(this);
						drag->setMimeData(mimeData);
						QPointF diffPoint = dragStartPoint - m_CurrentPage->mapFromItem(pressedItem->parentItem(), pressedItem->position());
						drag->setHotSpot(diffPoint.toPoint());

						QSharedPointer<QQuickItemGrabResult> result = pressedItem->grabToImage();
						connect(result.data(), &QQuickItemGrabResult::ready, this, [=] {
							drag->setPixmap(QPixmap::fromImage(result->image()));
							drag->exec();
						});
					}
				}

				return false;
			}

			default:
				return false;
		}
	} else if (object == m_d->centralWidget) {
		if (event->type() == QEvent::Move) {
			fixWebViewPositions();
			return false;
		}
	} else {
		return QWidget::eventFilter(object,event);
	}
	return false;
}

void MainWindow::resizeEvent(QResizeEvent* event)
{
	m_d->qmlEditor->setGeometry(0, 0, width(), height());
	QWidget::resizeEvent(event);
	emit resized();
}

void MainWindow::fixWebViewPosition(QQuickItem* const item)
{
#if defined(Q_OS_ANDROID) || defined(Q_OS_IOS) || defined(Q_OS_WINPHONE)
	// FIXME: If object resized or rotated
	// WARNING: All controls could only have one single webview
	QQuickItem* view = nullptr;
	for (auto ccitem : item->findChildren<QObject*>()) {
		if (QString(ccitem->metaObject()->className()) == "QQuickWebView")
			view = qobject_cast<QQuickItem*>(ccitem);
	}
	if (!view) return;

	QWindow* main_window = nullptr;
	for (auto window : QApplication::allWindows()) {
		if (QString(window->metaObject()->className()) == "QWidgetWindow" &&
			window->objectName() == "MainWindowWindow") {
			main_window = window;
		}
	}
	if (!main_window) return;

	int count = 0;
	for (auto citem : m_d->m_Items) {
		if (citem == item) break;
		for (auto ccitem : citem->findChildren<QObject*>()) {
			if (QString(ccitem->metaObject()->className()) == "QQuickWebView")
				count++;
		}
	}

#if defined(Q_OS_IOS)
	IOS::fixCoordOfWebView(main_window, view, count);
#else
	int count_2 = 0;
	for (auto child : main_window->children()) { //WARNING: We assume all child windows of QWidgetWindow are WebViews
		auto window = qobject_cast<QWindow*>(child);
		if (!window) continue;
		if (count_2 == count) {
			window->setPosition(view->parentItem()->mapToGlobal(view->position()).toPoint());
			QApplication::processEvents(QEventLoop::AllEvents);
			break;
		}
		count_2++;
	}
#endif

#else
	Q_UNUSED(item)
#endif
}

void MainWindow::fixWebViewPositions()
{
	for (auto item : m_d->m_Items)
		fixWebViewPosition(item);
}

QQuickItem* MainWindow::GetDeepestDesignItemOnPoint(const QPoint& point) const
{
	QPointF pt = point;
    QQuickItem* parent = m_CurrentPage,*final = m_CurrentPage;
	QQuickItem* item = parent->childAt(pt.x(), pt.y());
	if (m_d->m_Items.contains(item)) final = item;

    while (item) {
		pt = item->mapFromItem(parent, pt);
		parent = item;
		item = parent->childAt(pt.x(), pt.y());
		if (m_d->m_Items.contains(item)) final = item;
    }

    if (final == m_CurrentPage) return nullptr;
    return final;
}

const MainWindow::QQuickItemList MainWindow::GetAllChildren(QQuickItem* const item)
{
	/* Return all child items of item including item itself */
	QQuickItemList childList;
	for (auto child : item->childItems())
		childList << GetAllChildren(child);
	childList << item;
	return childList;
}

void MainWindow::ShowSelectionTools(QQuickItem* const selectedItem)
{
	m_d->qmlEditor->setDeactive(false);
	m_ResizerTick->SetTrackedItem(selectedItem);
	m_ResizerTick->show();
	m_RotatorTick->SetTrackedItem(selectedItem);
	m_RotatorTick->show();
	m_RemoverTick->SetTrackedItem(selectedItem);
    m_RemoverTick->show();
    m_d->toolboxList->setCurrentRow(-1);
	emit selectionShowed(selectedItem);
}

void MainWindow::handleCurrentPageChanges(const QVariant& CurrentPage, const QVariant& index)
{
	m_CurrentPage = qobject_cast<QQuickItem*>(CurrentPage.value<QObject*>());
	if (!m_CurrentPage) qFatal("MainWindow : Error occurred");
    m_d->pagesWidget->setCurrentPage(index.toInt());
}

void MainWindow::handleBubbleHeadClicked()
{
    if(m_ResizerTick->isVisible()) {
        m_d->qmlEditor->show();
    } else {
        if (m_d->toolboxList->currentRow() < 0) return;
        auto cItem = m_d->toolboxList->currentItem();
		m_d->qmlEditor->setRootFolder(ToolsManager::toolsDir() + separator() + m_d->toolboxList->currentItem()->text());
        m_d->qmlEditor->show(m_d->toolboxList->GetUrls(cItem)[0].toLocalFile());
	}
	m_d->qmlEditor->raise();
}

void MainWindow::clearStudio()
{
	m_d->toolboxList->ClearUrls();
	m_d->toolboxList->clear();
	m_d->qmlEditor->clearCache();

	for (auto item : m_d->m_Items) {
		m_d->designWidget->rootContext()->setContextProperty(
					m_d->designWidget->rootContext()->nameForObject(item), 0);
		item->deleteLater();
	}

	auto pages = m_d->pagesWidget->pages();
	for (int i = pages.count(); i--;) {
		if (pages[i] != m_CurrentPage) {
			m_d->pagesWidget->removePageWithoutSave(m_d->designWidget->rootContext()->nameForObject(pages[i]));
		}
	}
	QString page1Name = "page1";
	m_d->pagesWidget->changePageWithoutSave(m_d->designWidget->rootContext()->nameForObject(m_CurrentPage), page1Name);

	m_d->bindingWidget->clearAllBindings();
	m_d->m_Items.clear();
	m_d->m_ItemUrls.clear();
	HideSelectionTools();
	m_d->bubbleHead->move(fit(20), height()-fit(75));
	m_d->toolboxList->setCurrentRow(-1);
}

void MainWindow::HideSelectionTools()
{
	m_ResizerTick->hide();
	m_RemoverTick->hide();
	m_RotatorTick->hide();
	m_d->qmlEditor->setDeactive(true);
	emit selectionHided();
}

void MainWindow::on_clearButton_clicked()
{
	QMessageBox msgBox;
	msgBox.setText("<b>This will clear the current page's content.</b>");
	msgBox.setInformativeText("Do you want to continue?");
	msgBox.setStandardButtons(QMessageBox::No | QMessageBox::Yes);
	msgBox.setDefaultButton(QMessageBox::No);
	msgBox.setIcon(QMessageBox::Warning);
	const int ret = msgBox.exec();
	switch (ret) {
		case QMessageBox::Yes: {
			auto items = GetAllChildren(m_CurrentPage);
			for (auto item : items) {
				if (m_d->m_Items.contains(item)) {
					SaveManager::removeSave(m_d->designWidget->rootContext()->nameForObject(item));
					SaveManager::removeParentalRelationship(m_d->designWidget->rootContext()->nameForObject(item));
					m_d->designWidget->rootContext()->setContextProperty(
								m_d->designWidget->rootContext()->nameForObject(item), 0);
					int i = m_d->m_Items.indexOf(item);
					m_d->m_Items.removeOne(item);
					m_d->m_ItemUrls.removeAt(i);
					m_d->bindingWidget->detachBindingsFor(item);
					item->deleteLater();
				}
			}
			break;
		} default: {
			// Do nothing
			break;
		}
	}
}

void MainWindow::on_editButton_clicked()
{
	for (auto item : m_d->m_Items) {
		item->setEnabled(!m_d->editButton->isChecked());
	}
	m_d->propertiesWidget->clearList();
	m_d->bindingWidget->clearList();
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
	if (name == m_d->toolboxList->currentItem()->text() || name == "") return;

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

	auto from = ToolsManager::toolsDir() + "/" + m_d->toolboxList->currentItem()->text();
	auto to = ToolsManager::toolsDir() + "/" + name;
	if (!rn(from, to)) qFatal("MainWindow : Error occurred");

	m_d->toolboxList->currentItem()->setText(name);

	QList<QUrl> urls;
	urls << QUrl::fromLocalFile(to + "/main.qml");
	m_d->toolboxList->RemoveUrls(m_d->toolboxList->currentItem());
	m_d->toolboxList->AddUrls(m_d->toolboxList->currentItem(),urls);

	for (int i = 0; i < m_d->m_ItemUrls.size(); i++) {
		if (m_d->m_ItemUrls[i].toLocalFile() == (from+"/main.qml")) {
			m_d->m_ItemUrls[i] = QUrl::fromLocalFile(to+"/main.qml");
		}
	}

	m_d->qmlEditor->updateCacheForRenamedEntry(from, to, true);
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
	if (m_d->toolboxList->currentRow() < 0) return;
	auto name = m_d->toolboxList->currentItem()->text();
	QMessageBox msgBox;
	msgBox.setText(QString("<b>This will remove %1 from Tool Library and Dashboard.</b>").arg(name));
	msgBox.setInformativeText("Do you want to continue?");
	msgBox.setStandardButtons(QMessageBox::No | QMessageBox::Yes);
	msgBox.setDefaultButton(QMessageBox::No);
	msgBox.setIcon(QMessageBox::Warning);
	const int ret = msgBox.exec();
	switch (ret) {
		case QMessageBox::Yes: {
			m_d->qmlEditor->clearCacheFor(ToolsManager::toolsDir() + separator() + name, true);
			rm(ToolsManager::toolsDir() + separator() + name);
			m_d->toolboxList->RemoveUrls(m_d->toolboxList->currentItem());
			delete m_d->toolboxList->takeItem(m_d->toolboxList->currentRow());

			for (int i = 0; i < m_d->m_ItemUrls.size(); i++) {
				if (m_d->m_ItemUrls[i].toLocalFile() == (ToolsManager::toolsDir() + separator() + name + "/main.qml")) {
					auto items = GetAllChildren(m_d->m_Items[i]);
					for (auto item : items) {
						if (m_d->m_Items.contains(item)) {
							SaveManager::removeSave(m_d->designWidget->rootContext()->nameForObject(item));
							SaveManager::removeParentalRelationship(m_d->designWidget->rootContext()->nameForObject(item));
							m_d->designWidget->rootContext()->setContextProperty(
										m_d->designWidget->rootContext()->nameForObject(item), 0);
							int j = m_d->m_Items.indexOf(item);
							m_d->m_Items.removeOne(item);
							m_d->m_ItemUrls.removeAt(j);
							m_d->bindingWidget->detachBindingsFor(item);
							item->deleteLater();
						}
					}
				}
			}
			HideSelectionTools();
			break;
		} default: {
			// Do nothing
			break;
		}
	}
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
	QMessageBox msgBox;
	msgBox.setText("<b>This will clear Dashboard and reset Tool Library.</b>");
	msgBox.setInformativeText("Do you want to continue?");
	msgBox.setStandardButtons(QMessageBox::No | QMessageBox::Yes);
	msgBox.setDefaultButton(QMessageBox::No);
	msgBox.setIcon(QMessageBox::Warning);
	const int ret = msgBox.exec();
	switch (ret) {
		case QMessageBox::Yes: {
			m_d->toolboxList->ClearUrls();
			m_d->toolboxList->clear();
			m_d->qmlEditor->clearCache();
			ToolsManager::resetTools();

			for (auto item : m_d->m_Items) {
				SaveManager::removeSave(m_d->designWidget->rootContext()->nameForObject(item));
				SaveManager::removeParentalRelationship(m_d->designWidget->rootContext()->nameForObject(item));
				m_d->designWidget->rootContext()->setContextProperty(
							m_d->designWidget->rootContext()->nameForObject(item), 0);
				item->deleteLater();
			}
			m_d->bindingWidget->clearAllBindings();
			m_d->m_Items.clear();
			m_d->m_ItemUrls.clear();
			HideSelectionTools();
			m_d->toolboxList->setCurrentRow(-1);
			break;
		} default: {
			// Do nothing
			break;
		}
	}
}

void MainWindow::toolboxImportButtonClicked()
{
	QFileDialog dialog(this);
	dialog.setFileMode(QFileDialog::ExistingFiles);
	dialog.setNameFilter(tr("Zip files (*.zip)"));
	dialog.setViewMode(QFileDialog::Detail);
	if (dialog.exec()) {
		handleImports(dialog.selectedFiles());
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

bool MainWindow::addControlWithoutSave(const QUrl& url, const QString& parent)
{
	m_d->designWidget->engine()->clearComponentCache(); //WARNING: Performance issues?
	QQmlComponent component(m_d->designWidget->engine()); //TODO: Drop into another item?
	component.loadUrl(url);

	QQmlIncubator incubator;
	component.create(incubator, m_d->designWidget->rootContext());
	while (incubator.isLoading()) {
		QApplication::processEvents(QEventLoop::AllEvents, 50);
	}
	QQuickItem *qml = qobject_cast<QQuickItem*>(incubator.object());
	if (component.isError() || !qml) return false;

	int count = 1;
	QString componentName = qmlContext(qml)->nameForObject(qml);
	if (componentName.isEmpty()) componentName = "anonymous";
	for (int i=0; i<m_d->m_Items.size();i++) {
		if (componentName == QString(m_d->designWidget->rootContext()->nameForObject(m_d->m_Items[i])) ||
			componentName == QString("dpi") || componentName == QString("swipeView")) {
			// FIXME: If it's conflict with page names?
			if (componentName.at(componentName.size() - 1).isNumber()) {
				componentName.remove(componentName.size() - 1, 1);
			}
			componentName += QString::number(count);
			count++;
			i = -1;
		}
	}

	auto parentProperty = m_d->designWidget->rootContext()->contextProperty(parent);
	auto parentItem = qobject_cast<QQuickItem*>(parentProperty.value<QObject*>());
	if (!parentItem) qFatal("MainWindow::addControlWithoutSave : Error occurred");
	m_d->designWidget->rootContext()->setContextProperty(componentName, qml);
	qml->setParentItem(parentItem);
//	qml->setPosition({10,10});
	qml->setClip(true); // Even if it's not true
	qml->setEnabled(!m_d->editButton->isChecked());
	fit(qml, Fit::WidthHeight);
	m_d->m_Items << qml;
	m_d->m_ItemUrls << url;

	QTimer::singleShot(200, [qml] { m_d->parent->fixWebViewPosition(qml); });
	if (m_d->editButton->isChecked()) m_d->parent->ShowSelectionTools(qml);
	return true;
}

MainWindow::~MainWindow()
{
	delete m_d;
}

// TODO: Pop-up loading screen
// TODO: Make a object clone tick
// TODO: Make it possible to resize(zoom) and rotate operations with fingers
// TODO: Layouts?
// TODO: Code a version numberer for tools' objectNames
// FIXME: Make toolsExists function works
// FIXME: changed qml file doesn't make changes on designer due to source url is same
