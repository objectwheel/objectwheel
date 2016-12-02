#include <fit.h>
#include <editortick.h>
#include <QtGui>
#include <QtCore>
#include <QtQuick>
#include <QtWidgets>
#include <QtQuickWidgets>

#if !defined(Q_OS_IOS) && !defined(Q_OS_ANDROID) && !defined(Q_OS_WINPHONE)
#    define EDITORTICK_SIZE 15
#else
#    define EDITORTICK_SIZE 25
#endif

using namespace Fit;

EditorTick::EditorTick(QWidget* const parent)
	: QPushButton(parent)
	, m_TrackedItem(nullptr)
{
	connect(this, SIGNAL(clicked(bool)), this, SLOT(OpenEditor()));
	setCursor(QCursor(Qt::PointingHandCursor));
	resize(EDITORTICK_SIZE, EDITORTICK_SIZE);
	fit(this);
}

QQuickItem* EditorTick::TrackedItem() const
{
	return m_TrackedItem;
}

void EditorTick::SetTrackedItem(QQuickItem* const trackedItem)
{
	m_TrackedItem = trackedItem;
	FixCoord();
}

void EditorTick::FixCoord()
{
	QQuickWidget* parent = qobject_cast<QQuickWidget*>(this->parent());
	QQuickItem* rootObject = parent->rootObject();
	QPointF point = rootObject->mapFromItem(m_TrackedItem->parentItem(), m_TrackedItem->position());
	move({static_cast<int>(point.x()),
		  static_cast<int>(point.y() + m_TrackedItem->height())});
}

void EditorTick::paintEvent(QPaintEvent*)
{
	QPainter p(this);
	p.setRenderHint(QPainter::Antialiasing);
	if (isDown())
	{
		QBrush b(QColor("#f37934"));
		p.setPen(QColor("#e36924"));
		p.setBrush(b);
		p.drawEllipse(rect().adjusted(1, 1, -1, -1));
	}
	else
	{
		QBrush b(QColor("#fba026"));
		p.setPen(QColor("#eb9016"));
		p.setBrush(b);
		p.drawEllipse(rect().adjusted(1, 1, -1, -1));
	}
	p.drawPixmap(rect().adjusted(2, 2, -2, -2), icon().pixmap(size() - QSize(4, 4)));
}

QPlainTextEdit* textEdit = nullptr;
bool EditorTick::eventFilter(QObject* const object, QEvent* const event)
{
	if (event->type() == QEvent::KeyPress && object == textEdit)
	{
		QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
		if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter)
		{
			QApplication::inputMethod()->hide();
			return true;
		}
	}
	return false;
}

