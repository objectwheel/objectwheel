#include <propertieswidget.h>
#include <propertyitem.h>
#include <fit.h>
#include <listwidget.h>

#include <QStyleOption>
#include <QPainter>
#include <QQuickItem>
#include <QVBoxLayout>

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

void PropertiesWidget::updateProperties(QObject* const selectedItem)
{
	/* Get selected item's properties */
	m_Properties.clear();
	auto meta = selectedItem->metaObject();
	int count = meta->propertyCount();
	for (int i = 0; i < count; i++) {
		auto property = meta->property(i);
		if (property.isDesignable()) {
			m_Properties << QPair<QMetaProperty, QObject*>(property, selectedItem);
		}
	}

	/* Update list widget */
	updateListWidget();

	setEnabled(true);

	m_ListWidget->showBar();

	emit propertiesUpdated();
}

void PropertiesWidget::updateListWidget()
{
	m_ListWidget->clear();

	for (auto property : m_Properties) {
		QListWidgetItem* item = new QListWidgetItem;
		PropertyItem* propertyItem = new PropertyItem(property);
		item->setSizeHint(propertyItem->sizeHint());
		m_ListWidget->addItem(item);
		m_ListWidget->setItemWidget(item, propertyItem);
	}
}
