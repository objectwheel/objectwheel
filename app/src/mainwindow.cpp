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
#include <ui_mainwindow.h>
#include <container.h>
#include <css.h>

#include <QtQml>
#include <QtCore>
#include <QtQuick>
#include <QtWidgets>
#include <QtNetwork>

using namespace Fit;

MainWindow::MainWindow(QWidget *parent)
	: QWidget(parent)
	, ui(new Ui::MainWindow)
	, m_ResizerTick(new ResizerTick)
	, m_RotatorTick(new RotatorTick)
	, m_RemoverTick(new RemoverTick)
	, m_RootItem(nullptr)
	, m_LeftMenu(new CoverMenu)
	, m_RightMenu(new CoverMenu)
{
	ui->setupUi(this);
	SetToolsDir();
	SetupGui();
	DownloadTools(QUrl::fromUserInput("qrc:/resources/tools/tools.json"));
}

void MainWindow::SetupGui()
{
	/* Scaling things */
	fit(ui->editButton, Fit::WidthHeight, true);
	fit(ui->clearButton, Fit::WidthHeight, true);

	ui->centralWidget->layout()->setContentsMargins(0,0,0,fit(8));
	ui->buttonsLayout->setSpacing(fit(5));

	/* Set ticks' icons */
	m_ResizerTick->setIcon(QIcon(":/resources/images/resize-icon.png"));
	m_RemoverTick->setIcon(QIcon(":/resources/images/delete-icon.png"));
	m_RotatorTick->setIcon(QIcon(":/resources/images/rotate-icon.png"));

	/* Assign design area's root object */
	m_RootItem = ui->designWidget->rootObject();

	/* Toolbox stylizing */
	ui->toolboxWidget->setIconSize(fit({30, 30}));

	/* Start filtering design area */
	ui->designWidget->installEventFilter(this);

	/* Hide ticks when tracked item removed */
	connect(m_RemoverTick, &RemoverTick::ItemRemoved, m_ResizerTick, &ResizerTick::hide);
	connect(m_RemoverTick, &RemoverTick::ItemRemoved, m_RotatorTick, &RotatorTick::hide);
	connect(m_RemoverTick, &RemoverTick::ItemRemoved, ui->propertiesWidget, &PropertiesWidget::clearList);

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
	connect(ui->editButton, &QPushButton::clicked, m_ResizerTick, &ResizerTick::hide);
	connect(ui->editButton, &QPushButton::clicked, m_RemoverTick, &RemoverTick::hide);
	connect(ui->editButton, &QPushButton::clicked, m_RotatorTick, &RotatorTick::hide);

	/* Enable/Disable other controls when editButton clicked */
	connect(ui->editButton, &QPushButton::toggled, [this](bool checked) {ui->toolboxWidget->setEnabled(!checked);});
	connect(ui->editButton, &QPushButton::toggled, [this](bool checked) {ui->propertiesWidget->setEnabled(checked);});
	connect(ui->editButton, &QPushButton::toggled, [this](bool checked) {ui->clearButton->setEnabled(!checked);});

	/* Set ticks' Parents and hide ticks */
	m_ResizerTick->setParent(ui->designWidget);
	m_RemoverTick->setParent(ui->designWidget);
	m_RotatorTick->setParent(ui->designWidget);
	m_ResizerTick->hide();
	m_RemoverTick->hide();
	m_RotatorTick->hide();

	/* Add Tool Menu */
	m_LeftMenu->setCoverWidget(ui->centralWidget);
	m_LeftMenu->setCoverSide(CoverMenu::FromLeft);
	connect(this,SIGNAL(resized()),m_LeftMenu,SLOT(hide()));
	connect(this,&MainWindow::resized, [this] { ui->titleBar->setMenuChecked(false); });

	/* Add Properties Menu */
	m_RightMenu->setCoverWidget(ui->centralWidget);
	m_RightMenu->setCoverSide(CoverMenu::FromRight);
	connect(this,SIGNAL(resized()),m_RightMenu,SLOT(hide()));
	connect(this,&MainWindow::resized, [this] { ui->titleBar->setSettingsChecked(false); });

	/* Add Title Bar */
	fit(ui->titleBar, Fit::Height, true);
	ui->titleBar->setText("Studio");
	ui->titleBar->setColor("#2196f3");
	ui->titleBar->setShadowColor("#e0e4e7");
	connect(ui->titleBar, SIGNAL(MenuToggled(bool)), m_LeftMenu, SLOT(setCovered(bool)));
	connect(ui->titleBar, SIGNAL(SettingsToggled(bool)), m_RightMenu, SLOT(setCovered(bool)));
	connect(m_LeftMenu, SIGNAL(toggled(bool)), ui->titleBar, SLOT(setMenuChecked(bool)));
	connect(m_RightMenu, SIGNAL(toggled(bool)), ui->titleBar, SLOT(setSettingsChecked(bool)));

	/* Prepare Properties Widget */
	connect(this, SIGNAL(selectionShowed(QObject*const)), ui->propertiesWidget, SLOT(refreshList(QObject*const)));
	connect(this, &MainWindow::selectionHided, [this] { ui->propertiesWidget->setDisabled(true); });

	/* Pop-up toolbox widget's scrollbar */
	connect(m_LeftMenu, &CoverMenu::toggled, [this](bool checked) {if (checked) ui->toolboxWidget->showBar(); });
	connect(m_RightMenu, &CoverMenu::toggled, [this](bool checked) {if (checked) ui->propertiesWidget->showBar(); });

	/* Set flat buttons' colors*/
	ui->editButton->setColor(QColor("#2196f3"));
	ui->editButton->setCheckedColor(QColor("#72b240"));
	ui->editButton->setTextColor(Qt::white);
	ui->editButton->setCheckedTextColor(QColor("#444444"));
	ui->clearButton->setColor(QColor("#d95459"));
	ui->clearButton->setDisabledColor(Qt::darkGray);
	ui->clearButton->setDisabledTextColor(QColor("#444444"));
	ui->clearButton->setTextColor(Qt::white);

	/* Fix Coords of ticks when property changed */
	connect(ui->propertiesWidget, &PropertiesWidget::propertyChanged, m_RemoverTick, &RemoverTick::FixCoord);
	connect(ui->propertiesWidget, &PropertiesWidget::propertyChanged, m_RotatorTick, &RotatorTick::FixCoord);
	connect(ui->propertiesWidget, &PropertiesWidget::propertyChanged, m_ResizerTick, &ResizerTick::FixCoord);

    ui->designWidget->rootContext()->setContextProperty("dpi", Fit::ratio());

	/* Init Left Container */
	QVariant toolboxVariant;
	toolboxVariant.setValue<QWidget*>(ui->toolboxWidget);
	QVariant propertiesVariant;
	propertiesVariant.setValue<QWidget*>(ui->propertiesWidget);
	Container* leftContainer = new Container;
	leftContainer->addWidget(ui->toolboxWidget);
	leftContainer->addWidget(ui->propertiesWidget);
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
	QRadioButton* propertiesButton = new QRadioButton;
	propertiesButton->setStyleSheet(CSS::PropertiesButton);
	propertiesButton->setCheckable(true);
	QWidgetAction* propertiesButtonAction = new QWidgetAction(this);
	propertiesButtonAction->setDefaultWidget(propertiesButton);
	propertiesButtonAction->setData(propertiesVariant);
	propertiesButtonAction->setCheckable(true);
	leftToolbar->addAction(propertiesButtonAction);
	connect(propertiesButton, SIGNAL(clicked(bool)), propertiesButtonAction, SLOT(trigger()));
	connect(toolboxButtonAction, SIGNAL(triggered(bool)), leftContainer, SLOT(handleAction()));
	connect(propertiesButtonAction, SIGNAL(triggered(bool)), leftContainer, SLOT(handleAction()));
	QWidget* leftMenuWidget = new QWidget;
	leftMenuWidget->setObjectName("leftMenuWidget");
	leftMenuWidget->setStyleSheet("#leftMenuWidget{background:#566573;}");
	QVBoxLayout* leftMenuLayout = new QVBoxLayout(leftMenuWidget);
	leftMenuLayout->setContentsMargins(0, 0, 0, 0);
	leftMenuLayout->setSpacing(fit(8));
	leftMenuLayout->addWidget(leftToolbar);
	leftMenuLayout->addWidget(leftContainer);
	m_LeftMenu->attachWidget(leftMenuWidget);

	ui->propertiesWidget->setRootContext(ui->designWidget->rootContext());
}


