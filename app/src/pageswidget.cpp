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

	private slots:
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
	addButton.setRadius(fit(15));
	addButton.setIconSize(QSize(fit(16),fit(16)));
	addButton.setIcon(QIcon(":/resources/images/plus.png"));
	QObject::connect(&addButton, (void(FlatButton::*)(bool))(&FlatButton::clicked), [=] {addButtonClicked();});

	removeButton.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	removeButton.setColor("#c03638");
	removeButton.setFixedSize(fit(30),fit(30));
	removeButton.setRadius(fit(15));
	removeButton.setIconSize(QSize(fit(16),fit(16)));
	removeButton.setIcon(QIcon(":/resources/images/minus.png"));
	removeButton.setDisabled(true);
	QObject::connect(&removeButton, (void(FlatButton::*)(bool))(&FlatButton::clicked), [=] {removeButtonClicked();});

	saveButton.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	saveButton.setColor("#2b5796");
	saveButton.setFixedSize(fit(30),fit(30));
	saveButton.setRadius(fit(15));
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
				Q_ASSERT(selectedItem);
				auto items = GetAllChildren(selectedItem);
				for (auto item : items) {
					if (itemList->contains(item)) {
						rootContext->setContextProperty(rootContext->nameForObject(item), 0);
						int i = itemList->indexOf(item);
						itemList->removeOne(item);
						urlList->removeAt(i);
					}
				}
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
	for (int i = 0; i < pagesListWidget.count(); i++) {
		if (pagesListWidget.item(i)->text() == name) {
			name.remove(name.size() - 1, 1);
			i = 0;
			count++;
			name += QString::number(count);
		}
	}
	pagesListWidget.addItem(name);

	QQmlComponent c(qmlEngine((QObject*)swipeItem));
	c.setData(QByteArray().insert(0,QString(PAGE_CODE).arg(name)), QUrl());
	auto item = qobject_cast<QQuickItem*>(c.create(qmlContext((QObject*)swipeItem)));
	Q_ASSERT(item);
	item->setParentItem(swipeItem);
	rootContext->setContextProperty(name, item);
}

void PagesWidgetPrivate::saveButtonClicked()
{
	auto name = pagesListWidget.currentItem()->text();
	auto v = rootContext->contextProperty(name);
	auto selectedItem = qobject_cast<QQuickItem*>(v.value<QObject*>());
	Q_ASSERT(selectedItem);
	rootContext->setContextProperty(name, 0);
	rootContext->setContextProperty(nameEdit.text(), selectedItem);
	pagesListWidget.currentItem()->setText(nameEdit.text());
}

PagesWidget::PagesWidget(QWidget *parent)
	: QWidget(parent)
	, m_d(new PagesWidgetPrivate(this))
{
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

void PagesWidget::setCurrentPage(int index)
{
	m_d->pagesListWidget.setCurrentRow(index);
}


