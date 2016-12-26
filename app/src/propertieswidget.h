#ifndef PROPERTIESWIDGET_H
#define PROPERTIESWIDGET_H

#include <listwidget.h>

#include <QWidget>
#include <QPair>
#include <QMetaProperty>

class QQmlContext;
class QVBoxLayout;
class LineEdit;

class PropertiesWidget : public QWidget
{
		Q_OBJECT

	public:
		typedef QPair<QMetaProperty, QObject*> Property;
		typedef QList<Property> Properties;

	private:
		Properties m_Properties;
		ListWidget* m_ListWidget;
		QVBoxLayout* m_Layout;
		QColor m_Color;
		QObject* m_LastObject;
		LineEdit* m_SearchEdit;
		QQmlContext* m_rootContext;

	public:
		explicit PropertiesWidget(QWidget *parent = 0);
		const Properties& properties() const;

		const QColor& color() const;
		void setColor(const QColor& color);
		void setRootContext(QQmlContext* const context);

	protected:
		void refreshListWidget(QObject* const selectedItem);
		void showEvent(QShowEvent *event);
		void fixItemsGeometry();

	signals:
		void listRefreshed();
		void propertyChanged(const Property& property);
		void idChanged(const QString& newId);

	public slots:
		inline void showBar() { m_ListWidget->showBar(); }
		void refreshList(QObject* const selectedItem, const QString& filter = QString());
		void clearList();
};

#endif // PROPERTIESWIDGET_H
