#ifndef PROPERTYITEM_H
#define PROPERTYITEM_H

#include <QWidget>
#include <QMetaProperty>

class QQuickItem;

class PropertyItem : public QWidget
{
		Q_OBJECT

	private:
		QPair<QMetaProperty, QObject*> m_Property;

	protected:
		void fillCup();
		void applyValue(const QVariant& value);

	public:
		explicit PropertyItem(QWidget *parent = 0);
		PropertyItem(const QPair<QMetaProperty, QObject*>& property, QWidget *parent = 0);

		const QPair<QMetaProperty, QObject*>& property() const;
		void setProperty(const QPair<QMetaProperty, QObject*>& property);

	signals:

	public slots:

};

#endif // PROPERTYITEM_H
