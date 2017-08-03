#ifndef PAGESWIDGET_H
#define PAGESWIDGET_H

#include <QWidget>

class PagesWidgetPrivate;
class QQuickItem;
class QQmlContext;
class BindingWidget;
class EventsWidget;

class PagesWidget : public QWidget
{
		Q_OBJECT
		Q_DISABLE_COPY(PagesWidget)

	public:
        explicit PagesWidget(QWidget *parent = 0);
        static PagesWidget* instance();
		~PagesWidget();

		static void setCurrentPage(int index);
		static void addPageWithoutSave(QString& name);
		static void changePageWithoutSave(const QString& from, QString& to);
		static void removePageWithoutSave(const QString& name);

	private:
		static PagesWidgetPrivate* m_d;
};

#endif // PAGESWIDGET_H