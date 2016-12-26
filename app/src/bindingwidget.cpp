#include <bindingwidget.h>
#include <listwidget.h>
#include <flatbutton.h>
#include <fit.h>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <lineedit.h>
#include <css.h>
#include <QPropertyAnimation>
#include <QEasingCurve>
#include <QTimer>
#include <combobox.h>
#include <QComboBox>
#include <QQuickItem>
#include <QQmlEngine>
#include <QQmlContext>
#include <QQmlProperty>
#include <QPair>
#include <QLineEdit>
#include <QLabel>
#include <QScrollArea>

using namespace Fit;

class BindingWidgetPrivate
{
	public:
		BindingWidget* parent;
		const QList<QQuickItem*>* items;
		QVBoxLayout verticalLayout;
		QHBoxLayout horizontalLayout;
		FlatButton addButton;
		FlatButton removeButton;
		ListWidget bindingListWidget;

		QWidget popupWidget;
		QVBoxLayout popupVLayout;
		FlatButton popupHideButton;
		LineEdit nameEdit;
		QLabel popupTitle;
		LineEdit popupItemNameTextBox;
		QQuickItem* targetItem;
		QScrollArea popupScrollArea;
		QWidget popupScrollAreaWidget;
		QVBoxLayout popupScrollAreaVLayout;
		QWidget popupScrollAreaEmptyWidget;
		ComboBox targetPropertyCombobox;
		ComboBox sourceItemCombobox;
		ComboBox sourcePropertyCombobox;
		QQuickItem* lastTargetItem;
		FlatButton popupOkButton;
		typedef QPair<QObject*, QString> BindingPair;
		typedef QPair<QMetaObject::Connection, BindingPair> ConnectionPair;
		QMap<BindingPair, BindingPair> bindings;
		QMap<QString, ConnectionPair> connections;

	public:
		BindingWidgetPrivate(BindingWidget* parent);

	private slots:
		void removeButtonClicked();
		void addButtonClicked();
		void popupHideButtonClicked();
		void popupOkButtonClicked();
		void ensureComboboxVisible(const QObject* obj);
};

