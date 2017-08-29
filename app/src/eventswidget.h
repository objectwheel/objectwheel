#ifndef EVENTSWIDGET_H
#define EVENTSWIDGET_H

#include <QWidget>
#include <savemanager.h>

class EventsWidgetPrivate;
class Control;

class EventsWidget : public QWidget
{
		Q_OBJECT
        friend class EventsWidgetPrivate;

	public:
        explicit EventsWidget(QWidget *parent = 0);
        static EventsWidget* instance();

    public slots:
		bool hasPopupOpen();

    protected slots:
        void clearList();
        void handleSelectionChange();
        void handleMainControlChange(Control* control);
        void handleDesignerModeChange();

    signals:
		void popupShowed();
		void popupHid();

	private:
        static EventsWidgetPrivate* _d;
        static QPointer<Control> _mainControl;
};

#endif // EVENTSWIDGET_H
