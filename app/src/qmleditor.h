#ifndef QMLEDITOR_H
#define QMLEDITOR_H

#include <QWidget>

class QmlEditorPrivate;
class QQuickItem;
class QQmlContext;

class QmlEditor : public QWidget
{
		Q_OBJECT
		Q_PROPERTY(float showRatio READ getShowRatio WRITE setShowRatio)

	public:
		explicit QmlEditor(QWidget *parent = 0);
		~QmlEditor();
		void setItems(QList<QQuickItem*>* const itemList, QList<QUrl>* const urlList);
		void setRootContext(QQmlContext* const context);

		float getShowRatio() const;
		void setShowRatio(float value);

	public slots:
		void selectItem(QObject* const item);
		void setShowCenter(const QPoint& p);
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

#endif // QMLEDITOR_H
