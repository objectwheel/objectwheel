#ifndef SELECTIONMANAGER_H
#define SELECTIONMANAGER_H

#include <QWidget>

class QQuickItem;

class SelectionManager : public QWidget
{
		Q_OBJECT
	private:
		QList<QQuickItem*> m_SelectedItems;

	public:
		explicit SelectionManager(QWidget *parent = 0);

		const QList<QQuickItem*>& selectedItems() const;
		void setSelectedItems(const QList<QQuickItem*>& selectedItems);

	protected:
		void paintEvent(QPaintEvent *event);

	signals:

	public slots:
};

#endif // SELECTIONMANAGER_H
