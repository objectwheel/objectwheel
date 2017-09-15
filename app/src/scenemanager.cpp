#include <scenemanager.h>
#include <QWidget>
#include <QMap>
#include <QMainWindow>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include <QEvent>
#include <listwidget.h>
#include <QTimer>
#include <QGraphicsOpacityEffect>

#define DURATION 500
#define SHOWER_TIMER_INTERVAL 300

class SceneManagerPrivate
{
	public:
		SceneManagerPrivate(SceneManager* uparent);

	public:
		SceneManager* parent = nullptr;
		QMap<QString, QWidget*> sceneMap;
		QString currentKey;
		QWidget* mainWindow = nullptr;
		ListWidget* sceneListWidget = nullptr;
		QParallelAnimationGroup parallelAnimationGroup;
		QPropertyAnimation nsShifterAmination;
		QPropertyAnimation csShifterAmination;
		QMetaObject::Connection connection;
		QMetaObject::Connection connection2;
        QTimer showerTimer;
        QGraphicsOpacityEffect* opacityEffect;
};

SceneManagerPrivate::SceneManagerPrivate(SceneManager* uparent)
	: parent(uparent)
    , opacityEffect(new QGraphicsOpacityEffect)
{
	parallelAnimationGroup.addAnimation(&csShifterAmination);
	parallelAnimationGroup.addAnimation(&nsShifterAmination);
    opacityEffect->setOpacity(1);
    showerTimer.setInterval(17); //For 60fps
    QObject::connect(&showerTimer, &QTimer::timeout, [=]{
        static qreal step = 1.0 / (SHOWER_TIMER_INTERVAL/17.0);
        if (opacityEffect->opacity() < 1) {
            opacityEffect->setOpacity(opacityEffect->opacity() + step);
        } else {
            opacityEffect->setOpacity(1);
            showerTimer.stop();
        }

        static bool test = false;
        if (test) {
            sceneMap[currentKey]->resize(sceneMap[currentKey]->width() - 1, sceneMap[currentKey]->height());
        } else {
            sceneMap[currentKey]->resize(sceneMap[currentKey]->width() + 1, sceneMap[currentKey]->height());
        }
        test = !test;
    });
}

SceneManagerPrivate* SceneManager::_d = nullptr;

SceneManager::SceneManager(QObject *parent)
	: QObject(parent)
{
	if (_d) return;
    _d = new SceneManagerPrivate(this);
}

SceneManager::~SceneManager()
{
    delete _d;
}

SceneManager* SceneManager::instance()
{
	return _d->parent;
}

QStringList SceneManager::scenes()
{
	return _d->sceneMap.keys();
}

QString SceneManager::cuurrentScene()
{
	return _d->currentKey;
}

QWidget* SceneManager::scene(const QString& key)
{
	return _d->sceneMap[key];
}

void SceneManager::addScene(const QString& key, QWidget* scene)
{
	_d->sceneMap.insert(key, scene);
    scene->hide();
}

void SceneManager::removeScene(const QString& key)
{
    _d->sceneMap.remove(key);
}

void SceneManager::show(const QString& key, SceneManager::Direction direction)
{
    if (!_d->sceneMap.contains(key)) return;
    if (!_d->sceneMap.contains(_d->currentKey)) {
        setCurrent(key, false);
        return;
    }
    auto cs = _d->sceneMap[_d->currentKey];
    auto ns = _d->sceneMap[key];
    if (cs == ns) return;

    auto leftRect = _d->mainWindow->rect().adjusted(-_d->mainWindow->width(), 0, -_d->mainWindow->width(), 0);
    auto rightRect = _d->mainWindow->rect().adjusted(_d->mainWindow->width(), 0, _d->mainWindow->width(), 0);

    disconnect(_d->connection);
    _d->csShifterAmination.stop();
    _d->nsShifterAmination.stop();
    _d->parallelAnimationGroup.stop();

    _d->csShifterAmination.setTargetObject(cs);
    _d->csShifterAmination.setPropertyName("geometry");
    _d->csShifterAmination.setDuration(DURATION);
    _d->csShifterAmination.setEasingCurve(QEasingCurve::OutExpo);
    _d->csShifterAmination.setStartValue(_d->mainWindow->rect());
    _d->csShifterAmination.setEndValue(direction == ToLeft ? leftRect : rightRect);

    _d->nsShifterAmination.setTargetObject(ns);
    _d->nsShifterAmination.setPropertyName("geometry");
    _d->nsShifterAmination.setDuration(DURATION);
    _d->nsShifterAmination.setEasingCurve(QEasingCurve::OutExpo);
    _d->nsShifterAmination.setEndValue(_d->mainWindow->rect());
    _d->nsShifterAmination.setStartValue(direction == ToLeft ? rightRect : leftRect);

    _d->currentKey = key;
    _d->parallelAnimationGroup.start();
    _d->connection = QObject::connect(&_d->parallelAnimationGroup, &QParallelAnimationGroup::finished, [=] {
        for (int i = _d->sceneListWidget->count(); i--;) {
            if (_d->sceneListWidget->GetUrls(_d->sceneListWidget->item(i))[0].toString() == key) {
                _d->sceneListWidget->setCurrentRow(i);
            }
        }
        cs->hide();
        emit instance()->currentSceneChanged(key);
    });
    ns->show();
    ns->raise();
}

void SceneManager::setCurrent(const QString& key, const bool animated)
{
    if (!_d->sceneMap.contains(key)) return;
    if (_d->sceneMap.contains(_d->currentKey)) _d->sceneMap[_d->currentKey]->hide();
    _d->currentKey = key;
    _d->sceneMap[key]->setGeometry(_d->mainWindow->rect());
    _d->sceneMap[key]->show();
    _d->sceneMap[key]->raise();
    if(animated) {
        _d->sceneMap[key]->setGraphicsEffect(_d->opacityEffect);
        _d->opacityEffect->setOpacity(0);
        _d->showerTimer.start();
    }
    emit instance()->currentSceneChanged(key);
}

void SceneManager::setMainWindow(QWidget* mainWindow)
{
	if (_d->mainWindow) _d->mainWindow->removeEventFilter(_d->parent);
	mainWindow->installEventFilter(_d->parent);
	_d->mainWindow = mainWindow;
	if (_d->sceneMap.contains(_d->currentKey)) _d->sceneMap[_d->currentKey]->setGeometry(_d->mainWindow->rect());
}

void SceneManager::setSceneListWidget(ListWidget* listWidget)
{
	disconnect(_d->connection2);
	_d->sceneListWidget = listWidget;
	_d->connection2 = connect(listWidget,
	(void (ListWidget::*)(QListWidgetItem*, QListWidgetItem*))&ListWidget::currentItemChanged, [](QListWidgetItem*c, QListWidgetItem*) {
		auto sceneName = _d->sceneListWidget->GetUrls(c)[0].toString();
		QTimer::singleShot(300, [=] { SceneManager::show(sceneName, SceneManager::ToLeft); });
	});
}

bool SceneManager::eventFilter(QObject* watched, QEvent* event)
{
	if (event->type() == QEvent::Resize && watched == _d->mainWindow) {
		if (_d->sceneMap.contains(_d->currentKey)) {
			_d->sceneMap[_d->currentKey]->setGeometry(_d->mainWindow->rect());
		}
	}
	return false;
}
