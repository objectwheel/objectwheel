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

MainWindow::~MainWindow()
{
	delete ui;
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

void MainWindow::on_clearButton_clicked()
{
	/* Delete design items */
	foreach(QQuickItem* item, ui->designWidget->rootObject()->childItems())
		item->deleteLater();
}

void MainWindow::on_editButton_clicked()
{
	/* Enable/Disable design items */
	foreach(QQuickItem* item, ui->designWidget->rootObject()->childItems())
		item->setEnabled(ui->editButton->isChecked());
}

bool MainWindow::eventFilter(QObject* object, QEvent* event)
{
	if (object == ui->designWidget)
	{
		switch (event->type())
		{
			case QEvent::DragEnter: /* Drag & Drop things */
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
				if (dropEvent->mimeData()->hasUrls())
				{
					QQmlComponent component(ui->designWidget->engine());
					component.loadUrl(dropEvent->mimeData()->urls().at(0));
					QQuickItem *qml = qobject_cast<QQuickItem*>(component.create(ui->designWidget->rootContext()));
					qml->setParentItem(ui->designWidget->rootObject());
					qml->setPosition(qml->mapFromGlobal(QCursor::pos()));
					ui->designWidget->rootContext()->setContextProperty(qml->objectName(), qml);
					Fitter::AddItem(qml, Fit::WidthHeight);
				}
				return true;
			}

			case QMouseEvent::MouseButtonRelease: /* Move ticks to clicked item */
			{
				QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
				if (!ui->editButton->isChecked())
				{
					QQuickItem* trackedItem = ui->designWidget->rootObject()->childAt(mouseEvent->x(),mouseEvent->y());
					if (nullptr != trackedItem)
					{
						m_ResizerTick->setParent(ui->designWidget);
						m_ResizerTick->SetTrackedItem(trackedItem);
						m_ResizerTick->show();

						m_RemoverTick->setParent(ui->designWidget);
						m_RemoverTick->SetTrackedItem(trackedItem);
						m_RemoverTick->show();
					}
				}
			}

			default:
				return false;
		}
	}
	else
		return QMainWindow::eventFilter(object,event);
}
