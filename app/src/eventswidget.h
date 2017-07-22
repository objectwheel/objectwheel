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
        ~EventsWidget();

        static void addEventWithoutSave(const SaveManager::EventInf& inf);

	public slots:
        void detachEventsFor(Control*);
        void clearAllEvents();
		bool hasPopupOpen();

    protected slots:
        void clearList();
        void handleSelectionChange();

	signals:
		void popupShowed();
		void popupHid();

	private:
        static EventsWidgetPrivate* m_d;
};

#endif // EVENTSWIDGET_H
