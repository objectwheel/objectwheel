#include <fit.h>
#include <resizertick.h>
#include <QPainter>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QColor>
#include <QQuickItem>
#include <QQuickWidget>
#include <QApplication>
#include <savemanager.h>
#include <QQmlContext>

#if !defined(Q_OS_IOS) && !defined(Q_OS_ANDROID) && !defined(Q_OS_WINPHONE)
#    define RESIZERTICK_SIZE 15
#else
#    define RESIZERTICK_SIZE 25
#endif

using namespace Fit;
qreal width, height;

ResizerTick::ResizerTick(QWidget* const parent)
	: QPushButton(parent)
	, m_TrackedItem(nullptr)
{
	connect(&m_SavingTimer, SIGNAL(timeout()), this, SLOT(handleSavingTimeout()));
	setCursor(QCursor(Qt::SizeFDiagCursor));
	resize(RESIZERTICK_SIZE, RESIZERTICK_SIZE);
	fit(this);
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

void ResizerTick::SetRootContext(QQmlContext* const context)
{
	m_RootContext = context;
}

void ResizerTick::FixCoord()
{
	QQuickWidget* parent = qobject_cast<QQuickWidget*>(this->parent());
	QQuickItem* rootObject = parent->rootObject();
	QPointF point = rootObject->mapFromItem(m_TrackedItem->parentItem(), m_TrackedItem->position());
	move({static_cast<int>(point.x() + m_TrackedItem->width()),
		  static_cast<int>(point.y() + m_TrackedItem->height())});
}

void ResizerTick::handleSavingTimeout()
{
	SaveManager::setVariantProperty(m_RootContext->nameForObject(m_TrackedItem), "width", ::width);
	SaveManager::setVariantProperty(m_RootContext->nameForObject(m_TrackedItem), "height", ::height);
	m_SavingTimer.stop();
}

void ResizerTick::paintEvent(QPaintEvent*)
{
	QPainter p(this);
	p.setRenderHint(QPainter::Antialiasing);
	p.setPen(QColor("#1959a0"));
	p.setBrush(QColor("#4989d0"));
	p.drawEllipse(rect().adjusted(1, 1, -1, -1));
	p.drawPixmap(rect().adjusted(2, 2, -2, -2), icon().pixmap(size() - QSize(4, 4)));
}

void ResizerTick::mouseMoveEvent(QMouseEvent* event)
{
	if (!(event->buttons() & Qt::LeftButton))
		return;
	if ((event->pos() - pos()).manhattanLength()
		< QApplication::startDragDistance())
		return;

	QQuickWidget* parent = qobject_cast<QQuickWidget*>(this->parent());
	QQuickItem* rootObject = parent->rootObject();
	QPointF point = rootObject->mapFromItem(m_TrackedItem->parentItem(), m_TrackedItem->position());
	move(pos() + event->pos() - m_HotspotDifference);
	m_TrackedItem->setWidth(pos().x() - point.x());
	m_TrackedItem->setHeight(pos().y() - point.y());
	::width = pos().x() - point.x();
	::height = pos().y() - point.y();
	m_SavingTimer.start(200);
	emit ItemResized(m_TrackedItem);
}

void ResizerTick::mousePressEvent(QMouseEvent* event)
{
	m_HotspotDifference = event->pos();
}