BindingWidgetPrivate::BindingWidgetPrivate(BindingWidget* p)
	: parent(p)
	, items(nullptr)
{
	bindingListWidget.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	bindingListWidget.setFocusPolicy(Qt::NoFocus);
	bindingListWidget.setStyleSheet(CSS::BindingListWidget);
	bindingListWidget.setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	bindingListWidget.setDragDropMode(QAbstractItemView::NoDragDrop);
	bindingListWidget.setSelectionBehavior(QAbstractItemView::SelectRows);
	bindingListWidget.setSelectionMode(ListWidget::SingleSelection);
	bindingListWidget.setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
	bindingListWidget.setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
	QObject::connect(&bindingListWidget,(void(ListWidget::*)(int))(&ListWidget::currentRowChanged),[=](int i){removeButton.setEnabled(i>=0);});

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

	horizontalLayout.addWidget(&addButton);
	horizontalLayout.addWidget(&removeButton);
	horizontalLayout.addStretch();

	verticalLayout.addWidget(&bindingListWidget);
	verticalLayout.addLayout(&horizontalLayout);
	verticalLayout.setContentsMargins(fit(6), 0, fit(8), fit(8));



	popupWidget.setParent(parent);
	popupWidget.setObjectName("popupWindow");
	popupWidget.setStyleSheet("#popupWindow{background:#2b5796;border-top:1px solid #555555;}");
	popupWidget.setHidden(true);
	popupWidget.setLayout(&popupVLayout);

	popupHideButton.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	popupHideButton.setColor("#c03638");
	popupHideButton.setFixedSize(fit(20),fit(16));
	popupHideButton.setRadius(fit(2));
	popupHideButton.setIconSize(QSize(fit(10),fit(10)));
	popupHideButton.setIcon(QIcon(":/resources/images/minus.png"));
	popupHideButton.setParent(&popupWidget);
	QObject::connect(&popupHideButton, (void(FlatButton::*)(bool))(&FlatButton::clicked), [=] {popupHideButtonClicked();});

	nameEdit.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	nameEdit.setFixedHeight(fit(30));
	nameEdit.setIcon(QIcon("/home/kozmon/Masaüstü/advanced.png"));
	nameEdit.setPlaceholderText("Binding name");
	nameEdit.show();

	targetPropertyCombobox.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	targetPropertyCombobox.setFixedHeight(fit(40));
	targetPropertyCombobox.setIcon(QIcon(":/resources/images/plug-up.png"));
	targetPropertyCombobox.setColor("#1e8145");
	targetPropertyCombobox.setPlaceHolderText("Target property");
	targetPropertyCombobox.show();
	QObject::connect(&targetPropertyCombobox, &ComboBox::popupShowed, [=] {ensureComboboxVisible(&targetPropertyCombobox);});

	sourceItemCombobox.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	sourceItemCombobox.setFixedHeight(fit(40));
	sourceItemCombobox.setIcon(QIcon(":/resources/images/down-arrow.png"));
	sourceItemCombobox.setColor("#1e8145");
	sourceItemCombobox.setPlaceHolderText("Source item");
	sourceItemCombobox.show();
	QObject::connect(&sourceItemCombobox, &ComboBox::popupShowed, [=] {ensureComboboxVisible(&sourceItemCombobox);});

	QObject::connect(&sourceItemCombobox,(void(ComboBox::*)(const QString&))(&ComboBox::currentTextChanged),
					 [=](const QString& text){
		sourcePropertyCombobox.clear();
		QQuickItem* sourceItem = nullptr;
		for (auto item : *items) {
			auto itemName = qmlContext(item)->nameForObject(item);
			if (itemName == text) {
				sourceItem = item;
			}
		}
		if (!sourceItem) return;
		if (!lastTargetItem) return;
		auto targetItemPropertyType = lastTargetItem->property(targetPropertyCombobox.currentItem().toStdString().c_str()).type();
		auto metaObject = sourceItem->metaObject();
		for (int i = 0; i < metaObject->propertyCount(); i++) {
			if (metaObject->property(i).isReadable() &&
				metaObject->property(i).hasNotifySignal() &&
				!QString(metaObject->property(i).name()).startsWith("__") &&
				metaObject->property(i).type() == targetItemPropertyType)
				sourcePropertyCombobox.addItem(metaObject->property(i).name());
		}
	});

	sourcePropertyCombobox.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	sourcePropertyCombobox.setFixedHeight(fit(40));
	sourcePropertyCombobox.setIcon(QIcon(":/resources/images/plug-down.png"));
	sourcePropertyCombobox.setColor("#1e8145");
	sourcePropertyCombobox.setPlaceHolderText("Source property");
	sourcePropertyCombobox.show();
	QObject::connect(&sourcePropertyCombobox, &ComboBox::popupShowed, [=] {ensureComboboxVisible(&sourcePropertyCombobox);});

	popupOkButton.setColor(QColor("#1e8145"));
	popupOkButton.setTextColor(Qt::white);
	popupOkButton.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	popupOkButton.setFixedHeight(fit(30));
	popupOkButton.setText("OK");
	QObject::connect(&popupOkButton, (void(FlatButton::*)(bool))(&FlatButton::clicked), [=] {popupOkButtonClicked();});

	popupTitle.setText("Binding Editor");
	popupTitle.setStyleSheet("background:transparent;color:white;font:bold;text-decoration:underline;");
	popupItemNameTextBox.setText("Target item");
	popupItemNameTextBox.lineEdit()->setReadOnly(true);
	popupItemNameTextBox.setIcon(QIcon(":/resources/images/item.png"));

	popupScrollArea.setWidgetResizable(true);
	popupScrollArea.setWidget(&popupScrollAreaWidget);
	popupScrollArea.widget()->setLayout(&popupScrollAreaVLayout);
	popupScrollArea.widget()->setStyleSheet("background:transparent;");
	popupScrollArea.setStyleSheet("background:transparent;");
	popupScrollArea.setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	popupScrollArea.setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	popupScrollArea.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	popupScrollAreaEmptyWidget.setFixedHeight(fit(10));
	popupScrollAreaEmptyWidget.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	popupScrollAreaVLayout.setSpacing(fit(5));
	popupScrollAreaVLayout.setContentsMargins(0,0,0,0);
	popupScrollAreaVLayout.addWidget(&targetPropertyCombobox);
	popupScrollAreaVLayout.addWidget(&sourceItemCombobox);
	popupScrollAreaVLayout.addWidget(&sourcePropertyCombobox);
	popupScrollAreaVLayout.addWidget(&popupScrollAreaEmptyWidget);
	popupScrollAreaVLayout.addStretch();

	popupVLayout.addWidget(&popupTitle);
	popupVLayout.addSpacing(fit(10));
	popupVLayout.addWidget(&popupItemNameTextBox);
	popupVLayout.addSpacing(fit(10));
	popupVLayout.addWidget(&nameEdit);
	popupVLayout.addSpacing(fit(10));
	popupVLayout.addWidget(&popupScrollArea);
	popupVLayout.addWidget(&popupOkButton);
}

