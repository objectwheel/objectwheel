#include <fitter.h>
#include <resizertick.h>
#include <QPainter>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QColor>
#include <QQuickItem>
#include <QQuickWidget>

#if !defined(Q_OS_IOS) && !defined(Q_OS_ANDROID) && !defined(Q_OS_WINPHONE)
#    define RESIZERTICK_SIZE 10
#else
#    define RESIZERTICK_SIZE 20
#endif

ResizerTick::ResizerTick(QWidget* const parent)
	: QWidget(parent)
	, m_TrackedItem(nullptr)
{
	setCursor(QCursor(Qt::SizeFDiagCursor));
	resize(RESIZERTICK_SIZE, RESIZERTICK_SIZE);
	Fitter::AddWidget(this);
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
	QQuickWidget* parent = qobject_cast<QQuickWidget*>(this->parent());
	QQuickItem* rootObject = parent->rootObject();
	QPointF point = rootObject->mapFromItem(m_TrackedItem->parentItem(), m_TrackedItem->position());
	move({static_cast<int>(point.x() + m_TrackedItem->width()),
		  static_cast<int>(point.y() + m_TrackedItem->height())});
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
	m_TrackedItem->setProperty("width", pos().x() - TrackedItem()->x());
	m_TrackedItem->setProperty("height", pos().y() - TrackedItem()->y());
	emit ItemResized(m_TrackedItem);
}

void ResizerTick::mousePressEvent(QMouseEvent* const event)
{
	m_HotspotDifference = event->pos();
}
