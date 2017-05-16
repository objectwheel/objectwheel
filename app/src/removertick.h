#ifndef REMOVERTICK_H
#define REMOVERTICK_H

#include <QPushButton>

class QQuickItem;

class RemoverTick : public QPushButton
{
		Q_OBJECT

	private:
		QQuickItem* m_TrackedItem;

	public:
		explicit RemoverTick(QWidget* const parent = 0);
		QQuickItem* TrackedItem() const;
		void SetTrackedItem(QQuickItem* const trackedItem);

	protected:
        void paintEvent(QPaintEvent* event) override;

	signals:
		void ItemRemoved(QQuickItem* const item) const;

	public slots:
		void FixCoord();

	private slots:
		void RemoveItem();

};

#endif // REMOVERTICK_H
