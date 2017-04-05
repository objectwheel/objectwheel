#include <pageswidget.h>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <flatbutton.h>
#include <listwidget.h>
#include <QQuickWidget>
#include <css.h>
#include <fit.h>
#include <lineedit.h>
#include <QLineEdit>
#include <QQmlEngine>
#include <QQmlComponent>
#include <QQuickItem>
#include <QQmlContext>
#include <QQmlProperty>
#include <QMessageBox>
#include <savemanager.h>
#include <bindingwidget.h>

using namespace Fit;

#define PAGE_CODE "\
import QtQuick 2.0 \n\
Item { \n\
			id:%1 \n\
				   \n\
				   function show() { \n\
				for (var i = 0; i < swipeView.count; i++) { \n\
					if (swipeView.itemAt(i) === %1) { \n\
						swipeView.currentIndex = i \n\
													} \n\
														  } \n\
								   } \n\
			\n\
	 }"


class PagesWidgetPrivate
{
	public:
		PagesWidget* parent;
		QQuickItem* swipeItem;
		QQmlContext* rootContext;
		BindingWidget* bindingWidget;
		QList<QQuickItem*>* itemList;
		QList<QUrl>* urlList;
		QVBoxLayout verticalLayout;
		QHBoxLayout horizontalLayout;
		QHBoxLayout horizontalLayout_2;
		FlatButton addButton;
		FlatButton saveButton;
		FlatButton removeButton;
		ListWidget pagesListWidget;
		LineEdit nameEdit;
		PagesWidgetPrivate(PagesWidget* p);
		const QList<QQuickItem*> GetAllChildren(QQuickItem* const item) const;
		bool checkName(const QString& name) const;

	public slots:
		void removeButtonClicked();
		void addButtonClicked();
		void saveButtonClicked();
};

PagesWidgetPrivate::PagesWidgetPrivate(PagesWidget* p)
	: parent(p)
{
	pagesListWidget.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	pagesListWidget.setFocusPolicy(Qt::NoFocus);
	pagesListWidget.setStyleSheet(CSS::BindingListWidget);
	pagesListWidget.setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	pagesListWidget.setDragDropMode(QAbstractItemView::NoDragDrop);
	pagesListWidget.setSelectionBehavior(QAbstractItemView::SelectRows);
	pagesListWidget.setSelectionMode(ListWidget::SingleSelection);
	pagesListWidget.setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
	pagesListWidget.setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
	QObject::connect(&pagesListWidget,(void(ListWidget::*)(int))(&ListWidget::currentRowChanged),[=](int i){removeButton.setEnabled(i>=0);});
	QObject::connect(&pagesListWidget,(void(ListWidget::*)(int))(&ListWidget::currentRowChanged),[=](int i)
	{
		if (i>=0) {
			saveButton.setEnabled(i>=0);
			nameEdit.setText(pagesListWidget.currentItem()->text());
			swipeItem->setProperty("currentIndex", pagesListWidget.currentRow());
		}
	});

	addButton.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	addButton.setColor("#1e8145");
	addButton.setFixedSize(fit(30),fit(30));
    addButton.setRadius(fit(14));
	addButton.setIconSize(QSize(fit(16),fit(16)));
	addButton.setIcon(QIcon(":/resources/images/plus.png"));
	QObject::connect(&addButton, (void(FlatButton::*)(bool))(&FlatButton::clicked), [=] {addButtonClicked();});

	removeButton.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	removeButton.setColor("#c03638");
	removeButton.setFixedSize(fit(30),fit(30));
    removeButton.setRadius(fit(14));
	removeButton.setIconSize(QSize(fit(16),fit(16)));
	removeButton.setIcon(QIcon(":/resources/images/minus.png"));
	removeButton.setDisabled(true);
	QObject::connect(&removeButton, (void(FlatButton::*)(bool))(&FlatButton::clicked), [=] {removeButtonClicked();});

	saveButton.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	saveButton.setColor("#2b5796");
	saveButton.setFixedSize(fit(30),fit(30));
    saveButton.setRadius(fit(14));
	saveButton.setIconSize(QSize(fit(16),fit(16)));
	saveButton.setIcon(QIcon(":/resources/images/save-icon.png"));
	saveButton.setDisabled(true);
	QObject::connect(&saveButton, (void(FlatButton::*)(bool))(&FlatButton::clicked), [=] {saveButtonClicked();});

	nameEdit.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	nameEdit.setFixedHeight(fit(30));
	nameEdit.setIcon(QIcon(":/resources/images/advanced.png"));
	nameEdit.setPlaceholderText("Page id");
	QRegExp rx ("[A-Za-z][A-Za-z0-9]+");
	nameEdit.lineEdit()->setValidator (new QRegExpValidator (rx, parent));
	QObject::connect(nameEdit.lineEdit(),
					 (void(QLineEdit::*)(const QString&))(&QLineEdit::textChanged),
					 [=] (const QString& text) {
		if (text.isEmpty()) {
			saveButton.setDisabled(true);
		} else if (!checkName(text)) {
			saveButton.setDisabled(true);
		} else {
			if (pagesListWidget.currentRow() >= 0)
				saveButton.setEnabled(true);
			QString tmp = text;
			tmp[0] = text[0].toLower();
			nameEdit.lineEdit()->setText(tmp);
		}
	});

	horizontalLayout.addWidget(&addButton);
	horizontalLayout.addStretch();
	horizontalLayout.addWidget(&removeButton);

	horizontalLayout_2.addWidget(&nameEdit);
	horizontalLayout_2.addWidget(&saveButton);
	horizontalLayout_2.addStretch();

	verticalLayout.addLayout(&horizontalLayout_2);
	verticalLayout.addWidget(&pagesListWidget);
	verticalLayout.addLayout(&horizontalLayout);
	verticalLayout.setContentsMargins(fit(6), 0, fit(8), fit(8));
	parent->setLayout(&verticalLayout);

	pagesListWidget.addItem("page1");
}

