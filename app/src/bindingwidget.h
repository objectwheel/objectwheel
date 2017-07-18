#ifndef BINDINGWIDGET_H
#define BINDINGWIDGET_H

#include <QWidget>
#include <savemanager.h>

class BindingWidgetPrivate;
class Page;
class Control;

class BindingWidget : public QWidget
{
		Q_OBJECT

		friend class BindingWidgetPrivate;

	public:
		explicit BindingWidget(QWidget *parent = 0);
        static BindingWidget* instance();
		~BindingWidget();

        static void addBindingWithoutSave(const SaveManager::BindingInf& inf);

	public slots:
        void detachBindingsFor(Control*);
		void clearAllBindings();
		bool hasPopupOpen();

	protected slots:
        void handleSelectionChange();
        void clearList();

	signals:
		void popupShowed();
		void popupHid();

	private:
        static BindingWidgetPrivate* m_d;
};

#endif // BINDINGWIDGET_H
