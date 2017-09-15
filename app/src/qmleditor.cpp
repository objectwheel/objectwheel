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
#include <savemanager.h>
#include <delayer.h>

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
		QQmlContext* dashboardRootContext;
		QPixmap snapshot;
		QPoint showCenter;
		bool deactive;

		QmlEditorPrivate(QmlEditor* p);
        void rebuildEditor();
        void resize();
        void saved(const QString& qmlPath);
		const QList<QQuickItem*> GetAllChildren(QQuickItem* const item) const;
		void show(const QString& url);
        void showTextOnly(const QString& text);
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
	rootContext = quickWidget.rootContext();

	rootContext->setContextProperty("dpi", Fit::ratio());
	ComponentManager::registerQmlType();

    minimizeButton.setParent(parent);
    minimizeButton.setIconButton(true);
    minimizeButton.setIcon(QIcon(":/resources/images/back.png"));
    minimizeButton.resize(fit(35), fit(35));
    minimizeButton.setCursor(Qt::PointingHandCursor);
    QObject::connect(&minimizeButton, SIGNAL(clicked(bool)), parent, SLOT(hide()));

    rebuildEditor();

    QTimer::singleShot(500, [this] {
        auto snap = parent->grab();
        if (!snap.isNull()) {
            snapshot = snap;
            snapshot.setDevicePixelRatio(qApp->devicePixelRatio());
        }
    });
}

void QmlEditorPrivate::rebuildEditor()
{
    quickWidget.setSource(QUrl("qrc:/resources/qmls/empty-item.qml"));
    quickWidget.setResizeMode(QQuickWidget::SizeRootObjectToView);
    quickWidget.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    Delayer::delay(100);

    quickWidget.setSource(QUrl("qrc:/resources/qmls/qml-editor.qml"));
    quickWidget.setResizeMode(QQuickWidget::SizeRootObjectToView);
    quickWidget.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    rootItem = quickWidget.rootObject();
    QObject::connect(rootItem, SIGNAL(saved(QString)), parent, SLOT(saved(const QString&)));
    QObject::connect(rootItem, SIGNAL(savedTextOnly(QString)), parent, SIGNAL(savedTextOnly(const QString&)));
    QObject::connect(rootItem, SIGNAL(currentSaved()), parent, SLOT(hide()));

    QObject *textEdit = rootItem->findChild<QObject*>(QStringLiteral("editor"));
    QQuickTextDocument *quickTextDocument = textEdit->property("textDocument").value<QQuickTextDocument*>();
    textDocument = quickTextDocument->textDocument();
    QTextOption textOptions = textDocument->defaultTextOption();

    QFont font;
    font.setFamily("Liberation Mono");
    font.setStyleHint(QFont::Monospace);
    font.setPixelSize(fit(12));
    textEdit->setProperty("font", font);

    const int tabStop = 4;  // 4 characters
    QFontMetrics metrics(font);
    textOptions.setTabStop(tabStop * metrics.width(' '));
    textDocument->setDefaultTextOption(textOptions);

    auto item = qobject_cast<QQuickItem*>(QQmlProperty::read(rootItem,"view", rootContext).value<QObject*>());
    if (!item) qFatal("QmlEditor : Error occurred");
    ComponentManager::setParentItem(item);
    ComponentManager::rebuildEngine();
}

void QmlEditorPrivate::resize()
{
	minimizeButton.move(fit(8), parent->height() - minimizeButton.height() - fit(8));
}

