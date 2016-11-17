#ifndef PROPERTIESWIDGET_H
#define PROPERTIESWIDGET_H

#include <listwidget.h>

#include <QWidget>
#include <QPair>
#include <QMetaProperty>

class QQuickItem;
class QVBoxLayout;

class PropertiesWidget : public QWidget
{
		Q_OBJECT

	public:
		typedef QList<QPair<QMetaProperty, QObject*>> Properties;

	private:
		Properties m_Properties;
		ListWidget* m_ListWidget;
		QVBoxLayout* m_Layout;
		QColor m_Color;

	public:
		explicit PropertiesWidget(QWidget *parent = 0);
		const Properties& properties() const;

		const QColor& color() const;
		void setColor(const QColor& color);

	protected:
		void updateListWidget();

	signals:
		void propertiesUpdated();

	public slots:
		inline void showBar() { m_ListWidget->showBar(); }
		void updateProperties(QObject* const selectedItem);
};

#endif // PROPERTIESWIDGET_H
