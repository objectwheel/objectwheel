#ifndef ROTATORTICK_H
#define ROTATORTICK_H

#include <QPushButton>
#include <QQuickItem>

class RotatorTick : public QPushButton
{
		Q_OBJECT

	private:
		QQuickItem* m_TrackedItem;
		QPointF m_EndPoint;

	public:
		explicit RotatorTick(QWidget* const parent = 0);
		QQuickItem* TrackedItem() const;
		void SetTrackedItem(QQuickItem* const trackedItem);

	protected:
		void paintEvent(QPaintEvent* const event);
		void mouseMoveEvent(QMouseEvent* const event);

	public slots:
		void FixCoord();
		inline void ResetRotation() { m_TrackedItem->setRotation(0); }

	signals:
		void ItemRotated(QQuickItem* const item) const;
};

#endif // ROTATORTICK_H
