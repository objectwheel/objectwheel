#include <miniz.h>
#include <fitter.h>
#include <string.h>
#include <listwidget.h>
#include <mainwindow.h>
#include <resizertick.h>
#include <rotatortick.h>
#include <removertick.h>
#include <ui_mainwindow.h>

#include <QtQml>
#include <QtCore>
#include <QtQuick>
#include <QtWidgets>
#include <QtNetwork>

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
	, ui(new Ui::MainWindow)
	, m_ToolsDir("tools")
	, m_ResizerTick(new ResizerTick)
	, m_RotatorTick(new RotatorTick)
	, m_RemoverTick(new RemoverTick)
	, m_RootItem(nullptr)
{
	ui->setupUi(this);
	SetupGui();
	DownloadTools(QUrl::fromUserInput("qrc:/resources/tools/tools.json"));
}

void MainWindow::SetupGui()
{
	/* Scaling things */
	Fitter::AddWidget(ui->editButton, Fit::WidthHeight | Fit::LaidOut);
	Fitter::AddWidget(ui->clearButton, Fit::WidthHeight | Fit::LaidOut);
	Fitter::AddWidget(ui->toolboxWidget, Fit::WidthScaling | Fit::LaidOut);
	Fitter::AddWidget(ui->toolboxTitle, Fit::WidthHeight | Fit::LaidOut);
	Fitter::AddWidget(ui->designTitle, Fit::HeightScaling | Fit::LaidOut);

	/* Set ticks' icons */
	m_ResizerTick->setIcon(QIcon(":/resources/images/resize-icon.png"));
	m_RemoverTick->setIcon(QIcon(":/resources/images/delete-icon.png"));
	m_RotatorTick->setIcon(QIcon(":/resources/images/rotate-icon.png"));

	/* Assign design area's root object */
	m_RootItem = ui->designWidget->rootObject();

	/* Layout spacing things */
	ui->leftLayout->setSpacing(fit(6));
	ui->downsideLayout->setSpacing(fit(6));
	ui->mainLayout->setSpacing(fit(6));
	ui->mainLayout->setContentsMargins(fit(9),fit(9),fit(9),fit(9));

	/* Toolbox touch-shift things */
	QScroller::grabGesture(ui->toolboxWidget, QScroller::TouchGesture);

	/* Toolbox stylizing */
	ui->toolboxWidget->setIconSize(fit({30, 30}));

	/* Start filtering design area */
	ui->designWidget->installEventFilter(this);

	/* Hide ticks when tracked item removed */
	connect(m_RemoverTick, &RemoverTick::ItemRemoved, m_ResizerTick, &ResizerTick::hide);
	connect(m_RemoverTick, &RemoverTick::ItemRemoved, m_RotatorTick, &RotatorTick::hide);

	/* Remove deleted items from internal item list */
	connect(m_RemoverTick, &RemoverTick::ItemRemoved, this, &MainWindow::RemoveItem);

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
	connect(ui->editButton, &QPushButton::toggled, ui->toolboxWidget, &ListWidget::setEnabled);
	connect(ui->editButton, &QPushButton::toggled, ui->clearButton, &QPushButton::setEnabled);

	/* Set ticks' Parents and hide ticks */
	m_ResizerTick->setParent(ui->designWidget);
	m_RemoverTick->setParent(ui->designWidget);
	m_RotatorTick->setParent(ui->designWidget);
	m_ResizerTick->hide();
	m_RemoverTick->hide();
	m_RotatorTick->hide();
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
				if (!ui->editButton->isChecked())
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
							else if (pressedItem != itemAtDroppedPoint)
							{
								QPointF mappedPoint = itemAtDroppedPoint->mapFromItem(m_RootItem, droppedPoint);
								pressedItem->setParentItem(itemAtDroppedPoint->childItems()[0]);
								pressedItem->setPosition(mappedPoint);
							}
							QTimer::singleShot(100, [=]{ ShowSelectionTools(pressedItem); });
						}
					}
				}

				/* Non-edit mode things */
				else
				{
					/* Handled drops coming from toolbox */
					if (dropEvent->mimeData()->hasUrls()) // WARNING: All kind of urls enter!
					{
						QQmlComponent component(ui->designWidget->engine());
						component.loadUrl(dropEvent->mimeData()->urls().at(0));
						QQuickItem *qml = qobject_cast<QQuickItem*>(component.create(ui->designWidget->rootContext()));
						ui->designWidget->rootContext()->setContextProperty(qml->objectName(), qml);

						QQmlComponent designComponent(ui->designWidget->engine());
						designComponent.loadUrl(QUrl("qrc:/resources/qmls/design-item.qml"));
						QQuickItem *qmlDesing = qobject_cast<QQuickItem*>(designComponent.create(ui->designWidget->rootContext()));
						qmlDesing->setParentItem(m_RootItem);
						qmlDesing->setPosition(qmlDesing->mapFromItem(m_RootItem, dropEvent->pos()));
						qmlDesing->setWidth(qml->width());
						qmlDesing->setHeight(qml->height());
						Fitter::AddItem(qmlDesing, Fit::WidthHeight);
						qml->setParentItem(qmlDesing);
						m_Items << qmlDesing;

						QVariant variant;
						variant.setValue(qmlDesing);
						QQmlProperty::write(qml, "anchors.margins", 1);
						QQmlProperty::write(qml, "anchors.fill", variant);

						QQmlProperty::write(qmlDesing, "Layout.fillHeight",
											QQmlProperty::read(qml, "Layout.fillHeight", qmlContext(qml)), qmlContext(qmlDesing));
						QQmlProperty::write(qmlDesing, "Layout.fillWidth",
											QQmlProperty::read(qml, "Layout.fillWidth", qmlContext(qml)), qmlContext(qmlDesing));
						QQmlProperty::write(qmlDesing, "Layout.alignment",
											QQmlProperty::read(qml, "Layout.alignment", qmlContext(qml)), qmlContext(qmlDesing));
						QQmlProperty::write(qmlDesing, "Layout.margins",
											QQmlProperty::read(qml, "Layout.margins", qmlContext(qml)), qmlContext(qmlDesing));
						QQmlProperty::write(qmlDesing, "Layout.maximumHeight",
											QQmlProperty::read(qml, "Layout.maximumHeight", qmlContext(qml)), qmlContext(qmlDesing));
						QQmlProperty::write(qmlDesing, "Layout.maximumWidth",
											QQmlProperty::read(qml, "Layout.maximumWidth", qmlContext(qml)), qmlContext(qmlDesing));
						QQmlProperty::write(qmlDesing, "Layout.minimumHeight",
											QQmlProperty::read(qml, "Layout.minimumHeight", qmlContext(qml)), qmlContext(qmlDesing));
						QQmlProperty::write(qmlDesing, "Layout.minimumWidth",
											QQmlProperty::read(qml, "Layout.minimumWidth", qmlContext(qml)), qmlContext(qmlDesing));
					}
				}

				return false;
			}

			case QMouseEvent::MouseButtonPress:
			{
				QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);

				/* Edit mode things */
				if (!ui->editButton->isChecked())
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
				if (!ui->editButton->isChecked())
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

						QSharedPointer<QQuickItemGrabResult> result = pressedItem->grabToImage();
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
		return QMainWindow::eventFilter(object,event);
}

