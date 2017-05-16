#ifndef RESIZERTICK_H
#define RESIZERTICK_H

#include <QPushButton>
#include <QTimer>

class QQmlContext;
class QQuickItem;

class ResizerTick : public QPushButton
{
		Q_OBJECT

	private:
		QQuickItem* m_TrackedItem;
		QPoint m_HotspotDifference;
		QTimer m_SavingTimer;
		QQmlContext* m_RootContext;

	public:
		explicit ResizerTick(QWidget* const parent = 0);
		QQuickItem* TrackedItem() const;
		void SetTrackedItem(QQuickItem* const trackedItem);
		void SetRootContext(QQmlContext* const context);

	protected:
        void paintEvent(QPaintEvent* event) override;
        void mouseMoveEvent(QMouseEvent* event) override;
        void mousePressEvent(QMouseEvent* event) override;

	public slots:
		void FixCoord();
		void handleSavingTimeout();

	signals:
		void ItemResized(QQuickItem* const item) const;
};

#endif // RESIZERTICK_H
