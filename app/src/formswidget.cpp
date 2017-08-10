#include <formswidget.h>
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
#include <eventswidget.h>
#include <formscene.h>

//TODO: Fix this

using namespace Fit;

#define FORM_CODE \
    "import QtQuick 2.0\n"\
    "import QtQuick.Window 2.0\n\n"\
    "Window {\n"\
    "    id:%1\n"\
    "}"

class FormsWidgetPrivate
{
    public:
        FormsWidget* parent;
		QVBoxLayout verticalLayout;
		QHBoxLayout horizontalLayout;
		QHBoxLayout horizontalLayout_2;
		FlatButton addButton;
		FlatButton saveButton;
		FlatButton removeButton;
        ListWidget formsListWidget;
		LineEdit nameEdit;
        FormsWidgetPrivate(FormsWidget* p);
		bool checkName(const QString& name) const;

	public slots:
		void removeButtonClicked();
		void addButtonClicked();
		void saveButtonClicked();
};

FormsWidgetPrivate::FormsWidgetPrivate(FormsWidget* p)
	: parent(p)
{
    formsListWidget.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    formsListWidget.setFocusPolicy(Qt::NoFocus);
    formsListWidget.setStyleSheet(CSS::BindingListWidget);
    formsListWidget.setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    formsListWidget.setDragDropMode(QAbstractItemView::NoDragDrop);
    formsListWidget.setSelectionBehavior(QAbstractItemView::SelectRows);
    formsListWidget.setSelectionMode(ListWidget::SingleSelection);
    formsListWidget.setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    formsListWidget.setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    QObject::connect(&formsListWidget,(void(ListWidget::*)(int))(&ListWidget::currentRowChanged),[=](int i){removeButton.setEnabled(i>=0);});
    QObject::connect(&formsListWidget,(void(ListWidget::*)(int))(&ListWidget::currentRowChanged),[=](int i)
	{
		if (i>=0) {
			saveButton.setEnabled(i>=0);
            nameEdit.setText(formsListWidget.currentItem()->text());
        }
	});

	addButton.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	addButton.setColor("#6BB64B");
	addButton.setFixedSize(fit(30),fit(30));
    addButton.setRadius(fit(13));
	addButton.setIconSize(QSize(fit(16),fit(16)));
	addButton.setIcon(QIcon(":/resources/images/plus.png"));
	QObject::connect(&addButton, (void(FlatButton::*)(bool))(&FlatButton::clicked), [=] {addButtonClicked();});

	removeButton.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	removeButton.setColor("#C61717");
	removeButton.setFixedSize(fit(30),fit(30));
    removeButton.setRadius(fit(13));
	removeButton.setIconSize(QSize(fit(16),fit(16)));
	removeButton.setIcon(QIcon(":/resources/images/minus.png"));
	removeButton.setDisabled(true);
	QObject::connect(&removeButton, (void(FlatButton::*)(bool))(&FlatButton::clicked), [=] {removeButtonClicked();});

	saveButton.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	saveButton.setColor("#0D74C8");
	saveButton.setFixedSize(fit(30),fit(30));
    saveButton.setRadius(fit(13));
	saveButton.setIconSize(QSize(fit(16),fit(16)));
	saveButton.setIcon(QIcon(":/resources/images/save-icon.png"));
	saveButton.setDisabled(true);
	QObject::connect(&saveButton, (void(FlatButton::*)(bool))(&FlatButton::clicked), [=] {saveButtonClicked();});

	nameEdit.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	nameEdit.setFixedHeight(fit(30));
	nameEdit.setIcon(QIcon(":/resources/images/advanced.png"));
    nameEdit.setPlaceholderText("Form id");
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
            if (formsListWidget.currentRow() >= 0)
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
    verticalLayout.addWidget(&formsListWidget);
	verticalLayout.addLayout(&horizontalLayout);
	verticalLayout.setContentsMargins(fit(6), 0, fit(8), fit(8));
	parent->setLayout(&verticalLayout);

    formsListWidget.addItem("form1");
}