void MainWindow::DownloadTools(const QUrl& url)
{
	QString m_ToolsDir = "tools";
	QNetworkAccessManager *manager = new QNetworkAccessManager(this);
	QNetworkRequest request;
	request.setUrl(url);
	request.setRawHeader("User-Agent", "DemoApp 1.0");

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
			request.setRawHeader("User-Agent", "DemoApp 1.0");

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
	QQuickItem* item = m_RootItem->childAt(point.x(), point.y());

	if (nullptr != item)
	{
		QQuickItem* designItem = nullptr;
		if (0 == QString::compare(item->objectName(), "design-item"))
			designItem = item;

		QPointF newPos = item->mapFromItem(m_RootItem, {point});
		while (nullptr != item->childAt(newPos.x(), newPos.y()))
		{
			item = item->childAt(newPos.x(), newPos.y());
			newPos = item->mapFromItem(item->parentItem(), newPos);

			if (0 == QString::compare(item->objectName(), "design-item"))
				designItem = item;
		}
		return designItem;
	}

	return nullptr;
}

const MainWindow::QQuickItemList MainWindow::GetAllChildren(QQuickItem* const item) const
{
	QQuickItemList childList;
	for (auto child : item->childItems())
		childList << GetAllChildren(child);
	childList << item;
	return childList;
}

void MainWindow::ShowSelectionTools(QQuickItem* const selectedItem)
{
	HideSelectionTools();

	m_ResizerTick->SetTrackedItem(selectedItem);
	m_ResizerTick->show();

	m_RotatorTick->SetTrackedItem(selectedItem);
	m_RotatorTick->show();

	m_RemoverTick->SetTrackedItem(selectedItem);
	m_RemoverTick->show();

	QQmlProperty::write(selectedItem, "border.color", "blue");
}

void MainWindow::HideSelectionTools()
{
	m_ResizerTick->hide();
	m_RemoverTick->hide();
	m_RotatorTick->hide();

	for (auto item : m_Items)
		QQmlProperty::write(item, "border.color", "gray");
}

void MainWindow::on_clearButton_clicked()
{
	/* Delete design items */
	for (auto item : m_Items)
		item->deleteLater();

	/* Clear list */
	m_Items.clear();
}

void MainWindow::on_editButton_clicked()
{
	/* Enable/Disable design items */
	for (auto item : m_Items)
	{
		item->setEnabled(ui->editButton->isChecked());
		if (ui->editButton->isChecked())
			QQmlProperty::write(item, "border.color", "transparent");
		else
			QQmlProperty::write(item, "border.color", "gray");
	}
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::RemoveItem(QQuickItem* const item)
{
	for (auto child : GetAllChildren(item))
		if (m_Items.contains(child))
			m_Items.removeAll(child);
}

// TODO: Make a object copy tick
// TODO: Make ticks ordered nicely
// TODO: Make it possible to resize(zoom) and rotate operations with fingers
// TODO: Make main design area layoutable
// TODO: Code a version numberer for tools' objectNames
// FIXME: Make CheckTools function works
// FIXME: changed qml file doesn't make changes on designer due to source url is same
