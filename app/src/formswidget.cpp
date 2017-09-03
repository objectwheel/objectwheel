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
    formsListWidget.setStyleSheet(CSS::FormsListWidget);
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


}

void FormsWidgetPrivate::addButtonClicked()
{

}

void FormsWidgetPrivate::saveButtonClicked()
{

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
