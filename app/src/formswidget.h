#ifndef FORMSWIDGET_H
#define FORMSWIDGET_H

#include <QWidget>

class FormsWidgetPrivate;
class QQuickItem;
class QQmlContext;
class BindingWidget;
class EventsWidget;

class FormsWidget : public QWidget
{
		Q_OBJECT
        Q_DISABLE_COPY(FormsWidget)

	public:
        explicit FormsWidget(QWidget *parent = 0);
        static FormsWidget* instance();
        ~FormsWidget();

        static void setCurrentForm(int index);
        static void addFormWithoutSave(QString& name);
        static void changeFormWithoutSave(const QString& from, QString& to);
        static void removeFormWithoutSave(const QString& name);

	private:
        static FormsWidgetPrivate* m_d;
};

#endif // FORMSWIDGET_H
