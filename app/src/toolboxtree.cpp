#include <toolboxtree.h>
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

using namespace Fit;

ToolboxTree::ToolboxTree(QWidget *parent)
    : QTreeWidget(parent)
    , _indicatorButton(this)
    , _indicatorButtonVisible(false)
{
	QScroller::grabGesture(viewport(), QScroller::TouchGesture);
	QScrollerProperties prop = QScroller::scroller(viewport())->scrollerProperties();
	prop.setScrollMetric(QScrollerProperties::VerticalOvershootPolicy, QScrollerProperties::OvershootAlwaysOff);
	prop.setScrollMetric(QScrollerProperties::HorizontalOvershootPolicy, QScrollerProperties::OvershootAlwaysOff);
	prop.setScrollMetric(QScrollerProperties::DragStartDistance, 0.009);
    QScroller::scroller(viewport())->setScrollerProperties(prop);

    _indicatorButton.setVisible(_indicatorButtonVisible);

    connect(this, &ToolboxTree::currentItemChanged, [=] {
        if (currentItem() == 0)
            return;
        if (model()->rowCount() < 1) {
            _indicatorButton.hide();
            return;
        }
        auto rect = visualItemRect(currentItem());
        _indicatorButton.move(rect.width() - _indicatorButton.width() - fit(5),
                               rect.y() + rect.height()/2.0 - _indicatorButton.height()/2.0);
        _indicatorButton.setVisible(_indicatorButtonVisible && currentItem()->parent() != 0);
    });
    connect(verticalScrollBar(), &QSlider::valueChanged , [=] {
        if (currentItem() == 0)
            return;
        if (model()->rowCount() < 1) {
            _indicatorButton.hide();
            return;
        }
        auto rect = visualItemRect(currentItem());
        _indicatorButton.move(rect.width() - _indicatorButton.width() - fit(5),
                               rect.y() + rect.height()/2.0 - _indicatorButton.height()/2.0);
        _indicatorButton.setVisible(_indicatorButtonVisible && currentItem()->parent() != 0);
    });
}

void ToolboxTree::addUrls(QTreeWidgetItem* item, const QList<QUrl>& urls)
{
    _urls.insert(item, urls);
}

void ToolboxTree::removeUrls(QTreeWidgetItem* item)
{
    _urls.remove(item);
}

void ToolboxTree::clearUrls()
{
    _urls.clear();
}

const QMap<QTreeWidgetItem*, QList<QUrl>>& ToolboxTree::allUrls() const
{
    return _urls;
}

QList<QUrl> ToolboxTree::urls(QTreeWidgetItem* item) const
{
    return _urls.value(item);
}

FlatButton* ToolboxTree::indicatorButton()
{
    return &_indicatorButton;
}

void ToolboxTree::setIndicatorButtonVisible(bool value)
{
    _indicatorButtonVisible = value;
}

bool ToolboxTree::contains(const QString& itemName)
{
    for (int i = 0; i < topLevelItemCount(); i++)
        for (int j = 0; j < topLevelItem(i)->childCount(); j++)
            if (topLevelItem(i)->child(j)->text(0) == itemName)
                return true;
    return false;
}

bool ToolboxTree::categoryContains(const QString& categoryName)
{
    for (int i = 0; i < topLevelItemCount(); i++)
        if (topLevelItem(i)->text(0) == categoryName)
            return true;
    return false;
}

QTreeWidgetItem* ToolboxTree::categoryItem(const QString& categoryName)
{
    for (int i = 0; i < topLevelItemCount(); i++)
        if (topLevelItem(i)->text(0) == categoryName)
            return topLevelItem(i);
    return nullptr;
}

QMimeData* ToolboxTree::mimeData(const QList<QTreeWidgetItem*> items) const
{
    QMimeData *data = QTreeWidget::mimeData(items);
	data->setUrls(_urls[items[0]]);
    data->setText(TOOLBOX_ITEM_KEY);
	return data;
}

void ToolboxTree::mousePressEvent(QMouseEvent* event)
{
    _angleList.clear();
    _previousPoint = event->pos();
    QTreeWidget::mousePressEvent(event);
}

void ToolboxTree::mouseMoveEvent(QMouseEvent *event)
{
#if defined(Q_OS_ANDROID) || defined(Q_OS_IOS) || defined(Q_OS_WINPHONE)
	/* Make dragging if direction is right out of the list widget */
	if (state() == DraggingState)
	{
        QPoint diff = event->pos() - _previousPoint;
		if (QApplication::startDragDistance() > diff.manhattanLength())
			return;
        _previousPoint = event->pos();

		qreal angle;
		if (0 == diff.x())
			angle = 90;
		else
			angle = qAbs(qRadiansToDegrees(qAtan(diff.y() / (qreal)diff.x())));

        _angleList << angle;
        if (10 < _angleList.size())
            _angleList.removeFirst();
		else
			return;

		qreal dangle = 0;
        for ( auto a : _angleList)
			dangle += a;
        dangle /= _angleList.size();

		if (35 < dangle)
			return;
	}
#endif

    QTreeWidget::mouseMoveEvent(event);
}
