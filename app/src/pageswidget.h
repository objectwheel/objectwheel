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
		Q_DISABLE_COPY(PagesWidget)

	public:
		explicit PagesWidget(QWidget *parent = 0);
		void setSwipeItem(QQuickItem* swipeItem);
		void setRootContext(QQmlContext* context);
		void setItemList(QList<QQuickItem*>* items);
		void setUrlList(QList<QUrl>* items);
		void setBindingWidget(BindingWidget* bindingWidget);
		~PagesWidget();

		static void setCurrentPage(int index);
		static void addPageWithoutSave(QString& name);
		static void changePageWithoutSave(const QString& from, QString& to);

	private:
		static PagesWidgetPrivate* m_d;
};

#endif // PAGESWIDGET_H
