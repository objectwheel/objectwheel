#include <qmleditor.h>
#include <QVBoxLayout>
#include <QQuickWidget>
#include <QQuickItem>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickTextDocument>
#include <QTextDocument>
#include <QTextOption>
#include <flatbutton.h>
#include <fit.h>
#include <QQmlContext>
#include <QQmlIncubator>
#include <QApplication>
#include <QPainter>
#include <QPainterPath>
#include <QPropertyAnimation>
#include <QTimer>
#include <QQmlProperty>
#include <filemanager.h>
#include <QFileInfo>
#include <QDir>

#define DURATION 400

using namespace Fit;

class QmlEditorPrivate
{
	public:
		QmlEditor* parent;
		QQuickItem* rootItem;
		QVBoxLayout mainLayout;
		QQuickWidget quickWidget;
		FlatButton minimizeButton;
		QList<QQuickItem*>* itemList;
		QList<QUrl>* urlList;
		QQuickItem* lastSelectedItem;
		QTextDocument* textDocument;
		QQmlContext* rootContext;
		QPixmap snapshot;
		QPoint showCenter;
		bool deactive;

		QmlEditorPrivate(QmlEditor* p);
		void resize();
		void saved(const QString& text);
		void selectionChanged();
		const QList<QQuickItem*> GetAllChildren(QQuickItem* const item) const;
		void show(const QString& url);
		void show();
		void hide();
};

QmlEditorPrivate::QmlEditorPrivate(QmlEditor* p)
	: parent(p)
	, deactive(true)
{
	parent->setLayout(&mainLayout);
	mainLayout.addWidget(&quickWidget);
	mainLayout.setSpacing(0);
	mainLayout.setContentsMargins(0, 0, 0, 0);

	CacheCleaner::setEngine(quickWidget.engine());
	CacheCleaner::registerQmlType();

#if !defined(Q_OS_IOS) && !defined(Q_OS_WINPHONE) && !defined(Q_OS_ANDROID)
	quickWidget.rootContext()->setContextProperty("isDesktop", true);
#else
	quickWidget.rootContext()->setContextProperty("isDesktop", false);
#endif
	quickWidget.rootContext()->setContextProperty("dpi", Fit::ratio());

	quickWidget.setSource(QUrl("qrc:/resources/qmls/qml-editor.qml"));
	quickWidget.setResizeMode(QQuickWidget::SizeRootObjectToView);
	quickWidget.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	rootItem = quickWidget.rootObject();
	QObject::connect(rootItem, SIGNAL(saved(QString)), parent, SLOT(saved(const QString&)));
	QObject::connect(rootItem, SIGNAL(currentSaved()), parent, SLOT(hide()));

	QObject *textEdit = rootItem->findChild<QObject*>(QStringLiteral("editor"));
	QQuickTextDocument *quickTextDocument = textEdit->property("textDocument").value<QQuickTextDocument*>();
	textDocument = quickTextDocument->textDocument();
	QTextOption textOptions = textDocument->defaultTextOption();

	QFont font;
	font.setFamily("Liberation Mono");
	font.setStyleHint(QFont::Monospace);
	font.setFixedPitch(true);
	font.setPixelSize(fit(12));
	textEdit->setProperty("font", font);

	const int tabStop = 4;  // 4 characters
	QFontMetrics metrics(font);
	textOptions.setTabStop(tabStop * metrics.width(' '));
	textDocument->setDefaultTextOption(textOptions);
	minimizeButton.setParent(parent);
	minimizeButton.setIconButton(true);
	minimizeButton.setIcon(QIcon(":/resources/images/back.png"));
	minimizeButton.resize(fit(35), fit(35));
	minimizeButton.setCursor(Qt::PointingHandCursor);
	QObject::connect(&minimizeButton, SIGNAL(clicked(bool)), parent, SLOT(hide()));

	QTimer::singleShot(500, [this] {
		auto snap = parent->grab();
		if (!snap.isNull()) {
			snapshot = snap;
			snapshot.setDevicePixelRatio(qApp->devicePixelRatio());
		}
	});
}

void QmlEditorPrivate::resize()
{
	minimizeButton.move(fit(8), parent->height() - minimizeButton.height() - fit(8));
}

