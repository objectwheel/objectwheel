#ifndef RESIZERTICK_H
#define RESIZERTICK_H

#include <QWidget>

class QQuickItem;

class ResizerTick : public QWidget
{
		Q_OBJECT

	private:
		QQuickItem* m_TrackedItem;
		QPoint m_HotspotDifference;

	public:
		explicit ResizerTick(QWidget* const parent = 0);
		QQuickItem* TrackedItem() const;
		void SetTrackedItem(QQuickItem* const trackedItem);

	protected:
		void paintEvent(QPaintEvent* const event);
		void mouseMoveEvent(QMouseEvent* const event);
		void mousePressEvent(QMouseEvent* const event);

	public slots:
		void FixCoord();

	signals:
		void ItemResized(QQuickItem* const item) const;
};

#endif // RESIZERTICK_H
