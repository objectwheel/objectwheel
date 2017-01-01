#include <bubblehead.h>
#include <fit.h>
#include <QPainter>
#include <QMouseEvent>
#include <QIcon>
#include <QGraphicsDropShadowEffect>
#include <mainwindow.h>

#if defined(Q_OS_IOS) || defined(Q_OS_ANDROID) || defined(Q_OS_WINPHONE)
#define MOBILE_FACTOR 1.3
#else
#define MOBILE_FACTOR 1.0
#endif
#define FIXED_SIZE (50.0 * (MOBILE_FACTOR))
#define PEN_SIZE 1.0

using namespace Fit;

class BubbleHeadPrivate
{
	public:
		BubbleHeadPrivate(BubbleHead* p);
		BubbleHead* parent;
		QPoint hotPoint;
		QIcon icon;
		QColor borderColor;
		QGraphicsDropShadowEffect shadowEffect;
		int notificationCount;
		bool moved;
		void fixCoord();
};


BubbleHeadPrivate::BubbleHeadPrivate(BubbleHead* p)
	: parent(p)
{
	moved = false;
	borderColor = "#666666";
	notificationCount = 0;
	shadowEffect.setBlurRadius(fit(13));
	shadowEffect.setOffset(0,fit(3));
	shadowEffect.setColor("#99000000");
	parent->setGraphicsEffect(&shadowEffect);
	QObject::connect((MainWindow*)parent->parent(), &MainWindow::resized, [this]{fixCoord();});
}

void BubbleHeadPrivate::fixCoord()
{
	QWidget* grandParent = (QWidget*)(parent->parent());
	QRect grandParentRect = grandParent->rect().adjusted(0, 0, -parent->width(), -parent->height());
	QPoint newPos = parent->pos();
	if (newPos.x() < 0) newPos.setX(0);
	if (newPos.y() < 0) newPos.setY(0);
	if (newPos.x() > grandParentRect.width()) newPos.setX(grandParentRect.width());
	if (newPos.y() > grandParentRect.height()) newPos.setY(grandParentRect.height());
	parent->move(newPos);
}

BubbleHead::BubbleHead(QWidget *parent)
	: QPushButton(parent)
	, m_d(new BubbleHeadPrivate(this))
{
}

BubbleHead::~BubbleHead()
{
	delete m_d;
}

void BubbleHead::setIcon(const QIcon& icon)
{
	m_d->icon = icon;
}

const QIcon& BubbleHead::icon() const
{
	return m_d->icon;
}

void BubbleHead::setBorderColor(const QColor& color)
{
	m_d->borderColor = color;
}

const QColor&BubbleHead::borderColor() const
{
	return m_d->borderColor;
}

void BubbleHead::setNotificationCount(const uint count)
{
	m_d->notificationCount = count;
}

int BubbleHead::notificationCount() const
{
	return m_d->notificationCount;
}

void BubbleHead::mousePressEvent(QMouseEvent* event)
{
	m_d->moved = false;
	m_d->hotPoint = event->pos();
	QPushButton::mousePressEvent(event);
}

void BubbleHead::mouseMoveEvent(QMouseEvent* event)
{
	event->accept();
	m_d->moved = true;
	QWidget* parent = (QWidget*)(BubbleHead::parent());
	QRect parentRect = parent->rect().adjusted(0, 0, -width(), -height());
	QPoint newPos = event->windowPos().toPoint() - m_d->hotPoint;
	if (newPos.x() < 0) newPos.setX(0);
	if (newPos.y() < 0) newPos.setY(0);
	if (newPos.x() > parentRect.width()) newPos.setX(parentRect.width());
	if (newPos.y() > parentRect.height()) newPos.setY(parentRect.height());
	move(newPos);
}

void BubbleHead::mouseReleaseEvent(QMouseEvent* event)
{
	if (!m_d->moved) {
		QPushButton::mouseReleaseEvent(event);
	} else {
		event->accept();
		setDown(false);
	}
}

void BubbleHead::resizeEvent(QResizeEvent*)
{
	resize(fit(FIXED_SIZE), fit(FIXED_SIZE));
}

void BubbleHead::paintEvent(QPaintEvent*)
{
	QPen p;
	p.setWidthF(fit(PEN_SIZE));
	p.setColor(m_d->borderColor);

	QPainterPath path;
	QRectF bgRect = QRectF(fit(PEN_SIZE/2.0),fit(PEN_SIZE/2.0),width()-fit(PEN_SIZE),height()-fit(PEN_SIZE));
	path.addEllipse(rect());

	QPainter painter(this);
	painter.setPen(p);
	painter.setRenderHint(QPainter::Antialiasing);
	painter.setClipPath(path);
	painter.drawPixmap(rect(),  m_d->icon.pixmap(size()));
	painter.setClipping(false);
	painter.setBrush(Qt::NoBrush);
	painter.drawEllipse(bgRect);

	if (isDown()) {
		painter.setBrush(QColor("#30000000"));
		painter.drawEllipse(bgRect);
	}

	if (m_d->notificationCount > 0) {
		QRect notfRect = QRect(fit(1),fit(3),fit(13.85*MOBILE_FACTOR),fit(16.15*MOBILE_FACTOR));

		QPen p;
		p.setWidthF(fit(1));
		p.setColor("#30000000");

		QLinearGradient g;
		g.setStart(fit(9), 0);
		g.setFinalStop(fit(9), fit(21));
		g.setColorAt(0, "#ed323f");
		g.setColorAt(1, "#da0220");

		painter.setPen(p);
		painter.setBrush(g);
		painter.drawRoundedRect(notfRect, fit(3), fit(3));

		QFont f("OpenSans Bold");
		f.setWeight(QFont::Black);

		painter.setFont(f);
		painter.setPen(Qt::white);
		if (m_d->notificationCount > 9) {
			painter.drawText(notfRect, "...", QTextOption(Qt::AlignCenter));
		} else {
			painter.drawText(notfRect, QString::number(m_d->notificationCount), QTextOption(Qt::AlignCenter));
		}
	}
}
