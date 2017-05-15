#include <combobox.h>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <css.h>
#include <fit.h>
#include <QColor>
#include <QLabel>
#include <QLineEdit>
#include <QLinearGradient>
#include <QPainter>
#include <QMouseEvent>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <listwidget.h>
#include <QString>

#define LIST_LEGHT 195 // 90

#define CSS_COMBOBOX_LISTWIDGET \
"QListView {\
	background:#566573;border:none;\
	border-bottom-left-radius:%1px;\
	border-bottom-right-radius:%1px\
} QListView::item {\
	color:white;\
	border: none;\
	border-radius: %1px;\
	padding: %2px;\
	margin: %2px;\
} QListView::item:selected {\
	background: white;\
	color: black\
}"

using namespace Fit;

class ComboBoxPrivate
{
	public:
		bool down = false;
		bool collapsed = true;
		QIcon icon;
		QColor color = "#cccccc";
		QRectF indicatorRect;
		int previousMinHeight;
		QLineEdit searchEdit;
		QStringList items;
		ListWidget itemListWidget;
		ComboBox* parent;
		ComboBoxPrivate(ComboBox* parent);

	public slots:
		void filterListWidget(const QString& filter);
		void indicatorClickHandler(bool checked);
};

ComboBox::ComboBox(QWidget *parent)
	: QWidget(parent)
	, m_d(new ComboBoxPrivate(this))
{
	setCursor(Qt::PointingHandCursor);
	connect(&m_d->itemListWidget, SIGNAL(currentTextChanged(QString)), this, SIGNAL(currentTextChanged(QString)));
}

void ComboBox::setColor(const QColor& color)
{
	m_d->color = color;
}

void ComboBox::setIcon(const QIcon& icon)
{
	m_d->icon = icon;
}

void ComboBox::setPlaceHolderText(const QString& text)
{
	m_d->searchEdit.setPlaceholderText(text);
}

void ComboBox::addItem(const QString& item)
{
	m_d->items << item;
	m_d->itemListWidget.addItem(item);
}

const QString ComboBox::item(const int index) const
{
	return m_d->itemListWidget.item(index)->text();
}

void ComboBox::clear()
{
	m_d->items.clear();
	m_d->itemListWidget.clear();
	m_d->searchEdit.clear();
}

const QString ComboBox::currentItem() const
{
	auto currentItem =  m_d->itemListWidget.currentItem();
	if (!currentItem) return QString();
	return currentItem->text();
}

void ComboBox::setCurrentItem(const QString& text)
{
	int row = -1;
	for (int i = 0; i < m_d->itemListWidget.count(); i++)
		if (m_d->itemListWidget.item(i)->text() == text)
			row = i;
	if (row >= 0) {
		m_d->itemListWidget.setCurrentRow(row);
		m_d->searchEdit.setText(text);
	}
}

const QStringList ComboBox::items() const
{
	QStringList list;
	for (int i = 0; i < m_d->itemListWidget.count(); i++)
		list << m_d->itemListWidget.item(i)->text();
	return list;
}

ComboBox::~ComboBox()
{
	delete m_d;
}