void QmlEditorPrivate::saved(const QString& text)
{
	QQmlComponent component(rootContext->engine()); //TODO: Drop into another item?
	int index = itemList->indexOf(lastSelectedItem);
	if (index < 0) return;
	auto url = urlList->at(index);
	component.setData(QByteArray().insert(0,text), QUrl());

	QQmlIncubator incubator;
	component.create(incubator, rootContext);
	while (incubator.isLoading()) {
		QApplication::processEvents(QEventLoop::AllEvents, 50);
	}
	QQuickItem *qml = qobject_cast<QQuickItem*>(incubator.object());

	if (component.isError() || !qml) {qWarning() << component.errors(); return;}

	auto previousParent = lastSelectedItem->parentItem();
	auto previousPosition = lastSelectedItem->position();

	auto childs = GetAllChildren(lastSelectedItem);
	for (auto child : childs) {
		if (itemList->contains(child)) {
			int index = itemList->indexOf(child);
			itemList->removeAt(index);
			urlList->removeAt(index);
		}
	}
	lastSelectedItem->deleteLater();
	lastSelectedItem = qml;
	qml->setEnabled(false);

	int count = 1;
	QString componentName = qmlContext(qml)->nameForObject(qml);
	if (componentName.isEmpty()) componentName = "anonymous";
	for (int i=0; i<itemList->size();i++) {
		if (componentName == QString(rootContext->nameForObject((*itemList)[i])) ||
			componentName == QString("dpi") || componentName == QString("swipeView")) {
			componentName += QString::number(count);
			count++;
			i = 0;
		}
	}
	rootContext->setContextProperty(componentName, qml);
	qml->setParentItem(previousParent);
	qml->setPosition(previousPosition);
	qml->setClip(true); // Even if it's not true
	fit(qml, Fit::WidthHeight);
	*itemList << qml;
	*urlList << url;

	QMetaObject::invokeMethod(parent->parent(), "ShowSelectionTools", Qt::AutoConnection,
							  Q_ARG(QQuickItem*const,lastSelectedItem));
	parent->hide();

}

void QmlEditorPrivate::selectionChanged()
{
	int index = itemList->indexOf(lastSelectedItem);
	if (index >= 0) {
		QFile file(urlList->at(index).toLocalFile());
		Q_ASSERT(file.open(QIODevice::ReadOnly));
		QTextStream reader(&file);
		textDocument->setPlainText(reader.readAll());
		file.close();
	}
}

const QList<QQuickItem*> QmlEditorPrivate::GetAllChildren(QQuickItem* const item) const
{
	/* Return all child items of item including item itself */
	QList<QQuickItem*> childList;
	for (auto child : item->childItems())
		childList << GetAllChildren(child);
	childList << item;
	return childList;
}

void QmlEditorPrivate::show(const QString& url)
{
	QFileInfo info(url);
	QQmlProperty::write(rootItem, "toolboxMode", true, rootContext);
	QQmlProperty::write(rootItem, "folder", "file://" + info.dir().path(), rootContext);
	QMetaObject::invokeMethod(rootItem, "show", Qt::AutoConnection, Q_ARG(QVariant, url));

	((QWidget*)parent)->show();

	quickWidget.hide();
	minimizeButton.hide();

	QPropertyAnimation *animation = new QPropertyAnimation(parent, "showRatio");
	animation->setDuration(DURATION);
	animation->setStartValue(0.0);
	animation->setEndValue(1.0);
	animation->setEasingCurve(QEasingCurve::InExpo);
	animation->start();
	QObject::connect(animation, SIGNAL(finished()), &quickWidget, SLOT(show()));
	QObject::connect(animation, SIGNAL(finished()), &minimizeButton, SLOT(show()));
	QObject::connect(animation, SIGNAL(valueChanged(QVariant)), parent, SLOT(update()));
	QObject::connect(animation, SIGNAL(finished()), animation, SLOT(deleteLater()));
}

void QmlEditorPrivate::show()
{
	((QWidget*)parent)->show();

	QQmlProperty::write(rootItem, "toolboxMode", false, rootContext);
	QQmlProperty::write(rootItem, "rootFolder", "file://" + QCoreApplication::applicationDirPath(), rootContext);
	QQmlProperty::write(rootItem, "folder", "file://" + QCoreApplication::applicationDirPath(), rootContext);

	quickWidget.hide();
	minimizeButton.hide();

	QPropertyAnimation *animation = new QPropertyAnimation(parent, "showRatio");
	animation->setDuration(DURATION);
	animation->setStartValue(0.0);
	animation->setEndValue(1.0);
	animation->setEasingCurve(QEasingCurve::InExpo);
	animation->start();
	QObject::connect(animation, SIGNAL(finished()), &quickWidget, SLOT(show()));
	QObject::connect(animation, SIGNAL(finished()), &minimizeButton, SLOT(show()));
	QObject::connect(animation, SIGNAL(valueChanged(QVariant)), parent, SLOT(update()));
	QObject::connect(animation, SIGNAL(finished()), animation, SLOT(deleteLater()));
}