void BindingWidgetPrivate::removeButtonClicked()
{
	auto connectionName = bindingListWidget.currentItem()->text();
	QObject::disconnect(connections[connectionName].first);
	bindings.remove(connections[connectionName].second);
	bindingListWidget.takeItem(bindingListWidget.currentRow());
}

void BindingWidgetPrivate::addButtonClicked()
{
	auto animation = new QPropertyAnimation(&popupWidget, "geometry");
	animation->setDuration(500);
	animation->setStartValue(QRect(0, parent->height(), parent->width(), parent->height()));
	animation->setEndValue(QRect(0, 0, parent->width(), parent->height()));
	animation->setEasingCurve(QEasingCurve::OutExpo);
	QObject::connect(animation, SIGNAL(finished()), animation, SLOT(deleteLater()));
	animation->start();
	popupWidget.show();
	popupWidget.raise();
	popupHideButton.raise();
	popupHideButton.move(popupWidget.width()-fit(30), fit(10));
}

void BindingWidgetPrivate::popupHideButtonClicked()
{
	auto animation = new QPropertyAnimation(&popupWidget, "geometry");
	animation->setDuration(500);
	animation->setEndValue(QRect(0, parent->height(), parent->width(), parent->height()));
	animation->setStartValue(QRect(0, 0, parent->width(), parent->height()));
	animation->setEasingCurve(QEasingCurve::OutExpo);
	QObject::connect(animation, SIGNAL(finished()), &popupWidget, SLOT(hide()));
	QObject::connect(animation, SIGNAL(finished()), animation, SLOT(deleteLater()));
	animation->start();
}

void BindingWidgetPrivate::popupOkButtonClicked()
{
	if (!targetPropertyCombobox.currentItem().isEmpty() &&
		!sourceItemCombobox.currentItem().isEmpty() &&
		!sourcePropertyCombobox.currentItem().isEmpty()) {

		QQuickItem* sourceItem = nullptr;
		for (auto item : *items) {
			auto itemName = qmlContext(item)->nameForObject(item);
			if (itemName == sourceItemCombobox.currentItem()) {
				sourceItem = item;
			}
		}
		if (!sourceItem) return;
		if (!lastTargetItem) return;

		QMetaMethod sourceSign;
		for (int i = 0; i < sourceItem->metaObject()->propertyCount(); i++)
			if (QString(sourceItem->metaObject()->property(i).name()) == sourcePropertyCombobox.currentItem())
				sourceSign = sourceItem->metaObject()->property(i).notifySignal();
		if (!sourceSign.isValid()) return;

		BindingPair sourceBindingPair;
		sourceBindingPair.first = sourceItem; //Source item
		sourceBindingPair.second = sourcePropertyCombobox.currentItem(); //Source property

		BindingPair targetBindingPair;
		targetBindingPair.first = lastTargetItem; //Target item
		targetBindingPair.second = targetPropertyCombobox.currentItem(); //Target property

		bindings.insert(sourceBindingPair, targetBindingPair);
		auto connection = QObject::connect(sourceItem, sourceSign, parent, parent->metaObject()->method(parent->metaObject()->indexOfSlot("processBindings()")));

		auto connectionName = nameEdit.text();
		if (connectionName.isEmpty()) {
			connectionName = QString("Connection %1").arg(bindingListWidget.count());
		}

		for (int i = 0; i < bindingListWidget.count(); i++) {
			if (bindingListWidget.item(i)->text() == connectionName) {
				connectionName+="+";
				i = 0;
			}
		}

		ConnectionPair connectionPair;
		connectionPair.first = connection;
		connectionPair.second =sourceBindingPair;
		connections.insert(connectionName, connectionPair);
		bindingListWidget.addItem(connectionName);
		popupHideButtonClicked();
	}
}

