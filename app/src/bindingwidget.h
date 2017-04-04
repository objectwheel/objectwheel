#ifndef BINDINGWIDGET_H
#define BINDINGWIDGET_H

#include <QWidget>
#include <savemanager.h>

class BindingWidgetPrivate;
class QQuickItem;
class QQmlContext;

class BindingWidget : public QWidget
{
		Q_OBJECT

		friend class BindingWidgetPrivate;

	public:
		explicit BindingWidget(QWidget *parent = 0);
        static BindingWidget* instance();
		~BindingWidget();

		const QList<QQuickItem*>* itemSource() const;
		void setItemSource(const QList<QQuickItem*>* const ItemSource);

		const QQmlContext* rootContext() const;
		void setRootContext(QQmlContext* const rootContext);

        static void addBindingWithoutSave(const SaveManager::BindingInf& inf);

	public slots:
		void clearList();
		void selectItem(QObject* const);
		void detachBindingsFor(QObject* const);
		void clearAllBindings();
		void showBar();
		bool hasPopupOpen();

	protected slots:
		void processBindings();

	signals:
		void popupShowed();
		void popupHid();

	private:
        static BindingWidgetPrivate* m_d;
};

#endif // BINDINGWIDGET_H
