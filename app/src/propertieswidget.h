#ifndef PROPERTIESWIDGET_H
#define PROPERTIESWIDGET_H

#include <listwidget.h>

#include <QWidget>
#include <QPair>
#include <QMetaProperty>

class QQuickItem;
class QVBoxLayout;
class QLineEdit;

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
		QLineEdit* m_SearchEdit;

	public:
		explicit PropertiesWidget(QWidget *parent = 0);
		const Properties& properties() const;

		const QColor& color() const;
		void setColor(const QColor& color);

	protected:
		void refreshListWidget();

	signals:
		void listRefreshed();
		void propertyChanged(const Property& property);

	public slots:
		inline void showBar() { m_ListWidget->showBar(); }
		void refreshList(QObject* const selectedItem, const QString& filter = QString());
		void clearList();
		void updateLayout();
};

#endif // PROPERTIESWIDGET_H
