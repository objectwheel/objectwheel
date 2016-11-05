#include <listwidget.h>
#include <QMimeData>
#include <QApplication>
#include <QtMath>
#include <QMouseEvent>

ListWidget::ListWidget(QWidget *parent)
	: QListWidget(parent)
{
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