void BindingWidgetPrivate::ensureComboboxVisible(const QObject* obj)
{
	popupScrollArea.ensureWidgetVisible((QWidget*)obj);
}

BindingWidget::BindingWidget(QWidget *parent)
	: QWidget(parent)
	, m_d(new BindingWidgetPrivate(this))
{
	setLayout(&m_d->verticalLayout);
	setDisabled(true);
}

BindingWidget::~BindingWidget()
{
	delete m_d;
}

void BindingWidget::clearList()
{
	m_d->targetPropertyCombobox.clear();
	m_d->sourceItemCombobox.clear();
	m_d->sourcePropertyCombobox.clear();
	m_d->popupItemNameTextBox.setText("Target item");
}

void BindingWidget::refreshList(QObject* const selectedItem)
{
	m_d->lastTargetItem = qobject_cast<QQuickItem*>(selectedItem);
	m_d->targetPropertyCombobox.clear();
	m_d->targetItem = qobject_cast<QQuickItem*>(selectedItem);
	m_d->popupItemNameTextBox.setText(qmlContext(m_d->targetItem)->nameForObject(m_d->targetItem));
	auto metaObject = selectedItem->metaObject();
	for (int i = 0; i < metaObject->propertyCount(); i++) {
		if (metaObject->property(i).isWritable() && !QString(metaObject->property(i).name()).startsWith("__"))
			m_d->targetPropertyCombobox.addItem(metaObject->property(i).name());
	}

	m_d->sourceItemCombobox.clear();
	for (auto item : *m_d->items) {
		if (item != m_d->lastTargetItem)
			m_d->sourceItemCombobox.addItem(qmlContext(item)->nameForObject(item));
	}
	setEnabled(true);
}

const QList<QQuickItem*>* BindingWidget::items() const
{
	return m_d->items;
}

void BindingWidget::setItems(const QList<QQuickItem*>* const items)
{
	m_d->items = items;
}

void BindingWidget::showBar()
{
	m_d->bindingListWidget.showBar();
}

void BindingWidget::processBindings()
{
	auto sourceSignalIndex = senderSignalIndex();
	auto sourceItem = sender();
	if (!sourceItem || sourceSignalIndex < 0) { qWarning("Binding error! 0x00"); return; }

	QString property;
	for (int i = 0; i < sourceItem->metaObject()->propertyCount(); i++) {
		if (sourceSignalIndex == sourceItem->metaObject()->property(i).notifySignalIndex())
			property = sourceItem->metaObject()->property(i).name();
	}
	if (property.isEmpty()) { qWarning("Binding error! 0x01"); return; }

	BindingWidgetPrivate::BindingPair sourceBindingPair;
	sourceBindingPair.first = sourceItem;
	sourceBindingPair.second = property;

	auto targetBindingPair = m_d->bindings[sourceBindingPair];
	qDebug() << m_d->bindings;
	if (!targetBindingPair.first || targetBindingPair.second.isEmpty())  { qWarning("Binding error! 0x02"); return; }

	auto value = sourceItem->property(property.toStdString().c_str());
	targetBindingPair.first->setProperty(targetBindingPair.second.toStdString().c_str(), value);
}