void QmlEditorPrivate::hide()
{
	auto snap = parent->grab();
	if (!snap.isNull()) {
		snapshot = snap;
		snapshot.setDevicePixelRatio(qApp->devicePixelRatio());
	}

	quickWidget.hide();
	minimizeButton.hide();

	QPropertyAnimation *animation = new QPropertyAnimation(parent, "showRatio");
	animation->setDuration(DURATION);
	animation->setStartValue(1.0);
	animation->setEndValue(0.0);
	animation->setEasingCurve(QEasingCurve::OutExpo);
	animation->start();
	QObject::connect(animation, SIGNAL(valueChanged(QVariant)), parent, SLOT(update()));
	QObject::connect(animation, &QPropertyAnimation::finished, [this]{((QWidget*)parent)->hide();});
	QObject::connect(animation, SIGNAL(finished()), animation, SLOT(deleteLater()));
}

QmlEditor::QmlEditor(QWidget *parent)
	: QWidget(parent)
	, m_d(new QmlEditorPrivate(this))
{
}

QmlEditor::~QmlEditor()
{
	delete m_d;
}

void QmlEditor::setItems(QList<QQuickItem*>* const itemList, QList<QUrl>* const urlList)
{
	m_d->itemList = itemList;
	m_d->urlList = urlList;
}

void QmlEditor::setRootContext(QQmlContext* const context)
{
	m_d->rootContext = context;
}

void QmlEditor::selectItem(QObject* const item)
{
	m_d->lastSelectedItem = (QQuickItem*)item;
	m_d->selectionChanged();
}

void QmlEditor::setShowCenter(const QPoint& p)
{
	m_d->showCenter =  p;
}

void QmlEditor::setRootFolder(const QString& folder)
{
	QQmlProperty::write(m_d->rootItem, "rootFolder", "file://" + folder, m_d->rootContext);
}

void QmlEditor::show(const QString& url)
{
	m_d->show(url);
}

void QmlEditor::clearCache()
{
	QMetaObject::invokeMethod(m_d->rootItem, "clearCache");
}

void QmlEditor::clearCacheFor(const QString& url, const bool isdir)
{
	QMetaObject::invokeMethod(m_d->rootItem, "clearCacheFor", Qt::AutoConnection, Q_ARG(QVariant,url),Q_ARG(QVariant,isdir));
}

void QmlEditor::updateCacheForRenamedEntry(const QString& from, const QString& to, const bool isdir)
{
	QMetaObject::invokeMethod(m_d->rootItem, "updateCacheForRenamedEntry", Qt::AutoConnection,
							  Q_ARG(QVariant,from), Q_ARG(QVariant,to), Q_ARG(QVariant,isdir));
}

void QmlEditor::setDeactive(const bool d)
{
	m_d->deactive = d;
}

void QmlEditor::show()
{
	if (!m_d->deactive) {
		m_d->show();
	}
}

void QmlEditor::hide()
{
	m_d->hide();
}

void QmlEditor::saved(const QString& text)
{
	m_d->saved(text);
}

void QmlEditor::resizeEvent(QResizeEvent* event)
{
	m_d->resize();
	QWidget::resizeEvent(event);
}

void QmlEditor::paintEvent(QPaintEvent* event)
{
	QWidget::paintEvent(event);
	if ((showRatio > 0 || showRatio < 1) && !m_d->snapshot.isNull()) {
		QPainter painter(this);
		QPainterPath path;
		int caliber = qMax(width(), height());
		path.addEllipse(m_d->showCenter, caliber * showRatio, caliber * showRatio);
		painter.setClipPath(path);
		painter.setPen(Qt::NoPen);
		painter.drawPixmap(rect(), m_d->snapshot);
	}
}

float QmlEditor::getShowRatio() const
{
	return showRatio;
}

void QmlEditor::setShowRatio(float value)
{
	showRatio = value;
}

QQmlEngine* CacheCleaner::engine;
