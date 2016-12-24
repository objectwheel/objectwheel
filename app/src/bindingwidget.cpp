#include <bindingwidget.h>
#include <listwidget.h>
#include <flatbutton.h>
#include <fit.h>
#include <QtQml/private/qqmlbind_p.h>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <lineedit.h>
#include <css.h>
#include <QPropertyAnimation>
#include <QEasingCurve>
#include <QTimer>

using namespace Fit;

class BindingWidgetPrivate
{
	public:
		QWidget* parent;
		const BindingWidget::QQuickItemList* items;
		QVBoxLayout verticalLayout;
		QHBoxLayout horizontalLayout;
		FlatButton addButton;
		FlatButton removeButton;
		ListWidget bindingListWidget;

		QWidget popupWidget;
		QVBoxLayout popupVLayout;
		FlatButton popupHideButton;
		LineEdit nameEdit;

	public:
		BindingWidgetPrivate(QWidget* parent);

	private slots:
		void removeButtonClicked();
		void addButtonClicked();
		void popupHideButtonClicked();

};

BindingWidgetPrivate::BindingWidgetPrivate(QWidget* p)
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

	nameEdit.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	nameEdit.setFixedHeight(fit(30));
	nameEdit.show();
	popupVLayout.addWidget(&nameEdit);

	popupHideButton.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	popupHideButton.setColor("#555555");
	popupHideButton.setFixedSize(fit(20),fit(12));
	popupHideButton.setRadius(fit(2));
	popupHideButton.setIconSize(QSize(fit(10),fit(10)));
	popupHideButton.setIcon(QIcon(":/resources/images/minus.png"));
	popupHideButton.setParent(&popupWidget);
	QObject::connect(&popupHideButton, (void(FlatButton::*)(bool))(&FlatButton::clicked), [=] {popupHideButtonClicked();});

	bindingListWidget.addItem("Conenction 1");
	bindingListWidget.addItem("Conenction 2");
	bindingListWidget.addItem("Conenction 3");

}

void BindingWidgetPrivate::removeButtonClicked()
{

}

void BindingWidgetPrivate::addButtonClicked()
{
	auto animation = new QPropertyAnimation(&popupWidget, "geometry");
	animation->setDuration(500);
	animation->setStartValue(QRect(0, parent->height(), parent->width(), fit(190)));
	animation->setEndValue(QRect(0, parent->height()-fit(190), parent->width(), fit(190)));
	animation->setEasingCurve(QEasingCurve::OutExpo);
	QObject::connect(animation, SIGNAL(finished()), animation, SLOT(deleteLater()));
	animation->start();
	popupWidget.show();
	popupWidget.raise();
	popupHideButton.move(popupWidget.width()-fit(30), fit(10));

}

void BindingWidgetPrivate::popupHideButtonClicked()
{
	auto animation = new QPropertyAnimation(&popupWidget, "geometry");
	animation->setDuration(500);
	animation->setEndValue(QRect(0, parent->height(), parent->width(), fit(190)));
	animation->setStartValue(QRect(0, parent->height()-fit(190), parent->width(), fit(190)));
	animation->setEasingCurve(QEasingCurve::OutExpo);
	QObject::connect(animation, SIGNAL(finished()), &popupWidget, SLOT(hide()));
	QObject::connect(animation, SIGNAL(finished()), animation, SLOT(deleteLater()));
	animation->start();
}

BindingWidget::BindingWidget(QWidget *parent)
	: QWidget(parent)
	, m_d(new BindingWidgetPrivate(this))
{
	setLayout(&m_d->verticalLayout);
}

BindingWidget::~BindingWidget()
{
	delete m_d;
}

void BindingWidget::clearList()
{
	m_d->bindingListWidget.clear();
}

void BindingWidget::refreshList(QObject* const)
{

}

const BindingWidget::QQuickItemList* BindingWidget::items() const
{
	return m_d->items;
}

void BindingWidget::setItems(const QQuickItemList* const items)
{
	m_d->items = items;
}

void BindingWidget::showBar()
{
	m_d->bindingListWidget.showBar();
}
