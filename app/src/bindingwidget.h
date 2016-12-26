#ifndef BINDINGWIDGET_H
#define BINDINGWIDGET_H

#include <QWidget>

class BindingWidgetPrivate;
class QQuickItem;
class QQmlContext;

class BindingWidget : public QWidget
{
		Q_OBJECT

		friend class BindingWidgetPrivate;

	public:
		explicit BindingWidget(QWidget *parent = 0);
		~BindingWidget();

		const QList<QQuickItem*>* itemSource() const;
		void setItemSource(const QList<QQuickItem*>* const ItemSource);

		const QQmlContext* rootContext() const;
		void setRootContext(QQmlContext* const rootContext);

	public slots:
		void clearList();
		void selectItem(QObject* const);
		void detachBindingsFor(QObject* const);
		void clearAllBindings();
		void showBar();

	protected slots:
		void processBindings();

	private:
		BindingWidgetPrivate* m_d;
};

#endif // BINDINGWIDGET_H
