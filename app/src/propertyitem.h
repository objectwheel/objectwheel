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
		bool m_Valid;

	protected:
		void fillCup();
		void applyValue(const QVariant& value);
		bool eventFilter(QObject* o, QEvent* e);
		void paintEvent(QPaintEvent *e);

	public:
		explicit PropertyItem(const QPair<QMetaProperty, QObject*>& property, QWidget *parent = 0);
		const QPair<QMetaProperty, QObject*>& property() const;
		inline bool isValid() const { return m_Valid; }

	signals:
		void valueApplied();
};

#endif // PROPERTYITEM_H
