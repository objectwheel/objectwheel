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

SceneManagerPrivate* SceneManager::m_d = nullptr;

SceneManager::SceneManager(QObject *parent)
	: QObject(parent)
{
	if (m_d) return;
    m_d = new SceneManagerPrivate(this);
}

SceneManager::~SceneManager()
{
    delete m_d;
}

SceneManager* SceneManager::instance()
{
	return m_d->parent;
}

QStringList SceneManager::scenes()
{
	return m_d->sceneMap.keys();
}

QString SceneManager::cuurrentScene()
{
	return m_d->currentKey;
}

QWidget* SceneManager::scene(const QString& key)
{
	return m_d->sceneMap[key];
}

void SceneManager::addScene(const QString& key, QWidget* scene)
{
	m_d->sceneMap.insert(key, scene);
    scene->hide();
}

void SceneManager::removeScene(const QString& key)
{
    m_d->sceneMap.remove(key);
}

void SceneManager::show(const QString& key, SceneManager::Direction direction)
{
    if (!m_d->sceneMap.contains(key)) return;
    if (!m_d->sceneMap.contains(m_d->currentKey)) {
        setCurrent(key, false);
        return;
    }
    auto cs = m_d->sceneMap[m_d->currentKey];
    auto ns = m_d->sceneMap[key];
    if (cs == ns) return;

    auto leftRect = m_d->mainWindow->rect().adjusted(-m_d->mainWindow->width(), 0, -m_d->mainWindow->width(), 0);
    auto rightRect = m_d->mainWindow->rect().adjusted(m_d->mainWindow->width(), 0, m_d->mainWindow->width(), 0);

    disconnect(m_d->connection);
    m_d->csShifterAmination.stop();
    m_d->nsShifterAmination.stop();
    m_d->parallelAnimationGroup.stop();

    m_d->csShifterAmination.setTargetObject(cs);
    m_d->csShifterAmination.setPropertyName("geometry");
    m_d->csShifterAmination.setDuration(DURATION);
    m_d->csShifterAmination.setEasingCurve(QEasingCurve::OutExpo);
    m_d->csShifterAmination.setStartValue(m_d->mainWindow->rect());
    m_d->csShifterAmination.setEndValue(direction == ToLeft ? leftRect : rightRect);

    m_d->nsShifterAmination.setTargetObject(ns);
    m_d->nsShifterAmination.setPropertyName("geometry");
    m_d->nsShifterAmination.setDuration(DURATION);
    m_d->nsShifterAmination.setEasingCurve(QEasingCurve::OutExpo);
    m_d->nsShifterAmination.setEndValue(m_d->mainWindow->rect());
    m_d->nsShifterAmination.setStartValue(direction == ToLeft ? rightRect : leftRect);

    m_d->currentKey = key;
    m_d->parallelAnimationGroup.start();
    m_d->connection = QObject::connect(&m_d->parallelAnimationGroup, &QParallelAnimationGroup::finished, [=] {
        for (int i = m_d->sceneListWidget->count(); i--;) {
            if (m_d->sceneListWidget->GetUrls(m_d->sceneListWidget->item(i))[0].toString() == key) {
                m_d->sceneListWidget->setCurrentRow(i);
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
    if (!m_d->sceneMap.contains(key)) return;
    if (m_d->sceneMap.contains(m_d->currentKey)) m_d->sceneMap[m_d->currentKey]->hide();
    m_d->currentKey = key;
    m_d->sceneMap[key]->setGeometry(m_d->mainWindow->rect());
    m_d->sceneMap[key]->show();
    m_d->sceneMap[key]->raise();
    if(animated) {
        m_d->sceneMap[key]->setGraphicsEffect(m_d->opacityEffect);
        m_d->opacityEffect->setOpacity(0);
        m_d->showerTimer.start();
    }
    emit instance()->currentSceneChanged(key);
}

void SceneManager::setMainWindow(QWidget* mainWindow)
{
	if (m_d->mainWindow) m_d->mainWindow->removeEventFilter(m_d->parent);
	mainWindow->installEventFilter(m_d->parent);
	m_d->mainWindow = mainWindow;
	if (m_d->sceneMap.contains(m_d->currentKey)) m_d->sceneMap[m_d->currentKey]->setGeometry(m_d->mainWindow->rect());
}

void SceneManager::setSceneListWidget(ListWidget* listWidget)
{
	disconnect(m_d->connection2);
	m_d->sceneListWidget = listWidget;
	m_d->connection2 = connect(listWidget,
	(void (ListWidget::*)(QListWidgetItem*, QListWidgetItem*))&ListWidget::currentItemChanged, [](QListWidgetItem*c, QListWidgetItem*) {
		auto sceneName = m_d->sceneListWidget->GetUrls(c)[0].toString();
		QTimer::singleShot(300, [=] { SceneManager::show(sceneName, SceneManager::ToLeft); });
	});
}

bool SceneManager::eventFilter(QObject* watched, QEvent* event)
{
	if (event->type() == QEvent::Resize && watched == m_d->mainWindow) {
		if (m_d->sceneMap.contains(m_d->currentKey)) {
			m_d->sceneMap[m_d->currentKey]->setGeometry(m_d->mainWindow->rect());
		}
	}
	return false;
}
