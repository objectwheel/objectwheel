#ifndef PROPERTYITEM_H
#define PROPERTYITEM_H

#include <QWidget>
#include <QMetaProperty>

class QQuickItem;
class QQmlContext;

class PropertyItem : public QWidget
{
		Q_OBJECT

	private:
		QPair<QMetaProperty, QObject*> m_Property;
		bool m_Valid;


	public:
		explicit PropertyItem(const QPair<QMetaProperty, QObject*>& property, QWidget *parent = 0);
		explicit PropertyItem(QObject* const selectedItem, QQmlContext* const context, QWidget *parent = 0);
		const QPair<QMetaProperty, QObject*>& property() const;
		inline bool isValid() const { return m_Valid; }

	protected:
		void fillCup();
		void fillId(QObject* const selectedItem, QQmlContext* const context);
		void applyValue(const QVariant& value);
		bool eventFilter(QObject* o, QEvent* e);
		void paintEvent(QPaintEvent *e);

	protected slots:
		void applyId(const QString& id, QObject* const selectedItem, QQmlContext* const context);

	signals:
		void valueApplied();
};

#endif // PROPERTYITEM_H