void QmlEditorPrivate::saved(const QString& qmlPath)
{
    dashboardRootContext->engine()->clearComponentCache();
    QQmlComponent component(dashboardRootContext->engine(), qmlPath); //TODO: Drop into another item?
	int index = itemList->indexOf(lastSelectedItem);
	if (index < 0) return;
	itemList->removeAt(index);
	urlList->removeAt(index);

	QQmlIncubator incubator;
	component.create(incubator, dashboardRootContext);
    Delayer::delay(&incubator, &QQmlIncubator::isLoading);
	QQuickItem *qml = qobject_cast<QQuickItem*>(incubator.object());

	if (component.isError() || !qml) {qWarning() << component.errorString(); return;}

	int count = 1;
	QString componentName = qmlContext(qml)->nameForObject(qml);
	if (componentName.isEmpty()) componentName = "anonymous";
	for (int i=0; i<itemList->size();i++) {
		if (componentName == QString(dashboardRootContext->nameForObject((*itemList)[i])) ||
			componentName == QString("dpi") || componentName == QString("swipeView")) {
			componentName += QString::number(count);
			count++;
			i = -1;
		}
	}
	dashboardRootContext->setContextProperty(dashboardRootContext->nameForObject(qml), 0);
	dashboardRootContext->setContextProperty(componentName, qml); //WARNING:
	qml->setParentItem(lastSelectedItem->parentItem());
//	SaveManager::addParentalRelationship(dashboardRootContext->nameForObject(qml),
//										 dashboardRootContext->nameForObject(lastSelectedItem->parentItem()));
	qml->setPosition(lastSelectedItem->position());
	qml->setEnabled(false);
	fit(qml, Fit::WidthHeight);
	*itemList << qml;
    *urlList << qmlPath;

	auto childs = GetAllChildren(lastSelectedItem);
	for (auto child : childs) {
		if (itemList->contains(child) && lastSelectedItem!=child) {
			auto prevPos = child->position();
			child->setParentItem(qml);
//			SaveManager::addParentalRelationship(dashboardRootContext->nameForObject(child),
//												 dashboardRootContext->nameForObject(qml));
			child->setPosition(prevPos);
		}
	}
	if (dashboardRootContext->nameForObject(lastSelectedItem) != componentName) {
		dashboardRootContext->setContextProperty(dashboardRootContext->nameForObject(lastSelectedItem), 0);
	}
	lastSelectedItem->deleteLater();
	lastSelectedItem = qml;

    auto items = GetAllChildren((QQuickItem*)lastSelectedItem);
    for (auto item : items) {
        auto ctxId = dashboardRootContext->nameForObject((QObject*)item);
        if (item == (QQuickItem*)lastSelectedItem) {
//            auto prevParent = SaveManager::parentalRelationship(ctxId);
//            SaveManager::removeParentalRelationship(ctxId);
//            SaveManager::addParentalRelationship(componentName, prevParent);
//            SaveManager::changeSave(ctxId, componentName); //BUG: KLASÖR İSMİ DEĞİŞMİYOR
            //control.setId(ctxId, componentName); //TODO:
            if (itemList->indexOf(item) >= 0) {            //BUG: AYNI TARGETITEM'E IKI TANE AYNI EVENTTEN EKLEYINCE BOZULUYOR
                auto oldUrl = (*urlList)[itemList->indexOf(item)].toLocalFile();
                auto newUrl = dname(dname(oldUrl)) + separator() + componentName + separator() + "main.qml";
                (*urlList)[itemList->indexOf(item)] = QUrl::fromLocalFile(newUrl);
            }
        } else if (itemList->contains(item)) {
//            SaveManager::addParentalRelationship(ctxId, componentName);
        }
    }

	QMetaObject::invokeMethod(parent->parent(), "ShowSelectionTools", Qt::AutoConnection,
							  Q_ARG(QQuickItem*const,lastSelectedItem));
	parent->hide();
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
    QQmlProperty::write(rootItem, "visible", true, rootContext);
    QMetaObject::invokeMethod(rootItem, "setToolboxMode", Qt::AutoConnection, Q_ARG(QVariant, true));
    QMetaObject::invokeMethod(rootItem, "setFolder", Qt::AutoConnection, Q_ARG(QVariant, QUrl::fromLocalFile(info.dir().path())));
    QMetaObject::invokeMethod(rootItem, "setTextOnly", Qt::AutoConnection, Q_ARG(QVariant, false));

    QTimer::singleShot(DURATION,[=] { //FIXME
        QMetaObject::invokeMethod(rootItem, "show", Qt::AutoConnection, Q_ARG(QVariant, url));
    });

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

void QmlEditorPrivate::showTextOnly(const QString& text)
{
    QQmlProperty::write(rootItem, "visible", true, rootContext);
    textDocument->setPlainText(text);
    QMetaObject::invokeMethod(rootItem, "setTextOnly", Qt::AutoConnection, Q_ARG(QVariant, true));

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
//    QString controlPath = SaveManager::saveDirectory(dashboardRootContext->nameForObject(lastSelectedItem));
	QQmlProperty::write(rootItem, "visible", true, rootContext);
    QMetaObject::invokeMethod(rootItem, "setToolboxMode", Qt::AutoConnection, Q_ARG(QVariant, false));
//    QMetaObject::invokeMethod(rootItem, "setRootFolder", Qt::AutoConnection, Q_ARG(QVariant, QUrl::fromLocalFile(controlPath)));
//    QMetaObject::invokeMethod(rootItem, "setFolder", Qt::AutoConnection, Q_ARG(QVariant, QUrl::fromLocalFile(controlPath)));
    QMetaObject::invokeMethod(rootItem, "setTextOnly", Qt::AutoConnection, Q_ARG(QVariant, false));

//    QTimer::singleShot(DURATION,[=] { //FIXME
//        QMetaObject::invokeMethod(rootItem, "show", Qt::AutoConnection, Q_ARG(QVariant, controlPath + separator() + "main.qml"));
//    });

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

void QmlEditorPrivate::hide()
{
	QQmlProperty::write(rootItem, "visible", false, rootContext);
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

QmlEditorPrivate* QmlEditor::_d = nullptr;
float QmlEditor::showRatio;

QmlEditor::QmlEditor(QWidget *parent)
	: QWidget(parent)
{
    if (_d) return;
    _d = new QmlEditorPrivate(this);
}

QmlEditor* QmlEditor::instance()
{
    return _d->parent;
}

QmlEditor::~QmlEditor()
{
    delete _d;
}

void QmlEditor::setItems(QList<QQuickItem*>* const itemList, QList<QUrl>* const urlList)
{
    _d->itemList = itemList;
    _d->urlList = urlList;
}

void QmlEditor::setRootContext(QQmlContext* const context)
{
    _d->dashboardRootContext = context;
}

void QmlEditor::selectItem(QObject* const item)
{
    _d->lastSelectedItem = (QQuickItem*)item;
}

void QmlEditor::setShowCenter(const QPoint& p)
{
    _d->showCenter =  p;
}

void QmlEditor::setRootFolder(const QString& folder)
{
    QMetaObject::invokeMethod(_d->rootItem, "setRootFolder", Qt::AutoConnection, Q_ARG(QVariant, QUrl::fromLocalFile(folder)));
}

void QmlEditor::show(const QString& url)
{
    _d->show(url);
}

void QmlEditor::clearEditor()
{
    _d->rebuildEditor();
}

void QmlEditor::clearCache()
{
    QMetaObject::invokeMethod(_d->rootItem, "clearCache");
}

void QmlEditor::clearCacheFor(const QString& url, const bool isdir)
{
    QMetaObject::invokeMethod(_d->rootItem, "clearCacheFor", Qt::AutoConnection, Q_ARG(QVariant,url),Q_ARG(QVariant,isdir));
}

void QmlEditor::updateCacheForRenamedEntry(const QString& from, const QString& to, const bool isdir)
{
    QMetaObject::invokeMethod(_d->rootItem, "updateCacheForRenamedEntry", Qt::AutoConnection,
							  Q_ARG(QVariant,from), Q_ARG(QVariant,to), Q_ARG(QVariant,isdir));
}

void QmlEditor::setDeactive(const bool d)
{
    _d->deactive = d;
}

void QmlEditor::show()
{
    if (!_d->deactive) {
        _d->show();
	}
}

void QmlEditor::hide()
{
    _d->hide();
}

void QmlEditor::showTextOnly(const QString& text)
{
    _d->showTextOnly(text);
}

void QmlEditor::saved(const QString& qmlPath)
{
    _d->saved(qmlPath);
}

void QmlEditor::resizeEvent(QResizeEvent* event)
{
    _d->resize();
	QWidget::resizeEvent(event);
}

void QmlEditor::paintEvent(QPaintEvent* event)
{
	QWidget::paintEvent(event);
    if ((showRatio > 0 || showRatio < 1) && !_d->snapshot.isNull()) {
		QPainter painter(this);
		QPainterPath path;
		int caliber = qMax(width(), height());
        path.addEllipse(_d->showCenter, caliber * showRatio, caliber * showRatio);
		painter.setClipPath(path);
		painter.setPen(Qt::NoPen);
        painter.drawPixmap(rect(), _d->snapshot);
	}
}

float QmlEditor::getShowRatio()
{
	return showRatio;
}

void QmlEditor::setShowRatio(float value)
{
	showRatio = value;
}

/*! [*] Component Manager [*] !*/

QQmlEngine* ComponentManager::engine;
QStringList ComponentManager::lastErrors;
QPointer<QQuickItem> ComponentManager::lastItem = nullptr;
QQuickItem* ComponentManager::parentItem;

void ComponentManager::setParentItem(QQuickItem* i)
{
	parentItem = i;
}

void ComponentManager::registerQmlType()
{
	qmlRegisterType<ComponentManager>("com.objectwheel.editor",1,0,"ComponentManager");
	engine = new QQmlEngine;
    engine->rootContext()->setContextProperty("dpi", Fit::ratio());
}

void ComponentManager::rebuildEngine()
{
    clear();
    lastItem = nullptr;
    lastErrors.clear();
    engine->deleteLater();
    engine = new QQmlEngine;
    engine->rootContext()->setContextProperty("dpi", Fit::ratio());
}

void ComponentManager::clear()
{
    if (lastItem) {
        lastItem->deleteLater();
    }
    engine->clearComponentCache();
}

QStringList ComponentManager::errors()
{
    return lastErrors;
}

QQuickItem* ComponentManager::build(const QString& url)
{
	QQmlComponent component(engine);
    component.loadUrl(QUrl::fromUserInput(url));

    QQmlIncubator incubator;
	component.create(incubator);
    Delayer::delay(&incubator, &QQmlIncubator::isLoading);
	lastItem = qobject_cast<QQuickItem*>(incubator.object());

	if (component.isError() || !lastItem) {
        QStringList errors;
        for (auto e : component.errors()) {
            QJsonObject error;
            error["description"] = e.description();
            error["line"] = e.line();
            error["column"] = e.column();
            error["path"] = e.url().toLocalFile();
            errors.append(QJsonDocument(error).toJson());
        }
        lastErrors = errors;
		return nullptr;
	}

	lastItem->setParentItem(parentItem);
	lastItem->setWidth(fit(lastItem->width()));
	lastItem->setHeight(fit(lastItem->height()));
	QVariant variant;
	variant.setValue<QQuickItem*>(parentItem);
	QQmlProperty::write(lastItem, "anchors.centerIn", variant);
	return lastItem;
}
