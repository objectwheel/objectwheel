#include <listwidget.h>
#include <fit.h>

#include <QMimeData>
#include <QApplication>
#include <QtMath>
#include <QMouseEvent>
#include <QScrollBar>
#include <QTimer>
#include <QPropertyAnimation>
#include <QScroller>
#include <QWheelEvent>

#define ALPHA 70
#define STYLE_SHEET "\
QScrollBar:vertical { \
background: transparent; \
width: %2px; \
} QScrollBar::handle:vertical { \
background: rgba(0,0,0,%4); \
min-height: %1px; \
border-radius: %3px; \
} QScrollBar::add-line:vertical { \
background: none; \
} QScrollBar::sub-line:vertical { \
background: none; \
} QScrollBar::up-arrow:vertical, QScrollBar::down-arrow:vertical { \
background: none; \
} QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { \
background: none; \
}"

ListWidget::ListWidget(QWidget *parent)
	: QListWidget(parent)
{
	setBarOpacity(ALPHA);
	QScroller::grabGesture(this, QScroller::TouchGesture);
}

QMimeData* ListWidget::mimeData(const QList<QListWidgetItem*> items) const
{
	QMimeData *data = new QMimeData();
	data->setUrls(m_Urls[items[0]]);
	return data;
}

void ListWidget::mousePressEvent(QMouseEvent* const event)
{
	m_AngleList.clear();
	m_PreviousPoint = event->pos();
	QListWidget::mousePressEvent(event);
}

void ListWidget::mouseMoveEvent(QMouseEvent* const event)
{
	showBar();

#ifndef QT_NO_DRAGANDDROP
	/* Make dragging if direction is right out of the list widget */
	if (state() == DraggingState)
	{
		QPoint diff = event->pos() - m_PreviousPoint;
		if (QApplication::startDragDistance() > diff.manhattanLength())
			return;
		m_PreviousPoint = event->pos();

		qreal angle;
		if (0 == diff.x())
			angle = 90;
		else
			angle = qAbs(qRadiansToDegrees(qAtan(diff.y() / (qreal)diff.x())));

		m_AngleList << angle;
		if (10 < m_AngleList.size())
			m_AngleList.removeFirst();
		else
			return;

		qreal dangle = 0;
		for ( auto a : m_AngleList)
			dangle += a;
		dangle /= m_AngleList.size();

		if (25 < dangle)
			return;
	}
#endif // QT_NO_DRAGANDDROP

	QListWidget::mouseMoveEvent(event);
}

void ListWidget::wheelEvent(QWheelEvent* e)
{
	showBar();
	QListWidget::wheelEvent(e);
}

void ListWidget::setBarOpacity(const int opacity)
{
	QString styleSheet(STYLE_SHEET);
	styleSheet = styleSheet.arg(fit(15)).arg(fit(4)).arg(fit(2)).arg(opacity);
	verticalScrollBar()->setStyleSheet(styleSheet);
}

int ListWidget::getBarOpacity() const
{
	QString styleSheet = verticalScrollBar()->styleSheet();
	styleSheet = styleSheet.split("rgba(0,0,0,").at(1);
	QString value;
	if (styleSheet[0].isNumber())
		value.append(styleSheet.at(0));
	if (styleSheet[1].isNumber())
		value.append(styleSheet.at(1));
	if (styleSheet[2].isNumber())
		value.append(styleSheet.at(2));
	return value.toInt();
}

void ListWidget::showBar()
{
	setBarOpacity(ALPHA);

	static bool firstTime = true;
	static QTimer* timer = new QTimer;
	timer->stop();
	if (firstTime) {
		firstTime = false;
		connect(timer, &QTimer::timeout, [&]{
			timer->stop();
			hideBar();
		});
	}
	timer->start(400);
}

void ListWidget::hideBar()
{
	static int alpha;
	static bool running = false;
	static QTimer* timer;

	if (true == running)
		return;

	alpha = ALPHA;
	running = true;

	timer = new QTimer;
	connect(timer, &QTimer::timeout, [&] {
		if (getBarOpacity() > alpha + 2) {
			timer->stop();
			timer->deleteLater();
			running = false;
		}

		setBarOpacity(alpha);
		alpha-=2;

		if (alpha < 2) { // this "2" is constant
			timer->stop();
			timer->deleteLater();
			running = false;
		}
	});
	timer->start(10);
}