bool FormsWidgetPrivate::checkName(const QString& name) const
{
    for (int i = 0; i < formsListWidget.count(); i++) {
        if (formsListWidget.item(i)->text() == name) {
			return false;
		}
	}
	return true;
}

void FormsWidgetPrivate::removeButtonClicked()
{
//    auto bindingName = bindingListWidget.currentItem()->text();
//    QMessageBox msgBox;
//    msgBox.setText(QString("<b>This will delete %1 named binding.</b>").arg(bindingName));
//    msgBox.setInformativeText("Do you want to continue?");
//    msgBox.setStandardButtons(QMessageBox::No | QMessageBox::Yes);
//    msgBox.setDefaultButton(QMessageBox::No);
//    msgBox.setIcon(QMessageBox::Warning);
//    const int ret = msgBox.exec();
//    switch (ret) {
//        case QMessageBox::Yes: {
//            SaveManager::removeBindingSave(bindingName);
//            delete bindingListWidget.takeItem(bindingListWidget.currentRow());
//            break;
//        } default: {
//            // Do nothing
//            break;
//        }
//    }


//    auto formName = formsListWidget.currentItem()->text();
//    for (auto form : FormScene::forms())
//        if (form->id() == formName && form->isMain())
//            return;

//    QMessageBox msgBox;
//    msgBox.setText(QString("<b>This will delete %1 and its content.</b>").arg(formName));
//    msgBox.setInformativeText("Do you want to continue?");
//    msgBox.setStandardButtons(QMessageBox::No | QMessageBox::Yes);
//    msgBox.setDefaultButton(QMessageBox::No);
//    msgBox.setIcon(QMessageBox::Warning);
//    const int ret = msgBox.exec();
//    switch (ret) {
//        case QMessageBox::Yes: {
//            delete formsListWidget.takeItem(formsListWidget.currentRow());
//            auto v = rootContext->contextProperty(formName);
//            auto selectedItem = qobject_cast<QQuickItem*>(v.value<QObject*>());
//            if (!selectedItem) qFatal("FormsWidget : Error occurred");
//            auto items = GetAllChildren(selectedItem);
//            for (auto item : items) {
//                if (itemList->contains(item)) {
//                    SaveManager::removeSave(rootContext->nameForObject(item));
//                    SaveManager::removeParentalRelationship(rootContext->nameForObject(item));
//                    //						bindingWidget->detachBindingsFor(item);
//                    //                        eventWidget->detachEventsFor(item);
//                    rootContext->setContextProperty(rootContext->nameForObject(item), 0);
//                    int i = itemList->indexOf(item);
//                    itemList->removeOne(item);
//                    urlList->removeAt(i);
//                }
//            }
//            SaveManager::removeFormOrder(rootContext->nameForObject(selectedItem));
//            rootContext->setContextProperty(rootContext->nameForObject(selectedItem), 0);
//            selectedItem->setParentItem(0);
//            selectedItem->deleteLater();
//            break;
//        } default: {
//            // Do nothing
//            break;
//        }
//    }
}

void FormsWidgetPrivate::addButtonClicked()
{
    int count = formsListWidget.count();
    auto name = QString("form%1").arg(count);
    parent->addFormWithoutSave(name);
    SaveManager::addFormOrder(name);
}

void FormsWidgetPrivate::saveButtonClicked()
{
//	auto name = formsListWidget.currentItem()->text();
//	auto v = rootContext->contextProperty(name);
//	auto selectedItem = qobject_cast<QQuickItem*>(v.value<QObject*>());
//	if (!selectedItem) qFatal("FormsWidget : Error occurred");
//	rootContext->setContextProperty(name, 0);
//	rootContext->setContextProperty(nameEdit.text(), selectedItem);
//	formsListWidget.currentItem()->setText(nameEdit.text());
//	SaveManager::changeFormOrder(name, nameEdit.text());
//	auto items = GetAllChildren(selectedItem);
//	for (auto item : items) {
//		if (itemList->contains(item)) {
//			SaveManager::addParentalRelationship(rootContext->nameForObject(item), nameEdit.text());
//		}
//	}
}

