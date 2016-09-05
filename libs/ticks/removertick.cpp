#include <removertick.h>
#include <fit.h>
#include <QPainter>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QColor>
#include <QQuickItem>

#if !defined(Q_OS_IOS) && !defined(Q_OS_ANDROID) && !defined(Q_OS_WINPHONE)
#  define REMOVERTICK_SIZE 10
#else
#  define REMOVERTICK_SIZE 35
#endif

RemoverTick::RemoverTick(QWidget* const parent)
	: QPushButton(parent)
	, m_TrackedItem(nullptr)
{
	connect(this, SIGNAL(clicked(bool)), this, SLOT(RemoveItem()));
	setCursor(QCursor(Qt::PointingHandCursor));
	resize(fit({REMOVERTICK_SIZE, REMOVERTICK_SIZE}));
}

QQuickItem* RemoverTick::TrackedItem() const
{
	return m_TrackedItem;
}

void RemoverTick::SetTrackedItem(QQuickItem* const trackedItem)
{
	m_TrackedItem = trackedItem;
	FixCoord();
}

void RemoverTick::FixCoord()
{
	move({static_cast<int>(m_TrackedItem->x() + m_TrackedItem->width()),
		  static_cast<int>(m_TrackedItem->y())});
}

void RemoverTick::RemoveItem()
{
	hide();
	QQuickItem* const widget = m_TrackedItem;
	m_TrackedItem->deleteLater();
	m_TrackedItem = nullptr;
	emit ItemRemoved(widget);
}

void RemoverTick::paintEvent(QPaintEvent* const)
{
	QPainter p(this);
	p.setRenderHint(QPainter::Antialiasing);
	if (isDown())
	{
		QBrush b(QColor("#b8312f"));
		p.setPen(QColor("#a8211f"));
		p.setBrush(b);
		p.drawEllipse(rect().adjusted(1, 1, -1, -1));
	}
	else
	{
		QBrush b(QColor("#d14938"));
		p.setPen(QColor("#c13928"));
		p.setBrush(b);
		p.drawEllipse(rect().adjusted(1, 1, -1, -1));
	}
}