void ComboBox::paintEvent(QPaintEvent* event)
{
	QPainter painter(this);
	painter.save();

	if (!m_d->collapsed) {
		painter.setPen(QColor("#444444"));
		painter.setBrush(QColor("#566573"));
		painter.drawRoundedRect(rect().adjusted(0,0,-fit(1),-fit(1)), fit(2), fit(2));
	}

	painter.setRenderHint(QPainter::Antialiasing);
	painter.setPen(Qt::NoPen);

	/**/
	QLinearGradient baseShadow;
	baseShadow.setStart(width()/2.0, 0);
	baseShadow.setFinalStop(width()/2.0, fit(30));
	baseShadow.setColorAt(0, "#50000000");
	baseShadow.setColorAt(1, "#20000000");
	painter.setBrush(baseShadow);
	painter.drawRoundedRect(0, fit(23), width(), fit(8), fit(2), fit(2));

	/**/
	static const QPointF pts[3] = {
		QPointF(m_d->indicatorRect.center().x() - fit(6), fit(31)),
		QPointF(m_d->indicatorRect.center().x() + fit(6), fit(31)),
		QPointF(m_d->indicatorRect.center().x(), fit(40))
	};
	painter.setBrush(QColor("#20000000"));
	painter.drawConvexPolygon(pts, 3);

	/**/
	painter.setBrush(Qt::white);
	painter.drawRoundedRect(0, 0, width() - fit(2), fit(29), fit(2), fit(2));

	/**/
	QColor color;
	if (m_d->down) {
		color = m_d->color.darker(120);
	} else {
		color = m_d->color;
	}
	painter.setBrush(color);
	painter.drawRect(m_d->indicatorRect.adjusted(0, 0, -fit(2), 0));
	painter.drawRoundedRect(m_d->indicatorRect, fit(2), fit(2));

	/**/
	static const QPointF points[3] = {
		QPointF(m_d->indicatorRect.center().x() - fit(6), fit(29)),
		QPointF(m_d->indicatorRect.center().x() + fit(6), fit(29)),
		QPointF(m_d->indicatorRect.center().x(), fit(37))
	};
	painter.setBrush(color);
	painter.drawConvexPolygon(points, 3);

	/**/
	if (!m_d->icon.isNull()) {
		QPixmap p(m_d->icon.pixmap(m_d->indicatorRect.size().toSize()));
		painter.drawPixmap(m_d->indicatorRect.toRect(), p);
	}

	painter.restore();
	if (!m_d->collapsed) {
		painter.setPen("#444444");
		painter.drawLine(0, fit(29), width() - fit(30), fit(29));
	}
	QWidget::paintEvent(event);
}

void ComboBox::mousePressEvent(QMouseEvent* event)
{
	if (m_d->indicatorRect.contains(event->pos())) {
		m_d->down = !m_d->down;
		update();
		m_d->indicatorClickHandler(m_d->down);
	}
	QWidget::mousePressEvent(event);
}

void ComboBox::resizeEvent(QResizeEvent* event)
{
	m_d->indicatorRect = QRectF(width() - fit(30), 0, fit(30), fit(30));
	m_d->searchEdit.setGeometry(fit(2),fit(2),width()-fit(34),fit(26));
	m_d->itemListWidget.setGeometry(fit(1), fit(40), width() - fit(1), m_d->itemListWidget.height());
	QWidget::resizeEvent(event);
}

void ComboBox::showPopup()
{
	if (!m_d->down) {
		m_d->down = !m_d->down;
		update();
		m_d->indicatorClickHandler(true);
	}
}

void ComboBox::hidePopup()
{
	if (m_d->down) {
		m_d->down = !m_d->down;
		update();
		m_d->indicatorClickHandler(false);
	}
}

