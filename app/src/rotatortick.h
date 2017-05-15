#ifndef ROTATORTICK_H
#define ROTATORTICK_H

#include <QPushButton>
#include <QQuickItem>
#include <QTimer>

class QQmlContext;

class RotatorTick : public QPushButton
{
		Q_OBJECT

	private:
		QQuickItem* m_TrackedItem;
		QPointF m_EndPoint;
		QTimer m_SavingTimer;
		QQmlContext* m_RootContext;

	public:
		explicit RotatorTick(QWidget* const parent = 0);
		QQuickItem* TrackedItem() const;
		void SetTrackedItem(QQuickItem* const trackedItem);
		void SetRootContext(QQmlContext* const context);

	protected:
        void paintEvent(QPaintEvent* event) override;
        void mouseMoveEvent(QMouseEvent* event) override;

	public slots:
		void FixCoord();
		inline void ResetRotation() { m_TrackedItem->setRotation(0); }
		void handleSavingTimeout();

	signals:
		void ItemRotated(QQuickItem* const item) const;
};

#endif // ROTATORTICK_H
