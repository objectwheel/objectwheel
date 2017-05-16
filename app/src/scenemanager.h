#ifndef SCENEMANAGER_H
#define SCENEMANAGER_H

#include <QObject>

class SceneManagerPrivate;
class QWidget;
class QMainWindow;
class ListWidget;

class SceneManager : public QObject
{
		Q_OBJECT
		Q_DISABLE_COPY(SceneManager)

	public:
		enum Direction { ToLeft, ToRight };
		explicit SceneManager(QObject *parent = 0);
        ~SceneManager();
		static SceneManager* instance();
		static QStringList scenes();
		static QString cuurrentScene();
		static QWidget* scene(const QString& key);
		static void addScene(const QString& key, QWidget* scene);
        static void removeScene(const QString& key);
        static void show(const QString& key, Direction direction);
        static void setCurrent(const QString& key, const bool animated = true);
		static void setMainWindow(QWidget* mainWindow);
		static void setSceneListWidget(ListWidget* listWidget);
		bool eventFilter(QObject *watched, QEvent *event);

	signals:
		void currentSceneChanged(const QString& key);

	private:
		static SceneManagerPrivate* m_d;
};

#endif // SCENEMANAGER_H
