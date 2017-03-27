#ifndef SCENEMANAGER_H
#define SCENEMANAGER_H

#include <QObject>

class SceneManagerPrivate;
class QWidget;
class QMainWindow;

class SceneManager : public QObject
{
		Q_OBJECT
		Q_DISABLE_COPY(SceneManager)

	public:
		enum Direction { ToLeft, ToRight };
		explicit SceneManager(QObject *parent = 0);
		static SceneManager* instance();
		static void addScene(const QString& key, QWidget* scene);
		static void show(const QString& key, Direction direction);
		static void setCurrent(const QString& key);
		static void setMainWindow(QWidget* mainWindow);
		bool eventFilter(QObject *watched, QEvent *event);

	private:
		static SceneManagerPrivate* m_d;
};

#endif // SCENEMANAGER_H