void EditorTick::OpenEditor() // TODO:
{
	//	QQuickWidget* const previewScene = static_cast<QQuickWidget*>(m_TrackedItem);
	//	//////////////////////////////////////////////////////////
	//	QDialog compiler;
	//	QVBoxLayout generalLay;
	//	QVBoxLayout downLay;
	//	QVBoxLayout upLay;
	//	//////////////////////////////////////////////////////////
	//	QString source = previewScene->source().toString().remove("qrc");
	//	QFile qml(source);
	//	if (!qml.open(QIODevice::ReadOnly | QIODevice::Text)) {
	//		qWarning("Error 0x00000");
	//		return;
	//	}
	//	//////////////////////////////////////////////////////////
	//	QPushButton save;
	//	save.setText("Save ☑");
	//	save.setStyleSheet("QPushButton::!pressed"
	//					   "{"
	//					   "border:1px solid #51ad5d;"
	//					   "background:#61bd6d;"
	//					   "color:white;"
	//					   "}"

	//					   "QPushButton::pressed"
	//					   "{"
	//					   "border:1px solid #31984f;"
	//					   "background:#41a85f;"
	//					   "color:white;"
	//					   "}");
	//	save.setMinimumSize({fit(82),fit(35)});
	//	save.setMaximumSize({fit(82),fit(35)});
	//	//////////////////////////////////////////////////////////
	//	QPushButton test;
	//	test.setText("Test It ☔");
	//	test.setStyleSheet("QPushButton::!pressed"
	//					   "{"
	//					   "border:1px solid #8355a8;"
	//					   "background:#9365b8;"
	//					   "color:white;"
	//					   "}"

	//					   "QPushButton::pressed"
	//					   "{"
	//					   "border:1px solid #352972;"
	//					   "background:#553982;"
	//					   "color:white;"
	//					   "}");
	//	test.setMinimumSize({fit(82),fit(35)});
	//	test.setMaximumSize({fit(82),fit(35)});
	//	//////////////////////////////////////////////////////////
	//	QPushButton cancel;
	//	cancel.setText("Exit ✗");
	//	cancel.setStyleSheet("QPushButton::!pressed"
	//						 "{"
	//						 "border:1px solid #c13928;"
	//						 "background:#d14938;"
	//						 "color:white;"
	//						 "}"

	//						 "QPushButton::pressed"
	//						 "{"
	//						 "border:1px solid #a8211f;"
	//						 "background:#b8312f;"
	//						 "color:white;"
	//						 "}");
	//	cancel.setMinimumSize({fit(82),fit(35)});
	//	cancel.setMaximumSize({fit(82),fit(35)});
	//	//////////////////////////////////////////////////////////
	//	QWidget field;
	//	field.setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
	//	field.setStyleSheet("border:1px solid white; background:#61bd6d; color:white;");

	//	QLabel title2;
	//	title2.setText("Object Preview");
	//	title2.setStyleSheet("background:white;color:black;");
	//	title2.setAlignment(Qt::AlignCenter);

	//	QVBoxLayout fieldLay;
	//	fieldLay.addWidget(&title2);
	//	fieldLay.addWidget(&field);
	//	fieldLay.setSpacing(0);
	//	//////////////////////////////////////////////////////////
	//	QGridLayout quickLay(&field);
	//	QQuickView quick;
	//	QLabel label;
	//	label.setStyleSheet("background:transparent;");
	//	label.setMinimumSize(fit(100),fit(100));
	//	label.setMaximumSize(fit(100),fit(100));
	//	quickLay.addWidget(&label,0,0,Qt::AlignCenter);


	//	QTimer qucikTimer;
	//	qucikTimer.start(200);
	//	connect(&qucikTimer,&QTimer::timeout,this,[&]
	//	{
	//		quick.setPosition(field.mapToGlobal(label.pos()));
	//		quick.resize(label.size());
	//		quick.update();
	//	});


	//	quick.setSource(previewScene->source());
	//	quick.setResizeMode(QQuickView::SizeRootObjectToView);
	//	quick.setFlags(Qt::X11BypassWindowManagerHint | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint);
	//	quick.show();
	//	quick.raise();
	//	//////////////////////////////////////////////////////////
	//	QPlainTextEdit edit;
	//	edit.setStyleSheet("border:1px solid white; background:#2969b0; color:white;");
	//	edit.setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
	//	edit.setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	//	edit.installEventFilter(this);
	//	textEdit = &edit;
	//	QScroller::grabGesture(&edit, QScroller::LeftMouseButtonGesture);

	//	edit.setPlainText(qml.readAll());
	//	qml.close();

	//	QLabel title;
	//	title.setText("QML Code Editor");
	//	title.setStyleSheet("background:white;color:black;");
	//	title.setAlignment(Qt::AlignCenter);

	//	QVBoxLayout editLay;
	//	editLay.addWidget(&title);
	//	editLay.addWidget(&edit);
	//	editLay.setSpacing(0);
	//	//////////////////////////////////////////////////////////
	//	connect(&test,&QPushButton::clicked,this,[&]
	//	{
	//		static int i = 0;
	//		QString fileName = source.split("/").last()+QString("-%1").arg(i);
	//		QDir().remove(source.split("/").last()+QString("-%1").arg(i-1)); i++;
	//		QFile out(fileName);
	//		if (!out.open(QIODevice::WriteOnly))
	//		{
	//			qWarning("Error 0x00001");
	//			return;
	//		}
	//		out.write(QByteArray().insert(0,edit.toPlainText()));
	//		out.close();
	//		quick.setSource(QUrl::fromLocalFile(fileName));
	//		quick.update();
	//	});
	//	//////////////////////////////////////////////////////////
	//	connect(&save,&QPushButton::clicked,this,[&]
	//	{
	//		test.click();
	//		previewScene->setSource(quick.source());
	//		previewScene->resize(fit(50),fit(50));
	//		compiler.accept();
	//	});
	//	//////////////////////////////////////////////////////////
	//	connect(&cancel,&QPushButton::clicked,&compiler,&QDialog::reject);
	//	//////////////////////////////////////////////////////////
	//	QHBoxLayout buttonsLay;
	//	buttonsLay.addWidget(&test);
	//	buttonsLay.addWidget(&save);
	//	buttonsLay.addWidget(&cancel);
	//	buttonsLay.addStretch();
	//	//////////////////////////////////////////////////////////
	//	downLay.addLayout(&buttonsLay);
	//	downLay.setContentsMargins(0,fit(10),0,0);
	//	downLay.setSpacing(fit(10));
	//	//////////////////////////////////////////////////////////
	//	upLay.addLayout(&editLay);
	//	upLay.addLayout(&fieldLay);
	//	upLay.setSpacing(fit(10));
	//	//////////////////////////////////////////////////////////
	//	generalLay.addLayout(&upLay);
	//	generalLay.addLayout(&downLay);
	//	generalLay.setSpacing(0);
	//	//////////////////////////////////////////////////////////
	//	compiler.setStyleSheet("background:#333333;");
	//	compiler.setLayout(&generalLay);
	//	compiler.setModal(true);
	//	compiler.setGeometry(qApp->primaryScreen()->availableGeometry());
	//	QMetaObject::Connection connection = connect(qApp->desktop(),&QDesktopWidget::workAreaResized,this,[&]
	//	{
	//		compiler.setGeometry(qApp->primaryScreen()->availableGeometry());
	//		compiler.update();
	//	});
	//	compiler.exec();
	//	disconnect(connection);
	//	//////////////////////////////////////////////////////////
	//	emit EditorOpened(previewScene);
}