const QList<QQuickItem*> PagesWidgetPrivate::GetAllChildren(QQuickItem* const item) const
{
	/* Return all child items of item including item itself */
	QList<QQuickItem*> childList;
	for (auto child : item->childItems())
		childList << GetAllChildren(child);
	childList << item;
	return childList;
}

bool PagesWidgetPrivate::checkName(const QString& name) const
{
	for (int i = 0; i < pagesListWidget.count(); i++) {
		if (pagesListWidget.item(i)->text() == name) {
			return false;
		}
	}
	return true;
}

void PagesWidgetPrivate::removeButtonClicked()
{
	if (pagesListWidget.count() > 1) {
		auto name = pagesListWidget.currentItem()->text();
		QMessageBox msgBox;
		msgBox.setText(QString("<b>This will delete %1 and its content.</b>").arg(name));
		msgBox.setInformativeText("Do you want to continue?");
		msgBox.setStandardButtons(QMessageBox::No | QMessageBox::Yes);
		msgBox.setDefaultButton(QMessageBox::No);
		msgBox.setIcon(QMessageBox::Warning);
		const int ret = msgBox.exec();
		switch (ret) {
			case QMessageBox::Yes: {
				delete pagesListWidget.takeItem(pagesListWidget.currentRow());
				auto v = rootContext->contextProperty(name);
				auto selectedItem = qobject_cast<QQuickItem*>(v.value<QObject*>());
				if (!selectedItem) qFatal("PagesWidget : Error occurred");
				auto items = GetAllChildren(selectedItem);
				for (auto item : items) {
					if (itemList->contains(item)) {
						SaveManager::removeSave(rootContext->nameForObject(item));
						SaveManager::removeParentalRelationship(rootContext->nameForObject(item));
						bindingWidget->detachBindingsFor(item);
						rootContext->setContextProperty(rootContext->nameForObject(item), 0);
						int i = itemList->indexOf(item);
						itemList->removeOne(item);
						urlList->removeAt(i);
					}
				}
				SaveManager::removePageOrder(rootContext->nameForObject(selectedItem));
				rootContext->setContextProperty(rootContext->nameForObject(selectedItem), 0);
				selectedItem->setParentItem(0);
				selectedItem->deleteLater();
				break;
			} default: {
				// Do nothing
				break;
			}
		}
	}
}

void PagesWidgetPrivate::addButtonClicked()
{
	int count = pagesListWidget.count();
	auto name = QString("page%1").arg(count);
	parent->addPageWithoutSave(name);
	SaveManager::addPageOrder(name);
}

void PagesWidgetPrivate::saveButtonClicked()
{
	auto name = pagesListWidget.currentItem()->text();
	auto v = rootContext->contextProperty(name);
	auto selectedItem = qobject_cast<QQuickItem*>(v.value<QObject*>());
	if (!selectedItem) qFatal("PagesWidget : Error occurred");
	rootContext->setContextProperty(name, 0);
	rootContext->setContextProperty(nameEdit.text(), selectedItem);
	pagesListWidget.currentItem()->setText(nameEdit.text());
	SaveManager::changePageOrder(name, nameEdit.text());
	auto items = GetAllChildren(selectedItem);
	for (auto item : items) {
		if (itemList->contains(item)) {
			SaveManager::addParentalRelationship(rootContext->nameForObject(item), nameEdit.text());
		}
	}
}

PagesWidgetPrivate* PagesWidget::m_d = nullptr;