void ComboBoxPrivate::indicatorClickHandler(bool checked)
{
	if (checked) {
		previousMinHeight = parent->minimumHeight();
		collapsed = false;
		searchEdit.setReadOnly(false);
		auto animation = new QPropertyAnimation(parent, "minimumHeight");
		animation->setDuration(500);
		animation->setStartValue(previousMinHeight);
		animation->setEndValue(fit(LIST_LEGHT));
		animation->setEasingCurve(QEasingCurve::OutExpo);
		QObject::connect(animation, SIGNAL(finished()), animation, SLOT(deleteLater()));

		auto animation2 = new QPropertyAnimation(parent, "maximumHeight");
		animation2->setDuration(500);
		animation2->setStartValue(previousMinHeight);
		animation2->setEndValue(fit(LIST_LEGHT));
		animation2->setEasingCurve(QEasingCurve::OutExpo);
		QObject::connect(animation2, SIGNAL(finished()), animation2, SLOT(deleteLater()));

		auto animation3 = new QPropertyAnimation(&itemListWidget, "geometry");
		animation3->setDuration(500);
		animation3->setStartValue(QRect(itemListWidget.x(), itemListWidget.y(),  itemListWidget.width(), 0));
		animation3->setEndValue(QRect(itemListWidget.x(),  itemListWidget.y(),  itemListWidget.width(), fit(LIST_LEGHT-41)));
		animation3->setEasingCurve(QEasingCurve::OutExpo);
		QObject::connect(animation3, SIGNAL(finished()), animation3, SLOT(deleteLater()));

		QParallelAnimationGroup *group = new QParallelAnimationGroup;
		group->addAnimation(animation);
		group->addAnimation(animation2);
		group->addAnimation(animation3);
		QObject::connect(group, SIGNAL(finished()), parent, SIGNAL(popupShowed()));
		QObject::connect(group, SIGNAL(finished()), group, SLOT(deleteLater()));
		group->start();
	} else {
		searchEdit.setReadOnly(true);
		searchEdit.setText(parent->currentItem());
		auto animation = new QPropertyAnimation(parent, "minimumHeight");
		animation->setDuration(100);
		animation->setStartValue(fit(LIST_LEGHT));
		animation->setEndValue(previousMinHeight);
		QObject::connect(animation, SIGNAL(finished()), animation, SLOT(deleteLater()));
		animation->start();

		auto animation2 = new QPropertyAnimation(parent, "maximumHeight");
		animation2->setDuration(100);
		animation2->setStartValue(fit(LIST_LEGHT));
		animation2->setEndValue(previousMinHeight);
		QObject::connect(animation2, SIGNAL(finished()), animation2, SLOT(deleteLater()));

		auto animation3 = new QPropertyAnimation(&itemListWidget, "geometry");
		animation3->setDuration(100);
		animation3->setStartValue(QRect( itemListWidget.x(),  itemListWidget.y(),  itemListWidget.width(), fit(LIST_LEGHT-41)));
		animation3->setEndValue(QRect( itemListWidget.x(),  itemListWidget.y(),  itemListWidget.width(), 0));
		QObject::connect(animation3, SIGNAL(finished()), animation3, SLOT(deleteLater()));

		QParallelAnimationGroup *group = new QParallelAnimationGroup;
		group->addAnimation(animation);
		group->addAnimation(animation2);
		group->addAnimation(animation3);
		QObject::connect(group, &QParallelAnimationGroup::finished, [=]{collapsed = true;});
		QObject::connect(group, SIGNAL(finished()), parent, SLOT(update()));
		QObject::connect(group, SIGNAL(finished()), parent, SIGNAL(popupHided()));
		QObject::connect(group, SIGNAL(finished()), group, SLOT(deleteLater()));
		group->start();
	}
}

ComboBoxPrivate::ComboBoxPrivate(ComboBox* p)
	: parent(p)
{
	searchEdit.setParent(parent);
	searchEdit.setStyleSheet("background:transparent;border:none;");
	searchEdit.setReadOnly(true);

	itemListWidget.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	itemListWidget.setFocusPolicy(Qt::NoFocus);
	itemListWidget.setStyleSheet(QString(CSS_COMBOBOX_LISTWIDGET).arg(fit(2)).arg(1));
	itemListWidget.setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	itemListWidget.setDragDropMode(QAbstractItemView::NoDragDrop);
	itemListWidget.setSelectionBehavior(QAbstractItemView::SelectRows);
	itemListWidget.setSelectionMode(ListWidget::SingleSelection);
	itemListWidget.setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
	itemListWidget.setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);

	itemListWidget.setParent(parent);
	itemListWidget.resize(parent->width(), 0);
	QObject::connect(&searchEdit, (void(QLineEdit::*)(const QString&))(&QLineEdit::textEdited), [=] (const QString& str){
		filterListWidget(str);
	});

	QObject::connect(&itemListWidget, SIGNAL(clicked(QModelIndex)), parent, SLOT(hidePopup()));
}

void ComboBoxPrivate::filterListWidget(const QString& filter)
{
	itemListWidget.clear();

	if (filter.isEmpty()) {
		for (auto item : items) {
			itemListWidget.addItem(item);
		}
		return;
	}

	for (auto item : items) {
		if (item.contains(filter, Qt::CaseInsensitive)) {
			itemListWidget.addItem(item);
		}
	}
}
