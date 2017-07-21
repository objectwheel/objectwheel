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

#define TOOLBOX_ITEM_KEY "QURBUEFaQVJMSVlJWiBIQUZJWg"

#define STYLE_SHEET "\
QScrollBar:vertical { \
	background: transparent; \
    width: %2px; \
} QScrollBar::handle:vertical { \
	background: rgba(255,255,255,%4); \
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

using namespace Fit;

ListWidget::ListWidget(QWidget *parent)
	: QListWidget(parent)
{
    QString styleSheet(STYLE_SHEET);
    styleSheet = styleSheet.arg(fit(15)).arg(fit(4)).arg(fit(2)).arg(150);
    verticalScrollBar()->setStyleSheet(styleSheet);

	QScroller::grabGesture(viewport(), QScroller::TouchGesture);
	QScrollerProperties prop = QScroller::scroller(viewport())->scrollerProperties();
	prop.setScrollMetric(QScrollerProperties::VerticalOvershootPolicy, QScrollerProperties::OvershootAlwaysOff);
	prop.setScrollMetric(QScrollerProperties::HorizontalOvershootPolicy, QScrollerProperties::OvershootAlwaysOff);
	prop.setScrollMetric(QScrollerProperties::DragStartDistance, 0.009);
    QScroller::scroller(viewport())->setScrollerProperties(prop);
}

bool ListWidget::contains(const QString& itemName)
{
    for (int i = 0; i < count(); i++)
        if (item(i)->text() == itemName)
            return true;

    return false;
}

QMimeData* ListWidget::mimeData(const QList<QListWidgetItem*> items) const
{
	QMimeData *data = QListWidget::mimeData(items);
	data->setUrls(m_Urls[items[0]]);
    data->setText(TOOLBOX_ITEM_KEY);
	return data;
}

void ListWidget::mousePressEvent(QMouseEvent* event)
{
	m_AngleList.clear();
	m_PreviousPoint = event->pos();
	QListWidget::mousePressEvent(event);
}

void ListWidget::mouseMoveEvent(QMouseEvent *event)
{
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

		if (35 < dangle)
			return;
	}
#endif

	QListWidget::mouseMoveEvent(event);
}
