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
	, m_HiderTimer(new QTimer)
	, m_DelayTimer(new QTimer)
	, m_Running(false)
{
	setBarOpacity(ALPHA);
	QScroller::grabGesture(this, QScroller::TouchGesture);

	connect(m_DelayTimer, &QTimer::timeout, [&]{
		m_DelayTimer->stop();
		hideBar();
	});

	connect(m_HiderTimer, &QTimer::timeout, [&] {
		if (getBarOpacity() > m_Alpha + 2) {
			m_HiderTimer->stop();
			m_Running = false;
		}

		setBarOpacity(m_Alpha);
		m_Alpha-=2;

		if (m_Alpha < 2) { // this "2" is constant
			m_HiderTimer->stop();
			m_Running = false;
		}
	});
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

#if defined(Q_OS_ANDROID) || defined(Q_OS_IOS) || defined(Q_OS_WINPHONE)
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
#endif

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
	m_DelayTimer->stop();
	m_DelayTimer->start(400);
}

void ListWidget::hideBar()
{
	if (true == m_Running)
		return;

	m_Alpha = ALPHA;
	m_Running = true;
	m_HiderTimer->start(10);
}
