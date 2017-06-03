#ifndef EVENTSWIDGET_H
#define EVENTSWIDGET_H

#include <QWidget>
#include <savemanager.h>

class EventsWidgetPrivate;
class QQuickItem;
class QQmlContext;

class EventsWidget : public QWidget
{
		Q_OBJECT

        friend class EventsWidgetPrivate;

	public:
        explicit EventsWidget(QWidget *parent = 0);
        static EventsWidget* instance();
        ~EventsWidget();

		const QList<QQuickItem*>* itemSource() const;
		void setItemSource(const QList<QQuickItem*>* const ItemSource);

		const QQmlContext* rootContext() const;
		void setRootContext(QQmlContext* const rootContext);

        const QObject* rootObject() const;
        void setRootObject(QObject* const rootObject);

        static void addEventWithoutSave(const SaveManager::EventInf& inf);

	public slots:
		void clearList();
		void selectItem(QObject* const);
        void detachEventsFor(QObject* const);
        void clearAllEvents();
		bool hasPopupOpen();

	protected slots:
        void processEvents();

	signals:
		void popupShowed();
		void popupHid();

	private:
        static EventsWidgetPrivate* m_d;
};

#endif // EVENTSWIDGET_H