PagesWidget::PagesWidget(QWidget *parent)
	: QWidget(parent)
{
	if (m_d) return;
	m_d = new PagesWidgetPrivate(this);
}

PagesWidget::~PagesWidget()
{
	delete m_d;
}

void PagesWidget::setSwipeItem(QQuickItem* swipeItem)
{
	m_d->swipeItem = swipeItem;
}

void PagesWidget::setRootContext(QQmlContext* context)
{
	m_d->rootContext = context;
}

void PagesWidget::setItemList(QList<QQuickItem*>* items)
{
	m_d->itemList = items;
}

void PagesWidget::setUrlList(QList<QUrl>* items)
{
	m_d->urlList = items;
}

void PagesWidget::setBindingWidget(BindingWidget* bindingWidget)
{
	m_d->bindingWidget = bindingWidget;
}

QList<QQuickItem*> PagesWidget::pages()
{
	QList<QQuickItem*> itemList;
	for (int i = 0; i < m_d->pagesListWidget.count(); i++) {
		auto v = m_d->rootContext->contextProperty(m_d->pagesListWidget.item(i)->text());
		itemList << qobject_cast<QQuickItem*>(v.value<QObject*>());
	}
	return itemList;
}

void PagesWidget::setCurrentPage(int index)
{
	m_d->pagesListWidget.setCurrentRow(index);
}

void PagesWidget::addPageWithoutSave(QString& name)
{
	int count = m_d->pagesListWidget.count();
	for (int i = 0; i < m_d->pagesListWidget.count(); i++) {
		if (m_d->pagesListWidget.item(i)->text() == name) {
			if (name.at(name.size() - 1).isNumber()) {
				name.remove(name.size() - 1, 1);
			}
			i = -1;
			count++;
			name += QString::number(count);
		}
	}
	m_d->pagesListWidget.addItem(name);

	QQmlComponent c(qmlEngine((QObject*)m_d->swipeItem));
	c.setData(QByteArray().insert(0,QString(PAGE_CODE).arg(name)), QUrl());
	auto item = qobject_cast<QQuickItem*>(c.create(qmlContext((QObject*)m_d->swipeItem)));
	if (!item) qFatal("PagesWidget : Error occurred");
	item->setParentItem(m_d->swipeItem);
	m_d->rootContext->setContextProperty(name, item);
}

void PagesWidget::changePageWithoutSave(const QString& from, QString& to)
{
	if (from == to) return;
	int index = -1;
	for (int i = 0; i < m_d->pagesListWidget.count(); i++) {
		if (m_d->pagesListWidget.item(i)->text() == from) {
			index = i;
		}
	}
	if (index < 0) return;
	int count = m_d->pagesListWidget.count();
	for (int i = 0; i < m_d->pagesListWidget.count(); i++) {
		if (m_d->pagesListWidget.item(i)->text() == to) {
			if (to.at(to.size() - 1).isNumber()) {
				to.remove(to.size() - 1, 1);
			}
			i = -1;
			count++;
			to += QString::number(count);
		}
	}
	auto v = m_d->rootContext->contextProperty(from);
	auto selectedItem = qobject_cast<QQuickItem*>(v.value<QObject*>());
	if (!selectedItem) qFatal("PagesWidget : Error occurred");
	m_d->rootContext->setContextProperty(from, 0);
	m_d->rootContext->setContextProperty(to, selectedItem);
	m_d->pagesListWidget.item(index)->setText(to);
}

void PagesWidget::removePageWithoutSave(const QString& name)
{
	if (m_d->pagesListWidget.count() > 1) {
		int index = -1;
		for (int i = m_d->pagesListWidget.count(); i--;) {
			if (m_d->pagesListWidget.item(i)->text() == name) {
				index = i;
			}
		}
		if (index < 0) return;
		delete m_d->pagesListWidget.takeItem(index);
		auto v = m_d->rootContext->contextProperty(name);
		auto selectedItem = qobject_cast<QQuickItem*>(v.value<QObject*>());
		if (!selectedItem) qFatal("PagesWidget : Error occurred");
		auto items = m_d->GetAllChildren(selectedItem);
		for (auto item : items) {
			if (m_d->itemList->contains(item)) {
				m_d->bindingWidget->detachBindingsFor(item);
				m_d->rootContext->setContextProperty(m_d->rootContext->nameForObject(item), 0);
				int i = m_d->itemList->indexOf(item);
				m_d->itemList->removeOne(item);
				m_d->urlList->removeAt(i);
			}
		}
		m_d->rootContext->setContextProperty(m_d->rootContext->nameForObject(selectedItem), 0);
		selectedItem->setParentItem(0);
		selectedItem->deleteLater();
	}
}