bool MainWindow::eventFilter(QObject* object, QEvent* event)
{
	/* Handle events which are happening on design area */
	if (object == ui->designWidget)
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
				if (ui->editButton->isChecked())
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
							fixPosition(pressedItem);
							ShowSelectionTools(pressedItem);
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
						QQmlComponent component(ui->designWidget->engine()); //TODO: Drop into another item?
						component.loadUrl(url);

						QQmlIncubator incubator;
						component.create(incubator, ui->designWidget->rootContext());
						while (!incubator.isReady()) {
							QApplication::processEvents(QEventLoop::AllEvents, 50);
						}
						QQuickItem *qml = qobject_cast<QQuickItem*>(incubator.object());

						if (component.isError() || !qml) qWarning() << component.errors() << incubator.errors();
						ui->designWidget->rootContext()->setContextProperty(qmlContext(qml)->nameForObject(qml), qml);
						qml->setParentItem(m_RootItem);
						qml->setPosition(qml->mapFromItem(m_RootItem, dropEvent->pos()));
						qml->setClip(true); // Even if it's not true
						fit(qml, Fit::WidthHeight);
						m_Items << qml;
					}
				}

				return false;
			}

			case QMouseEvent::MouseButtonPress:
			{
				QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);

				/* Edit mode things */
				if (ui->editButton->isChecked())
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
				if (ui->editButton->isChecked())
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
						connect(result.data(), &QQuickItemGrabResult::ready, this, [drag, result]
						{
							drag->setPixmap(QPixmap::fromImage(result.data()->image()));
							drag->exec();
						});
					}
				}

				return false;
			}

			default:
				return false;
		}
	}
	else
		return QWidget::eventFilter(object,event);
}

void MainWindow::resizeEvent(QResizeEvent* event)
{
	ui->centralWidget->setGeometry(0, 0, width(), height());
	QWidget::resizeEvent(event);
	emit resized();
}

void MainWindow::fixPosition(QQuickItem* const item)
{
	item->setWidth(item->width() + 1);
	QTimer::singleShot(0, [=]{
		item->setWidth(item->width() - 1);
	});
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
	ui->toolboxWidget->addItem(item);
	ui->toolboxWidget->AddUrls(item,urls);
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
	for (auto item : m_Items)
		item->deleteLater();
	m_Items.clear();
}

void MainWindow::on_editButton_clicked()
{
	for (auto item : m_Items) {
		item->setEnabled(!ui->editButton->isChecked());
	}
	ui->propertiesWidget->clearList();
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
	delete ui;
}

// TODO: Pop-up loading screen
// TODO: Make a object clone tick
// TODO: Make it possible to resize(zoom) and rotate operations with fingers
// TODO: Layouts?
// TODO: Code a version numberer for tools' objectNames
// FIXME: Make CheckTools function works
// FIXME: changed qml file doesn't make changes on designer due to source url is same
