#include <fit.h>
#include <rotatortick.h>
#include <savemanager.h>
#include <QPainter>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QColor>
#include <QQuickItem>
#include <QQuickWidget>
#include <QtMath>
#include <QApplication>
#include <QQmlContext>

#if !defined(Q_OS_IOS) && !defined(Q_OS_ANDROID) && !defined(Q_OS_WINPHONE)
#    define ROTATORTICK_SIZE 12
#else
#    define ROTATORTICK_SIZE 25
#endif

using namespace Fit;
qreal rotation;

RotatorTick::RotatorTick(QWidget* const parent)
	: QPushButton(parent)
	, m_TrackedItem(nullptr)
{
	connect(&m_SavingTimer, SIGNAL(timeout()), this, SLOT(handleSavingTimeout()));
	connect(this, &RotatorTick::clicked, this, &RotatorTick::ResetRotation);
	setCursor(QCursor(Qt::CrossCursor));
	resize(ROTATORTICK_SIZE, ROTATORTICK_SIZE);
	fit(this);
}

QQuickItem* RotatorTick::TrackedItem() const
{
	return m_TrackedItem;
}

void RotatorTick::SetRootContext(QQmlContext* const context)
{
	m_RootContext = context;
}

void RotatorTick::handleSavingTimeout()
{
	SaveManager::setVariantProperty(m_RootContext->nameForObject(m_TrackedItem), "rotation", ::rotation);
	m_SavingTimer.stop();
}

void RotatorTick::SetTrackedItem(QQuickItem* const trackedItem)
{
	m_TrackedItem = trackedItem;
	FixCoord();
}

void RotatorTick::FixCoord()
{
	QQuickWidget* parent = qobject_cast<QQuickWidget*>(this->parent());
	QQuickItem* rootObject = parent->rootObject();
	QPointF point = rootObject->mapFromItem(m_TrackedItem->parentItem(), m_TrackedItem->position());
	move({static_cast<int>(point.x() + m_TrackedItem->width()),
		  static_cast<int>(point.y() - height())});
}

void RotatorTick::ResetRotation()
{
    m_TrackedItem->setRotation(0);
    SaveManager::setVariantProperty(m_RootContext->nameForObject(m_TrackedItem), "rotation", ::rotation);
}

void RotatorTick::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
	if (isDown())
	{
		QBrush b(QColor("#d88c05"));
		p.setPen(QColor("#c87c05"));
		p.setBrush(b);
		p.drawEllipse(rect().adjusted(1, 1, -1, -1));
	}
	else
	{
		QBrush b(QColor("#f8ac24"));
		p.setPen(QColor("#e89c14"));
		p.setBrush(b);
		p.drawEllipse(rect().adjusted(1, 1, -1, -1));
	}
	p.drawPixmap(rect().adjusted(2, 2, -2, -2), icon().pixmap(size() - QSize(4, 4)));
}

void RotatorTick::mouseMoveEvent(QMouseEvent *event)
{
	if (!(event->buttons() & Qt::LeftButton))
		return;
	if ((event->pos() - pos()).manhattanLength()
		< QApplication::startDragDistance())
		return;

	QQuickWidget* parent = qobject_cast<QQuickWidget*>(this->parent());
	QQuickItem* rootObject = parent->rootObject();
	QPointF cursor = rootObject->mapFromGlobal(QCursor::pos());
	QPointF center = rootObject->mapFromItem(m_TrackedItem->parentItem(), {m_TrackedItem->x() + m_TrackedItem->width()/2.0,
																		   m_TrackedItem->y() + m_TrackedItem->height()/2.0});
	qreal angle = -qRadiansToDegrees(qAtan((center.y() - cursor.y())/(cursor.x() - center.x())));
	if (cursor.x() - center.x() < 0)
		angle -= 180;
	::rotation = angle;
	m_TrackedItem->setRotation(angle);
	m_SavingTimer.start(200);
	emit ItemRotated(m_TrackedItem);
}
