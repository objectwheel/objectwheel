#include <miniz.h>
#include <fitter.h>
#include <string.h>
#include <listwidget.h>
#include <mainwindow.h>
#include <resizertick.h>
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
	, m_RemoverTick(new RemoverTick)
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

	/* Layout spacing things */
	ui->upsideLayout->setSpacing(fit(6));
	ui->downsideLayout->setSpacing(fit(6));
	ui->mainLayout->setSpacing(fit(6));
	ui->mainLayout->setContentsMargins(fit(9),fit(9),fit(9),fit(9));

	/* Load items' selection effect */
	QQmlComponent component(ui->designWidget->engine());
	component.loadUrl(QUrl("qrc:/resources/qmls/selection-effect.qml"));
	m_SelectionEffect = qobject_cast<QQuickItem*>(component.create(ui->designWidget->rootContext()));
	m_SelectionEffect->setParentItem(ui->designWidget->rootObject());
	ui->designWidget->rootContext()->setContextProperty(m_SelectionEffect->objectName(), m_SelectionEffect);

	/* Toolbox touch-shift things */
	QScroller::grabGesture(ui->toolboxWidget, QScroller::LeftMouseButtonGesture);

	/* Toolbox stylizing */
	ui->toolboxWidget->setIconSize(fit({30, 30}));

	/* Start filtering design area */
	ui->designWidget->installEventFilter(this);

	/* Hide ticks when tracked item removed */
	connect(m_RemoverTick, &RemoverTick::ItemRemoved, m_ResizerTick, &ResizerTick::hide);

	/* Re-move ticks when tracked item resized */
	connect(m_ResizerTick, &ResizerTick::ItemResized, m_RemoverTick, &RemoverTick::FixCoord);

	/* Hide ticks when editButton clicked */
	connect(ui->editButton, &QPushButton::clicked, m_ResizerTick, &ResizerTick::hide);
	connect(ui->editButton, &QPushButton::clicked, m_RemoverTick, &RemoverTick::hide);

	/* Enable/Disable other controls when editButton clicked */
	connect(ui->editButton, &QPushButton::toggled, ui->toolboxWidget, &ListWidget::setEnabled);
	connect(ui->editButton, &QPushButton::toggled, ui->clearButton, &QPushButton::setEnabled);

	/* Set ticks' Parents and hide ticks */
	m_ResizerTick->setParent(ui->designWidget);
	m_RemoverTick->setParent(ui->designWidget);
	m_ResizerTick->hide();
	m_RemoverTick->hide();

	/* Clear selection effect when it's necessary */
	connect(m_RemoverTick, &RemoverTick::ItemRemoved, this, &MainWindow::ClearSelectionEffect);
	connect(ui->editButton, &QPushButton::clicked, this, &MainWindow::ClearSelectionEffect);
}


bool MainWindow::eventFilter(QObject* object, QEvent* event)
{
	/* Handle events which are happening on design area */
	if (object == ui->designWidget)
	{
		/* Design area's events' shared variables */
		static QPoint dragStartPoint;

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
					QQuickItem* trackedItem = ui->designWidget->rootObject()->childAt(dragStartPoint.x(), dragStartPoint.y());

					if (nullptr != trackedItem)
					{
						/* Handled drops coming from design area itself */
						if (true == dropEvent->mimeData()->hasFormat("designarea/x-qquickitem"))
						{
							trackedItem->setPosition(dropEvent->pos() - dragStartPoint + trackedItem->position().toPoint());
							ShowSelectionTools(trackedItem);
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
						qml->setParentItem(ui->designWidget->rootObject());
						qml->setPosition(qml->mapFromGlobal(QCursor::pos()));
						ui->designWidget->rootContext()->setContextProperty(qml->objectName(), qml);
						Fitter::AddItem(qml, Fit::WidthHeight);
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
					QQuickItem* trackedItem = ui->designWidget->rootObject()->childAt(mouseEvent->x(), mouseEvent->y());

					/* Show selection tools */
					if (nullptr != trackedItem)
					{
						QQuickItem* source = qvariant_cast<QQuickItem*>(QQmlProperty::read(m_SelectionEffect, "source"));
						if (source != trackedItem)
							ShowSelectionTools(trackedItem);
						else if (m_ResizerTick->isHidden())
							ShowSelectionTools(trackedItem);
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
					QQuickItem* trackedItem = ui->designWidget->rootObject()->childAt(dragStartPoint.x(), dragStartPoint.y());

					/* Made Drags from design area */
					if (nullptr != trackedItem)
					{
						if (!(mouseEvent->buttons() & Qt::LeftButton))
							return false;
						if ((mouseEvent->pos() - dragStartPoint).manhattanLength()
							< QApplication::startDragDistance())
							return false;

						QVariant variant;
						variant.setValue<QQuickItem*>(trackedItem);

						QString mimeType = "designarea/x-qquickitem";
						QMimeData *mimeData = new QMimeData;
						mimeData->setData(mimeType, variant.toByteArray());

						QDrag *drag = new QDrag(this);
						drag->setMimeData(mimeData);
						drag->setHotSpot(dragStartPoint - trackedItem->position().toPoint());

						QSharedPointer<QQuickItemGrabResult> result = trackedItem->grabToImage();
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

void MainWindow::ShowSelectionTools(QQuickItem* const selectedItem)
{
	QVariant variant;
	variant.setValue(selectedItem);
	QQmlProperty::write(m_SelectionEffect, "source", variant);
	QQmlProperty::write(m_SelectionEffect, "anchors.fill", variant);
	QQmlProperty::write(m_SelectionEffect, "visible", true);

	m_ResizerTick->SetTrackedItem(selectedItem);
	m_ResizerTick->show();

	m_RemoverTick->SetTrackedItem(selectedItem);
	m_RemoverTick->show();
}

void MainWindow::HideSelectionTools()
{
	ClearSelectionEffect();
	m_ResizerTick->hide();
	m_RemoverTick->hide();
}

void MainWindow::on_clearButton_clicked()
{
	/* Delete design items */
	foreach(QQuickItem* item, ui->designWidget->rootObject()->childItems())
		if (m_SelectionEffect != item)
			item->deleteLater();
}

void MainWindow::on_editButton_clicked()
{
	/* Enable/Disable design items */
	foreach(QQuickItem* item, ui->designWidget->rootObject()->childItems())
		item->setEnabled(ui->editButton->isChecked());
}

void MainWindow::ClearSelectionEffect()
{
	/* Clear selection effect */
	QVariant variant;
	variant.setValue(ui->designWidget->rootObject());
	QQmlProperty::write(m_SelectionEffect, "source", variant);
	QQmlProperty::write(m_SelectionEffect, "anchors.fill", variant);
	QQmlProperty::write(m_SelectionEffect, "visible", false);
}

MainWindow::~MainWindow()
{
	delete ui;
}

// TODO: Code a version numberer for tools' objectNames
// TODO: Make layouts works
// FIXME: Make CheckTools works
// FIXME: changed qml file doesn't make changes on designer due to source url is same
// FIXME: Make CheckBox tool's internal "rectangle" visible
// TODO: Review selection effect
