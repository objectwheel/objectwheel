#include <propertieswidget.h>
#include <propertyitem.h>
#include <fit.h>
#include <listwidget.h>

#include <QStyleOption>
#include <QPainter>
#include <QQuickItem>
#include <QVBoxLayout>
#include <QLabel>

PropertiesWidget::PropertiesWidget(QWidget *parent)
	: QWidget(parent)
	, m_ListWidget(new ListWidget)
	, m_Layout(new QVBoxLayout(this))
	, m_Color(QColor("#566573"))
{
	setAutoFillBackground(true);
	QPalette p(palette());
	p.setColor(QPalette::Window, m_Color);
	setPalette(p);

	m_ListWidget->setStyleSheet(QString("QListView::item{background:none;border: 0px solid transparent;}"
										"QListView { border:0px solid white;background:rgba(%1,%2,%3,%4);}")
								.arg(m_Color.red()).arg(m_Color.green())
								.arg(m_Color.blue()).arg(m_Color.alpha()));
	m_ListWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	m_ListWidget->setHorizontalScrollMode(ListWidget::ScrollPerPixel);
	m_ListWidget->setVerticalScrollMode(ListWidget::ScrollPerPixel);
	m_ListWidget->setSelectionBehavior(ListWidget::SelectRows);
	m_ListWidget->setFocusPolicy(Qt::NoFocus);
	m_ListWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_Layout->setSpacing(0);
	m_Layout->setContentsMargins(fit(10), 5, fit(5), fit(10));
	m_Layout->addWidget(m_ListWidget);
}

const PropertiesWidget::Properties& PropertiesWidget::properties() const
{
	return m_Properties;
}

const QColor& PropertiesWidget::color() const
{
	return m_Color;
}

void PropertiesWidget::setColor(const QColor& color)
{
	m_Color = color;
	QPalette p(palette());
	p.setColor(QPalette::Window, m_Color);
	setPalette(p);
}

void PropertiesWidget::refreshList(QObject* const selectedItem)
{
	clearList();

	/* Get selected item's properties */
	auto meta = selectedItem->metaObject();
	int count = meta->propertyCount();
	for (int i = 0; i < count; i++) {
		auto property = meta->property(i);
		if (!QString(property.name()).startsWith("__")) {
			m_Properties << Property(property, selectedItem);
		}
	}

	/* Update list widget */
	refreshListWidget();

	setEnabled(true);

	m_ListWidget->showBar();

	emit listRefreshed();
}

void PropertiesWidget::clearList()
{
	m_ListWidget->clear();
	m_Properties.clear();
}

void PropertiesWidget::updateLayout()
{
	for (int i=0; i<m_ListWidget->count(); i++) {
		QWidget* propertyItem = m_ListWidget->itemWidget(m_ListWidget->item(i));
		propertyItem->resize(m_ListWidget->width() - fit(4), propertyItem->height());
		propertyItem->setFixedWidth(m_ListWidget->width() - fit(4));
		m_ListWidget->item(i)->setSizeHint(QSize(m_ListWidget->width() - fit(4), propertyItem->sizeHint().height()));
	}
}

void PropertiesWidget::refreshListWidget()
{
	for (auto property : m_Properties) {
		QListWidgetItem* item = new QListWidgetItem;
		PropertyItem* propertyItem = new PropertyItem(property);
		if (!propertyItem->isValid()) {
			delete item;
			propertyItem->deleteLater();
			continue;
		}
		connect(propertyItem, &PropertyItem::valueApplied, [&] {
			emit propertyChanged(property);
		});
		propertyItem->resize(m_ListWidget->width() - fit(4), propertyItem->height());
		propertyItem->setFixedWidth(m_ListWidget->width() - fit(4));
		item->setSizeHint(QSize(m_ListWidget->width() - fit(4), propertyItem->sizeHint().height()));
		m_ListWidget->addItem(item);
		m_ListWidget->setItemWidget(item, propertyItem);
	}
}
