#ifndef QMLEDITOR_H
#define QMLEDITOR_H

#include <QWidget>
#include <QQmlEngine>
#include <QPointer>

class QmlEditorPrivate;
class QQuickItem;
class QQmlContext;
class BindingWidget;

class QmlEditor : public QWidget
{
		Q_OBJECT
		Q_PROPERTY(float showRatio READ getShowRatio WRITE setShowRatio)

	public:
		explicit QmlEditor(QWidget *parent = 0);
		void setItems(QList<QQuickItem*>* const itemList, QList<QUrl>* const urlList);
		void setRootContext(QQmlContext* const context);
		void setBindingWidget(BindingWidget* bindngWidget);
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
		void setDeactive(const bool);
		void show();
		void hide();

	private slots:
        void saved(const QString& qmlPath);

	protected:
		void resizeEvent(QResizeEvent *event) override;
		void paintEvent(QPaintEvent *event) override;

	private:
		QmlEditorPrivate* m_d;
		float showRatio;
};

class ComponentManager : public QObject
{
		Q_OBJECT
	private:
		static QQmlEngine* engine;
		static QString lastError;
		static QPointer<QQuickItem> lastItem;
		static QQuickItem* parentItem;

	public:
		explicit ComponentManager (QObject* parent = 0) : QObject(parent) {}
		static void setParentItem(QQuickItem* i);
		static void registerQmlType();
        Q_INVOKABLE static void clear();
        Q_INVOKABLE static QString error();
        Q_INVOKABLE static QQuickItem* build(const QString& url);
};

#endif // QMLEDITOR_H
