#include <scenemanager.h>
#include <QWidget>
#include <QMap>
#include <QMainWindow>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include <QEvent>

#define DURATION 500

class SceneManagerPrivate
{
	public:
		SceneManagerPrivate(SceneManager* uparent);

	public:
		SceneManager* parent = nullptr;
		QMap<QString, QWidget*> sceneMap;
		QString currentKey;
		QWidget* mainWindow = nullptr;
		QParallelAnimationGroup parallelAnimationGroup;
		QPropertyAnimation nsShifterAmination;
		QPropertyAnimation csShifterAmination;
		QMetaObject::Connection connection;
};

SceneManagerPrivate::SceneManagerPrivate(SceneManager* uparent)
	: parent(uparent)
{
	parallelAnimationGroup.addAnimation(&csShifterAmination);
	parallelAnimationGroup.addAnimation(&nsShifterAmination);
}

SceneManagerPrivate* SceneManager::m_d = nullptr;

SceneManager::SceneManager(QObject *parent)
	: QObject(parent)
{
	if (m_d) return;
	m_d = new SceneManagerPrivate(this);
}

SceneManager* SceneManager::instance()
{
	return m_d->parent;
}

void SceneManager::addScene(const QString& key, QWidget* scene)
{
	m_d->sceneMap.insert(key, scene);
}

void SceneManager::show(const QString& key, SceneManager::Direction direction)
{
	if (!m_d->sceneMap.contains(key)) return;
	if (!m_d->sceneMap.contains(m_d->currentKey)) {
		setCurrent(key);
		return;
	}
	auto cs = m_d->sceneMap[m_d->currentKey];
	auto ns = m_d->sceneMap[key];

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
	m_d->connection = QObject::connect(&m_d->parallelAnimationGroup, &QParallelAnimationGroup::finished, [=] { cs->hide(); });
	ns->show();
	ns->raise();
}

void SceneManager::setCurrent(const QString& key)
{
	if (!m_d->sceneMap.contains(key)) return;
	if (m_d->sceneMap.contains(m_d->currentKey)) m_d->sceneMap[m_d->currentKey]->hide();
	m_d->currentKey = key;
	m_d->sceneMap[key]->setGeometry(m_d->mainWindow->rect());
	m_d->sceneMap[key]->show();
	m_d->sceneMap[key]->raise();
}

void SceneManager::setMainWindow(QWidget* mainWindow)
{
	if (m_d->mainWindow) m_d->mainWindow->removeEventFilter(m_d->parent);
	mainWindow->installEventFilter(m_d->parent);
	m_d->mainWindow = mainWindow;
	if (m_d->sceneMap.contains(m_d->currentKey)) m_d->sceneMap[m_d->currentKey]->setGeometry(m_d->mainWindow->rect());
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
