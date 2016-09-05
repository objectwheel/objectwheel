#include <resizertick.h>
#include <fit.h>
#include <QPainter>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QColor>
#include <QQuickItem>

#if !defined(Q_OS_IOS) && !defined(Q_OS_ANDROID) && !defined(Q_OS_WINPHONE)
#  define RESIZERTICK_SIZE 10
#else
#  define RESIZERTICK_SIZE 35
#endif

ResizerTick::ResizerTick(QWidget* const parent)
	: QWidget(parent)
	, m_TrackedItem(nullptr)
{
	setCursor(QCursor(Qt::SizeFDiagCursor));
	resize(fit({RESIZERTICK_SIZE, RESIZERTICK_SIZE}));
}

QQuickItem* ResizerTick::TrackedItem() const
{
	return m_TrackedItem;
}

void ResizerTick::SetTrackedItem(QQuickItem* const trackedItem)
{
	m_TrackedItem = trackedItem;
	FixCoord();
}

void ResizerTick::FixCoord()
{
	move({static_cast<int>(m_TrackedItem->x() + m_TrackedItem->width()),
		  static_cast<int>(m_TrackedItem->y() + m_TrackedItem->height())});
}

void ResizerTick::paintEvent(QPaintEvent* const)
{
	QPainter p(this);
	p.setRenderHint(QPainter::Antialiasing);
	p.setPen(QColor("#1959a0"));
	p.setBrush(QColor("#2969b0"));
	p.drawEllipse(rect().adjusted(1, 1, -1, -1));
}

void ResizerTick::mouseMoveEvent(QMouseEvent* const event)
{
	move(pos() + event->pos() - m_HotspotDifference);
	m_TrackedItem->setProperty("width",pos().x()-TrackedItem()->x());
	m_TrackedItem->setProperty("height",pos().y()-TrackedItem()->y());
	emit ItemResized(m_TrackedItem);
}

void ResizerTick::mousePressEvent(QMouseEvent* const event)
{
	m_HotspotDifference = event->pos();
}
