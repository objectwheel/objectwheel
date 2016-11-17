#include <selectionmanager.h>
#include <QPainter>

const QList<QQuickItem*>& SelectionManager::selectedItems() const
{
	return m_SelectedItems;
}

void SelectionManager::setSelectedItems(const QList<QQuickItem*>& selectedItems)
{
	m_SelectedItems = selectedItems;
}

void SelectionManager::paintEvent(QPaintEvent* event)
{
	QPainter painter(this);
	for (auto item : m_SelectedItems) {


	}
}

SelectionManager::SelectionManager(QWidget *parent) : QWidget(parent)
{

}
