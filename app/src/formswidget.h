#ifndef FORMSWIDGET_H
#define FORMSWIDGET_H

#include <QWidget>

class FormsWidgetPrivate;
class QQuickItem;
class QQmlContext;

class FormsWidget : public QWidget
{
		Q_OBJECT
        Q_DISABLE_COPY(FormsWidget)

	public:
        explicit FormsWidget(QWidget *parent = 0);
        static FormsWidget* instance();
        ~FormsWidget();

        static void setCurrentForm(int index);

	private:
        static FormsWidgetPrivate* m_d;
};

#endif // FORMSWIDGET_H
