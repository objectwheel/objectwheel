#include <fitter.h>
#include <removertick.h>
#include <QPainter>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QColor>
#include <QQuickItem>
#include <QQuickWidget>

#if !defined(Q_OS_IOS) && !defined(Q_OS_ANDROID) && !defined(Q_OS_WINPHONE)
#    define REMOVERTICK_SIZE 15
#else
#    define REMOVERTICK_SIZE 25
#endif

RemoverTick::RemoverTick(QWidget* const parent)
	: QPushButton(parent)
	, m_TrackedItem(nullptr)
{
	connect(this, SIGNAL(clicked(bool)), this, SLOT(RemoveItem()));
	setCursor(QCursor(Qt::PointingHandCursor));
	resize(REMOVERTICK_SIZE, REMOVERTICK_SIZE);
	Fitter::AddWidget(this);
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
	QQuickWidget* parent = qobject_cast<QQuickWidget*>(this->parent());
	QQuickItem* rootObject = parent->rootObject();
	QPointF point = rootObject->mapFromItem(m_TrackedItem->parentItem(), m_TrackedItem->position());
	move({static_cast<int>(point.x() - width()),
		  static_cast<int>(point.y() + m_TrackedItem->height())});
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
		QBrush b(QColor("#f15958"));
		p.setPen(QColor("#c13928"));
		p.setBrush(b);
		p.drawEllipse(rect().adjusted(1, 1, -1, -1));
	}
	p.drawPixmap(rect().adjusted(2, 2, -2, -2), icon().pixmap(size() - QSize(4, 4)));
}
