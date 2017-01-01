#include <fit.h>
#include <miniz.h>
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

#include <QtQml>
#include <QtCore>
#include <QtQuick>
#include <QtWidgets>
#include <QtNetwork>

#if defined(Q_OS_IOS)
#include <ios.h>
#endif

using namespace Fit;

MainWindow::MainWindow(QWidget *parent)
	: QWidget(parent)
	, m_d(new MainWindowPrivate)
	, m_ResizerTick(new ResizerTick)
	, m_RotatorTick(new RotatorTick)
	, m_RemoverTick(new RemoverTick)
	, m_RootItem(nullptr)
	, m_LeftMenu(new CoverMenu)
	, m_RightMenu(new CoverMenu)
{
	m_d->setupUi(this);
	SetToolsDir();
	SetupGui();
	DownloadTools(QUrl::fromUserInput("qrc:/resources/tools/tools.json"));
}

void MainWindow::SetupGui()
{
	/* Scaling things */
	fit(m_d->editButton, Fit::WidthHeight, true);
	fit(m_d->clearButton, Fit::WidthHeight, true);

	m_d->centralWidget->layout()->setContentsMargins(0,0,0,fit(8));
	m_d->buttonsLayout->setSpacing(fit(5));
	m_d->bindingWidget->setRootContext(m_d->designWidget->rootContext());
	m_d->bindingWidget->setItemSource(&m_Items);
	/* Set ticks' icons */
	m_ResizerTick->setIcon(QIcon(":/resources/images/resize-icon.png"));
	m_RemoverTick->setIcon(QIcon(":/resources/images/delete-icon.png"));
	m_RotatorTick->setIcon(QIcon(":/resources/images/rotate-icon.png"));

	/* Toolbox stylizing */
	m_d->toolboxWidget->setIconSize(fit({30, 30}));

	/* Start filtering design area */
	m_d->designWidget->installEventFilter(this);

	/* Hide ticks when tracked item removed */
	connect(m_RemoverTick, &RemoverTick::ItemRemoved, m_ResizerTick, &ResizerTick::hide);
	connect(m_RemoverTick, &RemoverTick::ItemRemoved, m_RotatorTick, &RotatorTick::hide);
	connect(m_RemoverTick, &RemoverTick::ItemRemoved, m_d->propertiesWidget, &PropertiesWidget::clearList);
	connect(m_RemoverTick, &RemoverTick::ItemRemoved, m_d->bindingWidget, &BindingWidget::clearList);
	connect(m_RemoverTick, &RemoverTick::ItemRemoved, m_d->bindingWidget, &BindingWidget::detachBindingsFor);

	/* Remove deleted items from internal item list */
	connect(m_RemoverTick, static_cast<void (RemoverTick::*)(QQuickItem* const item)const>(&RemoverTick::ItemRemoved), [=] (QQuickItem* item) {
		m_Items.removeAll(item);
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
	connect(m_d->editButton, &QPushButton::toggled, [this](bool checked) {m_d->toolboxWidget->setEnabled(!checked);});
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
	m_LeftMenu->setCoverWidget(m_d->centralWidget);
	m_LeftMenu->setCoverSide(CoverMenu::FromLeft);
	connect(this,SIGNAL(resized()),m_LeftMenu,SLOT(hide()));
	connect(this,&MainWindow::resized, [this] { m_d->titleBar->setMenuChecked(false); });

	/* Add Properties Menu */
	m_RightMenu->setCoverWidget(m_d->centralWidget);
	m_RightMenu->setCoverSide(CoverMenu::FromRight);
	connect(this,SIGNAL(resized()),m_RightMenu,SLOT(hide()));
	connect(this,&MainWindow::resized, [this] { m_d->titleBar->setSettingsChecked(false); });

	/* Add Title Bar */
	fit(m_d->titleBar, Fit::Height, true);
	m_d->titleBar->setText("Studio");
	m_d->titleBar->setColor("#2b5796");
	m_d->titleBar->setShadowColor("#e0e4e7");
	connect(m_d->titleBar, SIGNAL(MenuToggled(bool)), m_LeftMenu, SLOT(setCovered(bool)));
	connect(m_d->titleBar, SIGNAL(SettingsToggled(bool)), m_RightMenu, SLOT(setCovered(bool)));
	connect(m_LeftMenu, SIGNAL(toggled(bool)), m_d->titleBar, SLOT(setMenuChecked(bool)));
	connect(m_RightMenu, SIGNAL(toggled(bool)), m_d->titleBar, SLOT(setSettingsChecked(bool)));

	/* Prepare Properties Widget */
	connect(this, SIGNAL(selectionShowed(QObject*const)), m_d->propertiesWidget, SLOT(refreshList(QObject*const)));
	connect(this, &MainWindow::selectionHided, [this] { m_d->propertiesWidget->setDisabled(true); });
	connect(this, SIGNAL(selectionShowed(QObject*const)), m_d->bindingWidget, SLOT(selectItem(QObject*const)));
	connect(this, SIGNAL(selectionHided()), m_d->bindingWidget, SLOT(clearList()));

	/* Pop-up toolbox widget's scrollbar */
	connect(m_LeftMenu, &CoverMenu::toggled, [this](bool checked) {if (checked) m_d->toolboxWidget->showBar(); });
	connect(m_RightMenu, &CoverMenu::toggled, [this](bool checked) {if (checked) m_d->propertiesWidget->showBar(); });
	connect(m_RightMenu, &CoverMenu::toggled, [this](bool checked) {if (checked) m_d->bindingWidget->showBar(); });

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
	Container* leftContainer = new Container;
	leftContainer->addWidget(m_d->toolboxWidget);
	leftContainer->addWidget(m_d->propertiesWidget);
	leftContainer->addWidget(m_d->bindingWidget);
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
	QWidgetAction* toolboxButtonAction = new QWidgetAction(this);
	toolboxButtonAction->setDefaultWidget(toolboxButton);
	toolboxButtonAction->setData(toolboxVariant);
	toolboxButtonAction->setCheckable(true);
	leftToolbar->addAction(toolboxButtonAction);
	connect(toolboxButton, SIGNAL(clicked(bool)), toolboxButtonAction, SLOT(trigger()));
	connect(toolboxButtonAction, SIGNAL(triggered(bool)), leftContainer, SLOT(handleAction()));

	QRadioButton* propertiesButton = new QRadioButton;
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
	bindingButton->setStyleSheet(CSS::BindingButton);
	bindingButton->setCheckable(true);
	QWidgetAction* bindingButtonAction = new QWidgetAction(this);
	bindingButtonAction->setDefaultWidget(bindingButton);
	bindingButtonAction->setData(bindingVariant);
	bindingButtonAction->setCheckable(true);
	leftToolbar->addAction(bindingButtonAction);
	connect(bindingButton, SIGNAL(clicked(bool)), bindingButtonAction, SLOT(trigger()));
	connect(bindingButtonAction, SIGNAL(triggered(bool)), leftContainer, SLOT(handleAction()));

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
	m_d->propertiesWidget->setItemSource(&m_Items);
	m_d->bindingWidget->setRootContext(m_d->designWidget->rootContext());

	m_d->centralWidget->installEventFilter(this);

	QTimer::singleShot(0, [=] {
	#if !defined(Q_OS_DARWIN)
		m_d->designWidget->setSource(QUrl("qrc:/resources/qmls/design-area.qml"));
	#endif
		m_d->designWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
		/* Assign design area's root object */
		m_RootItem = m_d->designWidget->rootObject();
	});

	m_d->bubbleHead = new BubbleHead(this);
	m_d->bubbleHead->setIcon(QIcon(":/resources/images/editor.png"));
	m_d->bubbleHead->setNotificationCount(2);
	m_d->bubbleHead->setNotificationCount(2);

	m_d->aboutWidget = new About(m_d->centralWidget, this);
	m_d->aboutButton = new FlatButton(this);
	m_d->aboutButton->setIcon(QIcon(":/resources/images/info.png"));
	m_d->aboutButton->setIconButton(true);
	m_d->aboutButton->setCheckable(true);
	connect(m_d->aboutButton, (void(FlatButton::*)(bool))(&FlatButton::clicked), [=]{
		m_LeftMenu->hide();
		m_d->titleBar->setMenuChecked(false);
		m_RightMenu->hide();
		m_d->titleBar->setSettingsChecked(false);
	});
	connect(this,&MainWindow::resized, [=]{
		m_d->aboutWidget->hide();
		m_d->aboutButton->setChecked(false);
	});
	connect(m_d->aboutButton, SIGNAL(clicked(bool)), m_d->aboutWidget, SLOT(show(bool)));
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

				/* Edit mode things */
				if (m_d->editButton->isChecked())
				{

					if (nullptr != pressedItem)
					{
						/* Handled drops coming from design area itself */
						if (true == dropEvent->mimeData()->hasFormat("designarea/x-qquickitem"))
						{
							/* Get deepest item under the dropped point */
							QQuickItem* itemAtDroppedPoint = GetDeepestDesignItemOnPoint(dropEvent->pos());

							/* Get all children */
							QQuickItemList childItems = GetAllChildren(pressedItem);

							/* Get fixed dropped point as in desing area coord system */
							QPointF droppedPoint = dropEvent->pos() - dragStartPoint +
												   m_RootItem->mapFromItem(pressedItem->parentItem(), pressedItem->position());

							/* Move related item to its new position */
							if (nullptr == itemAtDroppedPoint)
							{
								pressedItem->setParentItem(m_RootItem);
								pressedItem->setPosition(droppedPoint);
							}
							else if (true == childItems.contains(itemAtDroppedPoint))
							{
								pressedItem->setPosition(pressedItem->parentItem()->mapFromItem(m_RootItem, droppedPoint));
							}
							else if (pressedItem != itemAtDroppedPoint) // else handled in previous else if
							{
								QPointF mappedPoint = itemAtDroppedPoint->mapFromItem(m_RootItem, droppedPoint);
								pressedItem->setParentItem(itemAtDroppedPoint);
								pressedItem->setPosition(mappedPoint);
							}
							fixWebViewPosition(pressedItem);
							ShowSelectionTools(pressedItem);
							event->accept();
							return true;
						}
					}
				}

				/* Non-edit mode things */
				else
				{
					/* Handled drops coming from toolbox */
					if (dropEvent->mimeData()->hasUrls()) // WARNING: All kind of urls enter!
					{
						auto url = dropEvent->mimeData()->urls().at(0);
//						 m_d->designWidget->engine()->clearComponentCache(); //WARNING: Performance issues?
						QQmlComponent component(m_d->designWidget->engine()); //TODO: Drop into another item?
						component.loadUrl(url);

						QQmlIncubator incubator;
						component.create(incubator, m_d->designWidget->rootContext());
						while (incubator.isLoading()) {
							QApplication::processEvents(QEventLoop::AllEvents, 50);
						}
						QQuickItem *qml = qobject_cast<QQuickItem*>(incubator.object());

						if (component.isError() || !qml) {qWarning() << component.errors(); qApp->quit();}

						int count = 1;
						QString componentName = qmlContext(qml)->nameForObject(qml);
						if (componentName.isEmpty()) componentName = "anonymous";
						for (int i=0; i<m_Items.size();i++) {
							if (componentName == QString(m_d->designWidget->rootContext()->nameForObject(m_Items[i])) ||
								componentName == QString("dpi")) {
								componentName += QString::number(count);
								count++;
								i = 0;
							}
						}
						m_d->designWidget->rootContext()->setContextProperty(componentName, qml);
						qml->setParentItem(m_RootItem);
						qml->setPosition(qml->mapFromItem(m_RootItem, dropEvent->pos()));
						qml->setClip(true); // Even if it's not true
						fit(qml, Fit::WidthHeight);
						m_Items << qml;

						QTimer::singleShot(200, [qml, this] { fixWebViewPosition(qml); });
						event->accept();
						return true;
					}
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

						QString mimeType = "designarea/x-qquickitem";
						QMimeData *mimeData = new QMimeData;
						mimeData->setData(mimeType, variant.toByteArray());

						QDrag *drag = new QDrag(this);
						drag->setMimeData(mimeData);
						QPointF diffPoint = dragStartPoint - m_RootItem->mapFromItem(pressedItem->parentItem(), pressedItem->position());
						drag->setHotSpot(diffPoint.toPoint());

						QSharedPointer<QQuickItemGrabResult> result = pressedItem->grabToImage(); // FIXME: On IOS
						connect(result.data(), &QQuickItemGrabResult::ready, this, [=]
						{
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
	m_d->centralWidget->setGeometry(0, 0, width(), height());
	m_d->aboutButton->setGeometry(width()-fit(40), height()-fit(40), fit(30),fit(30));
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
	for (auto citem : m_Items) {
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
	for (auto item : m_Items)
		fixWebViewPosition(item);
}

void MainWindow::DownloadTools(const QUrl& url)
{
	QNetworkAccessManager *manager = new QNetworkAccessManager(this);
	QNetworkRequest request;
	request.setUrl(url);
	request.setRawHeader("User-Agent", "objectwheel 1.0");

	QNetworkReply *reply = manager->get(request);
	connect(reply, static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error),
			this, [] { Q_ASSERT_X(0, "GetTools()", "Network Error"); });
	connect(reply, &QNetworkReply::sslErrors,
			this, [] { Q_ASSERT_X(0, "GetTools()", "Ssl Error"); });
	connect(reply, &QNetworkReply::finished, this, [=]
	{
		QJsonDocument toolsDoc = QJsonDocument::fromJson(reply->readAll());
		QJsonObject toolsObject = toolsDoc.object();

		if (true == CheckTools(toolsObject))
		{
			for (int i = 0; i < toolsObject.size(); i++)
			{
				QString toolName = toolsObject.keys().at(i);
				AddTool(toolName);
			}
			reply->deleteLater();
			return;
		}

		if (!QDir(m_ToolsDir).removeRecursively())
			Q_ASSERT_X(0, "GetTools()", "Can not remove tools dir");

		for (int i = 0; i < toolsObject.size(); i++)
		{
			QString toolName = toolsObject.keys().at(i);
			QDir(m_ToolsDir).mkpath(toolName);
			QJsonObject toolObject = toolsObject.value(toolName).toObject();
			QUrl toolUrl = QUrl::fromUserInput(toolObject.value("toolUrl").toString());

			QNetworkRequest request;
			request.setUrl(toolUrl);
			request.setRawHeader("User-Agent", "objectwheel 1.0");

			QNetworkReply *toolReply = manager->get(request);
			connect(toolReply, static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>
					(&QNetworkReply::error), this, [] { Q_ASSERT_X(0, "GetTools()", "Network Error"); });
			connect(toolReply, &QNetworkReply::sslErrors,
					this, [] { Q_ASSERT_X(0, "GetTools()", "Ssl Error"); });
			connect(toolReply, &QNetworkReply::finished, this, [=]
			{
				ExtractZip(toolReply->readAll(), m_ToolsDir + "/" + toolName);
				AddTool(toolName);
				toolReply->deleteLater();
			});
		}

		reply->deleteLater();
	});
}

void MainWindow::AddTool(const QString& name)
{
	QList<QUrl> urls;
	QListWidgetItem* item = new QListWidgetItem(QIcon(m_ToolsDir + "/" + name + "/icon.png"), name);
	urls << QUrl::fromLocalFile(m_ToolsDir + "/" + name + "/main.qml");
	m_d->toolboxWidget->addItem(item);
	m_d->toolboxWidget->AddUrls(item,urls);
}

bool MainWindow::CheckTools(const QJsonObject& toolsObject) const
{
	//	if (!QDir().exists(m_ToolsDir))
	//		return false;

	//	for (int i = 0; i < toolsObject.size(); i++)
	//	{
	//		QString toolName = toolsObject.keys().at(i);
	//		if (!QDir(m_ToolsDir).exists(toolName))
	//			return false;
	//	}

	//	return true;
	Q_UNUSED(toolsObject);
	return false;
}

void MainWindow::ExtractZip(const QByteArray& zipData, const QString& path) const
{
	mz_zip_archive zip;
	memset(&zip, 0, sizeof(zip));

	if (!mz_zip_reader_init_mem(&zip, zipData.constData(), zipData.size(), 0))
	{
		Q_ASSERT_X(0, "ExtractZip()", "Zip file is corrupt");
	}

	// Spin for dirs
	for (int i = 0; i < (int)mz_zip_reader_get_num_files(&zip); i++)
	{
		mz_zip_archive_file_stat file_stat;
		if (!mz_zip_reader_file_stat(&zip, i, &file_stat))
		{
			Q_ASSERT_X(0, "ExtractZip()","mz_zip_reader_file_stat() failed!");
			mz_zip_reader_end(&zip);
		}

		if (mz_zip_reader_is_file_a_directory(&zip, i))
		{
			QDir(path).mkpath(file_stat.m_filename);
		}
	}

	// Spin for data
	for (int i = 0; i < (int)mz_zip_reader_get_num_files(&zip); i++)
	{
		mz_zip_archive_file_stat file_stat;
		if (!mz_zip_reader_file_stat(&zip, i, &file_stat))
		{
			Q_ASSERT_X(0, "ExtractZip()", "mz_zip_reader_file_stat() failed!");
			mz_zip_reader_end(&zip);
		}

		if (!mz_zip_reader_is_file_a_directory(&zip, i))
		{
			if (!mz_zip_reader_extract_to_file(&zip, i, QString(path+"/"+file_stat.m_filename).toStdString().c_str(), 0))
			{
				Q_ASSERT_X(0, "ExtractZip()", "mz_zip_reader_extract_to_file() failed!");
				mz_zip_reader_end(&zip);
			}
		}
	}

	// Close the archive, freeing any resources it was using
	mz_zip_reader_end(&zip);
}

QQuickItem* MainWindow::GetDeepestDesignItemOnPoint(const QPoint& point) const
{
	QPointF pt = point;
	QQuickItem* parent = m_RootItem;
	QQuickItem* item = parent->childAt(pt.x(), pt.y());

	while (item && m_Items.contains(item)) {
		pt = item->mapFromItem(parent, pt);
		parent = item;
		item = parent->childAt(pt.x(), pt.y());
	}

	if (parent == m_RootItem) return nullptr;
	return parent;
}

const MainWindow::QQuickItemList MainWindow::GetAllChildren(QQuickItem* const item) const
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
	m_ResizerTick->SetTrackedItem(selectedItem);
	m_ResizerTick->show();
	m_RotatorTick->SetTrackedItem(selectedItem);
	m_RotatorTick->show();
	m_RemoverTick->SetTrackedItem(selectedItem);
	m_RemoverTick->show();
	emit selectionShowed(selectedItem);
}

void MainWindow::HideSelectionTools()
{
	m_ResizerTick->hide();
	m_RemoverTick->hide();
	m_RotatorTick->hide();
	emit selectionHided();
}

void MainWindow::on_clearButton_clicked()
{
	m_d->bindingWidget->clearAllBindings();
	for (auto item : m_Items)
		item->deleteLater();
	m_Items.clear();
}

void MainWindow::on_editButton_clicked()
{
	for (auto item : m_Items) {
		item->setEnabled(!m_d->editButton->isChecked());
	}
	m_d->propertiesWidget->clearList();
	m_d->bindingWidget->clearList();
}

void MainWindow::SetToolsDir()
{
#if defined(Q_OS_IOS) || defined(Q_OS_ANDROID) || defined(Q_OS_WINPHONE)
	m_ToolsDir = QStandardPaths::standardLocations(QStandardPaths::DataLocation).value(0) + "/tools";
#else
	m_ToolsDir = "./tools";
#endif
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
// FIXME: Make CheckTools function works
// FIXME: changed qml file doesn't make changes on designer due to source url is same
