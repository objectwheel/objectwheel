#include <formswidget.h>
#include <flatButton.h>
#include <listwidget.h>
#include <savemanager.h>
#include <formscene.h>
#include <designmanager.h>
#include <filemanager.h>
#include <css.h>
#include <fit.h>
#include <delayer.h>

#include <QStandardPaths>
#include <QVBoxLayout>
#include <QHBoxLayout>

using namespace Fit;

class FormsWidgetPrivate : public QObject
{
        Q_OBJECT

    public:
        FormsWidgetPrivate(FormsWidget* parent);
		bool checkName(const QString& name) const;

	public slots:
		void removeButtonClicked();
		void addButtonClicked();
        void handleDatabaseChange();
        void handleCurrentFormChange();

    public:
        FormsWidget* parent;
        QVBoxLayout* verticalLayout;
        ListWidget* formsListWidget;
        QHBoxLayout* horizontalLayout;
        FlatButton* addButton;
        FlatButton* removeButton;
};

FormsWidgetPrivate::FormsWidgetPrivate(FormsWidget* parent)
    : QObject(parent)
    , parent(parent)
    , verticalLayout(new QVBoxLayout)
    , formsListWidget(new ListWidget)
    , horizontalLayout(new QHBoxLayout)
    , addButton(new FlatButton)
    , removeButton(new FlatButton)
{
    formsListWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    formsListWidget->setFocusPolicy(Qt::NoFocus);
    formsListWidget->setStyleSheet(CSS::FormsListWidget);
    formsListWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    formsListWidget->setDragDropMode(QAbstractItemView::NoDragDrop);
    formsListWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    formsListWidget->setSelectionMode(ListWidget::SingleSelection);
    formsListWidget->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    formsListWidget->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);

    QTimer::singleShot(10000, [=] { //FIXME
        Delayer::delay([]()->bool {if (SaveManager::instance()) return false; else return true;});
        connect(SaveManager::instance(), SIGNAL(projectExposed()), SLOT(handleDatabaseChange()));
        connect(SaveManager::instance(), SIGNAL(databaseChanged()), SLOT(handleDatabaseChange()));
        connect(formsListWidget, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), SLOT(handleCurrentFormChange()));
    });

    addButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    addButton->setColor("#6BB64B");
    addButton->setFixedSize(fit(30),fit(30));
    addButton->setRadius(fit(13));
    addButton->setIconSize(QSize(fit(16),fit(16)));
    addButton->setIcon(QIcon(":/resources/images/plus.png"));
    connect(addButton, SIGNAL(clicked(bool)), SLOT(addButtonClicked()));

    removeButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    removeButton->setColor("#C61717");
    removeButton->setFixedSize(fit(30),fit(30));
    removeButton->setRadius(fit(13));
    removeButton->setIconSize(QSize(fit(16),fit(16)));
    removeButton->setIcon(QIcon(":/resources/images/minus.png"));
    connect(removeButton, SIGNAL(clicked(bool)), SLOT(removeButtonClicked()));

    horizontalLayout->addWidget(addButton);
    horizontalLayout->addStretch();
    horizontalLayout->addWidget(removeButton);

    verticalLayout->addWidget(formsListWidget);
    verticalLayout->addLayout(horizontalLayout);
    verticalLayout->setContentsMargins(fit(6), 0, fit(8), fit(8));
    parent->setLayout(verticalLayout);
}

void FormsWidgetPrivate::removeButtonClicked()
{
    auto form = DesignManager::formScene()->mainForm();
    if (!form || !form->form() || form->main())
        return;
//    SaveManager::removeForm((Form*)form);
    DesignManager::formScene()->removeForm(form);
}

void FormsWidgetPrivate::addButtonClicked()
{
    auto tempPath = QStandardPaths::standardLocations(QStandardPaths::TempLocation)[0];
    tempPath = tempPath + separator() + "Objectwheel";

    rm(tempPath);

    if (!mkdir(tempPath) || !cp(DIR_QRC_FORM, tempPath, true, true))
        return;

    auto form = new Form(tempPath + separator() + DIR_THIS + separator() + "main.qml");
    DesignManager::formScene()->addForm(form);
    connect(form, &Form::initialized, [=] {
        SaveManager::addForm(form);
        form->controlTransaction()->setTransactionsEnabled(true);
        rm(tempPath);
    });
}

void FormsWidgetPrivate::handleDatabaseChange()
{
    int row = 0;
    QString id;
    if (formsListWidget->currentItem())
        id = formsListWidget->currentItem()->text();

    formsListWidget->clear();

    for (auto path : SaveManager::formPaths()) {
        auto _id = SaveManager::id(path);
        if (id == _id)
            row = formsListWidget->count();
        formsListWidget->addItem(_id);
    }
    formsListWidget->setCurrentRow(row);
}

void FormsWidgetPrivate::handleCurrentFormChange()
{
    if (!formsListWidget->currentItem())
        return;

    auto id = formsListWidget->currentItem()->text();
    for (auto form : DesignManager::formScene()->forms())
        if (form->id() == id)
            DesignManager::formScene()->setMainForm(form);
}

FormsWidgetPrivate* FormsWidget::_d = nullptr;

FormsWidget::FormsWidget(QWidget *parent)
	: QWidget(parent)
{
    if (_d)
        return;
    _d = new FormsWidgetPrivate(this);
}

FormsWidget* FormsWidget::instance()
{
    return _d->parent;
}

void FormsWidget::setCurrentForm(int index)
{
    _d->formsListWidget->setCurrentRow(index);
}

#include "formswidget.moc"
