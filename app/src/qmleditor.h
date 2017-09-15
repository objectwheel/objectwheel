#ifndef QMLEDITOR_H
#define QMLEDITOR_H

#include <QWidget>
#include <QQmlEngine>
#include <QPointer>
#include <QUrl>
#include <QJsonObject>
#include <QJsonDocument>

class QmlEditorPrivate;
class QQuickItem;
class QQmlContext;

class QmlEditor : public QWidget
{
		Q_OBJECT
        Q_DISABLE_COPY(QmlEditor)
		Q_PROPERTY(float showRatio READ getShowRatio WRITE setShowRatio)

	public:
		explicit QmlEditor(QWidget *parent = 0);
        static QmlEditor* instance();
        static void setItems(QList<QQuickItem*>* const itemList, QList<QUrl>* const urlList);
        static void setRootContext(QQmlContext* const context);
        ~QmlEditor();

        static float getShowRatio();
        static void setShowRatio(float value);

	public slots:
        static void selectItem(QObject* const item);
        static void setShowCenter(const QPoint& p);
        static void setRootFolder(const QString& folder);
        static void show(const QString& url);
        static void clearEditor();
        static void clearCache();
        static void clearCacheFor(const QString& url, const bool isdir);
        static void updateCacheForRenamedEntry(const QString& from, const QString& to, const bool isdir);
        static void setDeactive(const bool);
        static void show();
        static void hide();
        static void showTextOnly(const QString& text);

	private slots:
        void saved(const QString& qmlPath);

	protected:
		void resizeEvent(QResizeEvent *event) override;
		void paintEvent(QPaintEvent *event) override;

    signals:
        void savedTextOnly(const QString& text);

	private:
        static QmlEditorPrivate* _d;
        static float showRatio;
};

class ComponentManager : public QObject
{
		Q_OBJECT
	private:
		static QQmlEngine* engine;
        static QStringList lastErrors;
		static QPointer<QQuickItem> lastItem;
		static QQuickItem* parentItem;

	public:
		explicit ComponentManager (QObject* parent = 0) : QObject(parent) {}
		static void setParentItem(QQuickItem* i);
		static void registerQmlType();
        Q_INVOKABLE static void rebuildEngine();
        Q_INVOKABLE static void clear();
        Q_INVOKABLE static QStringList errors();
        Q_INVOKABLE static QQuickItem* build(const QString &url);
        Q_INVOKABLE static inline QUrl urlOfPath(const QString& path) { return QUrl::fromUserInput(path); }
        Q_INVOKABLE static inline QString pathOfUrl(const QUrl& url) { return url.toLocalFile(); }
};

#endif // QMLEDITOR_H
