#include <formspane.h>
#include <flatbutton.h>
#include <toolboxtree.h>
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
#include <QPalette>
#include <QListWidget>
#include <QScrollBar>

FormsPane::FormsPane(QWidget *parent) : QWidget(parent)
{
    _layout = new QVBoxLayout;
    _listWidget = new QListWidget;
    _buttonLayout = new QHBoxLayout;
    _addButton = new FlatButton;
    _removeButton = new FlatButton;

    QPalette p(palette());
    p.setColor(backgroundRole(), "#E0E4E7");
    setAutoFillBackground(true);
    setPalette(p);

    QPalette p2(_listWidget->palette());
    p2.setColor(QPalette::Base, "#F3F7FA");
    p2.setColor(QPalette::Window, "#F3F7FA");
    p2.setColor(QPalette::Highlight, "#d0d4d7");
    p2.setColor(QPalette::Text, "#202427");
    _listWidget->setPalette(p2);

    _listWidget->setFocusPolicy(Qt::NoFocus);
    _listWidget->setIconSize(QSize(fit::fx(14),fit::fx(14)));
    _listWidget->setSelectionMode(QListWidget::SingleSelection);
    _listWidget->setDragDropMode(QAbstractItemView::NoDragDrop);
    _listWidget->verticalScrollBar()->setStyleSheet(CSS::ScrollBar);
    _listWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    _listWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    _listWidget->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    _listWidget->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    _listWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QTimer::singleShot(1000, [=] { //FIXME
        Delayer::delay([]()->bool {if (SaveManager::instance()) return false; else return true;});
        connect(SaveManager::instance(), SIGNAL(projectExposed()), SLOT(handleDatabaseChange()));
        connect(SaveManager::instance(), SIGNAL(databaseChanged()), SLOT(handleDatabaseChange()));
        connect(_listWidget, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), SLOT(handleCurrentFormChange()));
    });

    _addButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    _addButton->setColor("#6BB64B");
    _addButton->setFixedSize(fit::fx(17),fit::fx(17));
    _addButton->setRadius(fit::fx(7.5));
    _addButton->setIconSize(QSize(fit::fx(11),fit::fx(11)));
    _addButton->setIcon(QIcon(":/resources/images/plus.png"));
    connect(_addButton, SIGNAL(clicked(bool)), SLOT(addButtonClicked()));

    _removeButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    _removeButton->setColor("#C61717");
    _removeButton->setFixedSize(fit::fx(17),fit::fx(17));
    _removeButton->setRadius(fit::fx(7.5));
    _removeButton->setIconSize(QSize(fit::fx(11),fit::fx(11)));
    _removeButton->setIcon(QIcon(":/resources/images/minus.png"));
    connect(_removeButton, SIGNAL(clicked(bool)), SLOT(removeButtonClicked()));

    _buttonLayout->addWidget(_addButton);
    _buttonLayout->addStretch();
    _buttonLayout->addWidget(_removeButton);

    _layout->addWidget(_listWidget);
    _layout->addLayout(_buttonLayout);
    _layout->setSpacing(fit::fx(2));
    _layout->setContentsMargins(fit::fx(3), fit::fx(3), fit::fx(3), fit::fx(3));
    setLayout(_layout);
}

void FormsPane::removeButtonClicked()
{
    auto form = DesignManager::formScene()->mainForm();
    if (!form || !form->form() || form->main())
        return;
    SaveManager::removeForm((Form*)form);
    DesignManager::formScene()->removeForm(form);
}

void FormsPane::addButtonClicked()
{
    auto tempPath = QStandardPaths::standardLocations(QStandardPaths::TempLocation)[0];
    tempPath = tempPath + separator() + "Objectwheel";

    rm(tempPath);

    if (!mkdir(tempPath) || !cp(DIR_QRC_FORM, tempPath, true, true))
        return;

    auto form = new Form(tempPath + separator() + DIR_THIS + separator() + "main.qml");
    DesignManager::formScene()->addForm(form);
    SaveManager::addForm(form);
    rm(tempPath);
}

void FormsPane::handleDatabaseChange()
{
    int row = 0;
    QString id;
    if (_listWidget->currentItem())
        id = _listWidget->currentItem()->text();

    _listWidget->clear();

    for (auto path : SaveManager::formPaths()) {
        auto _id = SaveManager::id(path);
        if (id == _id)
            row = _listWidget->count();

        auto item = new QListWidgetItem;
        item->setText(_id);
        if (SaveManager::isMain(path))
            item->setIcon(QIcon(":/resources/images/mform.png"));
        else
            item->setIcon(QIcon(":/resources/images/form.png"));
        _listWidget->addItem(item);
    }
    _listWidget->setCurrentRow(row);
}

void FormsPane::handleCurrentFormChange()
{
    if (!_listWidget->currentItem())
        return;

    auto id = _listWidget->currentItem()->text();
    for (auto form : DesignManager::formScene()->forms())
        if (form->id() == id)
            DesignManager::formScene()->setMainForm(form);
    DesignManager::instance()->updateSkin();
    emit currentFormChanged();
}

void FormsPane::setCurrentForm(int index)
{
    _listWidget->setCurrentRow(index);
}

QSize FormsPane::sizeHint() const
{
    return fit::fx(QSizF{200, 10}).toSize();
}