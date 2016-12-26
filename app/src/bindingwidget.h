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
		explicit BindingWidget(QWidget *parent = 0);
		~BindingWidget();

		const QList<QQuickItem*>* items() const;
		void setItems(const QList<QQuickItem*>* const items);

	public slots:
		void clearList();
		void refreshList(QObject* const);
		void showBar();

	protected slots:
		void processBindings();

	private:
		BindingWidgetPrivate* m_d;
};

#endif // BINDINGWIDGET_H