FormsWidgetPrivate* FormsWidget::m_d = nullptr;

FormsWidget::FormsWidget(QWidget *parent)
	: QWidget(parent)
{
	if (m_d) return;
    m_d = new FormsWidgetPrivate(this);
}

FormsWidget* FormsWidget::instance()
{
    return m_d->parent;
}

FormsWidget::~FormsWidget()
{
	delete m_d;
}

void FormsWidget::setCurrentForm(int index)
{
    m_d->formsListWidget.setCurrentRow(index);
}

void FormsWidget::addFormWithoutSave(QString& name)
{
//	int count = m_d->formsListWidget.count();
//	for (int i = 0; i < m_d->formsListWidget.count(); i++) {
//		if (m_d->formsListWidget.item(i)->text() == name) {
//			if (name.at(name.size() - 1).isNumber()) {
//				name.remove(name.size() - 1, 1);
//			}
//			i = -1;
//			count++;
//			name += QString::number(count);
//		}
//	}
//	m_d->formsListWidget.addItem(name);

//	QQmlComponent c(qmlEngine((QObject*)m_d->swipeItem));
//	c.setData(QByteArray().insert(0,QString(FORM_CODE).arg(name)), QUrl());
//	auto item = qobject_cast<QQuickItem*>(c.create(qmlContext((QObject*)m_d->swipeItem)));
//	if (!item) qFatal("FormsWidget : Error occurred");
//	item->setParentItem(m_d->swipeItem);
//	m_d->rootContext->setContextProperty(name, item);
}

void FormsWidget::changeFormWithoutSave(const QString& from, QString& to)
{
//	if (from == to) return;
//	int index = -1;
//	for (int i = 0; i < m_d->formsListWidget.count(); i++) {
//		if (m_d->formsListWidget.item(i)->text() == from) {
//			index = i;
//		}
//	}
//	if (index < 0) return;
//	int count = m_d->formsListWidget.count();
//	for (int i = 0; i < m_d->formsListWidget.count(); i++) {
//		if (m_d->formsListWidget.item(i)->text() == to) {
//			if (to.at(to.size() - 1).isNumber()) {
//				to.remove(to.size() - 1, 1);
//			}
//			i = -1;
//			count++;
//			to += QString::number(count);
//		}
//	}
//	auto v = m_d->rootContext->contextProperty(from);
//	auto selectedItem = qobject_cast<QQuickItem*>(v.value<QObject*>());
//	if (!selectedItem) qFatal("FormsWidget : Error occurred");
//	m_d->rootContext->setContextProperty(from, 0);
//	m_d->rootContext->setContextProperty(to, selectedItem);
//	m_d->formsListWidget.item(index)->setText(to);
}

void FormsWidget::removeFormWithoutSave(const QString& name)
{
//	if (m_d->formsListWidget.count() > 1) {
//		int index = -1;
//		for (int i = m_d->formsListWidget.count(); i--;) {
//			if (m_d->formsListWidget.item(i)->text() == name) {
//				index = i;
//			}
//		}
//		if (index < 0) return;
//		delete m_d->formsListWidget.takeItem(index);
//		auto v = m_d->rootContext->contextProperty(name);
//		auto selectedItem = qobject_cast<QQuickItem*>(v.value<QObject*>());
//		if (!selectedItem) qFatal("FormsWidget : Error occurred");
//		auto items = m_d->GetAllChildren(selectedItem);
//		for (auto item : items) {
//			if (m_d->itemList->contains(item)) {
////				m_d->bindingWidget->detachBindingsFor(item);
////                m_d->eventWidget->detachEventsFor(item);
//				m_d->rootContext->setContextProperty(m_d->rootContext->nameForObject(item), 0);
//				int i = m_d->itemList->indexOf(item);
//				m_d->itemList->removeOne(item);
//				m_d->urlList->removeAt(i);
//			}
//		}
//		m_d->rootContext->setContextProperty(m_d->rootContext->nameForObject(selectedItem), 0);
//		selectedItem->setParentItem(0);
//		selectedItem->deleteLater();
//	}
}
