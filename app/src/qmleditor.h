#ifndef QMLEDITOR_H
#define QMLEDITOR_H

#include <QWidget>
#include <QQmlEngine>

class QmlEditorPrivate;
class QQuickItem;
class QQmlContext;

class QmlEditor : public QWidget
{
		Q_OBJECT
		Q_PROPERTY(float showRatio READ getShowRatio WRITE setShowRatio)

	public:
		explicit QmlEditor(QWidget *parent = 0);
		void setItems(QList<QQuickItem*>* const itemList, QList<QUrl>* const urlList);
		void setRootContext(QQmlContext* const context);
		~QmlEditor();

		float getShowRatio() const;
		void setShowRatio(float value);

	public slots:
		void selectItem(QObject* const item);
		void setShowCenter(const QPoint& p);
		void setRootFolder(const QString& folder);
		void show(const QString& url);
		void clearCache();
		void clearCacheFor(const QString& url, const bool isdir);
		void updateCacheForRenamedEntry(const QString& from, const QString& to, const bool isdir);
		void show();
		void hide();

	private slots:
		void saved(const QString& text);

	protected:
		void resizeEvent(QResizeEvent *event) override;
		void paintEvent(QPaintEvent *event) override;

	private:
		QmlEditorPrivate* m_d;
		float showRatio;
};

class CacheCleaner : public QObject
{
		Q_OBJECT
	private:
		static QQmlEngine* engine;

	public:
		explicit CacheCleaner (QObject* parent = 0) : QObject(parent) {}
		Q_INVOKABLE inline void clear() { engine->clearComponentCache(); }
		inline static void setEngine(QQmlEngine* e) { engine = e; }
		inline static void registerQmlType() { qmlRegisterType<CacheCleaner>("com.objectwheel.editor",1,0,"CacheCleaner"); }
};

#endif // QMLEDITOR_H
