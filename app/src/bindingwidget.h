#ifndef BINDINGWIDGET_H
#define BINDINGWIDGET_H

#include <QWidget>

class BindingWidgetPrivate;
class QQuickItem;

class BindingWidget : public QWidget
{
		Q_OBJECT

		friend class BindingWidgetPrivate;

	public:
		typedef QList<QQuickItem*> QQuickItemList;
		explicit BindingWidget(QWidget *parent = 0);
		~BindingWidget();

		const QQuickItemList* items() const;
		void setItems(const QQuickItemList* const items);

	public slots:
		void clearList();
		void refreshList(QObject*const);
		void showBar();

	private:
		BindingWidgetPrivate* m_d;
};

#endif // BINDINGWIDGET_H
