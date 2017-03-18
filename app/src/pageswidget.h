#ifndef PAGESWIDGET_H
#define PAGESWIDGET_H

#include <QWidget>

class PagesWidgetPrivate;
class QQuickItem;
class QQmlContext;
class BindingWidget;

class PagesWidget : public QWidget
{
		Q_OBJECT

	public:
		explicit PagesWidget(QWidget *parent = 0);
		void setSwipeItem(QQuickItem* swipeItem);
		void setRootContext(QQmlContext* context);
		void setItemList(QList<QQuickItem*>* items);
		void setUrlList(QList<QUrl>* items);
		void setBindingWidget(BindingWidget* bindingWidget);
		void setCurrentPage(int index);
		~PagesWidget();

	private:
		PagesWidgetPrivate* m_d;
};

#endif // PAGESWIDGET_H
