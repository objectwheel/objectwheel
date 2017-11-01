#include <formswidget.h>
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
#include <QPainter>
#include <QPalette>
#include <QListWidget>
#include <QScrollBar>

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
        QListWidget* formsListWidget;
        QHBoxLayout* horizontalLayout;
        FlatButton* addButton;
        FlatButton* removeButton;
};

FormsWidgetPrivate::FormsWidgetPrivate(FormsWidget* parent)
    : QObject(parent)
    , parent(parent)
    , verticalLayout(new QVBoxLayout)
    , formsListWidget(new QListWidget)
    , horizontalLayout(new QHBoxLayout)
    , addButton(new FlatButton)
    , removeButton(new FlatButton)
{
    parent->setAutoFillBackground(true);
    QPalette p(parent->palette());
    p.setColor(QPalette::Window, QColor("#E0E4E7"));
    parent->setPalette(p);

    QPalette p2(formsListWidget->palette());
    p2.setColor(QPalette::Base, QColor("#F3F7FA"));
    p2.setColor(QPalette::Highlight, QColor("#E0E4E7"));
    p2.setColor(QPalette::Text, QColor("#202427"));
    formsListWidget->setPalette(p2);

    formsListWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    formsListWidget->setFocusPolicy(Qt::NoFocus);
    formsListWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    formsListWidget->setDragDropMode(QAbstractItemView::NoDragDrop);
    formsListWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    formsListWidget->setSelectionMode(QListWidget::SingleSelection);
    formsListWidget->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    formsListWidget->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    formsListWidget->verticalScrollBar()->setStyleSheet(CSS::ScrollBar);
    formsListWidget->setIconSize(QSize(fit(14),fit(14)));

    QTimer::singleShot(1000, [=] { //FIXME
        Delayer::delay([]()->bool {if (SaveManager::instance()) return false; else return true;});
        connect(SaveManager::instance(), SIGNAL(projectExposed()), SLOT(handleDatabaseChange()));
        connect(SaveManager::instance(), SIGNAL(databaseChanged()), SLOT(handleDatabaseChange()));
        connect(formsListWidget, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), SLOT(handleCurrentFormChange()));
    });

    addButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    addButton->setColor("#6BB64B");
    addButton->setFixedSize(fit(17),fit(17));
    addButton->setRadius(fit(8));
    addButton->setIconSize(QSize(fit(11),fit(11)));
    addButton->setIcon(QIcon(":/resources/images/plus.png"));
    connect(addButton, SIGNAL(clicked(bool)), SLOT(addButtonClicked()));

    removeButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    removeButton->setColor("#C61717");
    removeButton->setFixedSize(fit(17),fit(17));
    removeButton->setRadius(fit(8));
    removeButton->setIconSize(QSize(fit(11),fit(11)));
    removeButton->setIcon(QIcon(":/resources/images/minus.png"));
    connect(removeButton, SIGNAL(clicked(bool)), SLOT(removeButtonClicked()));

    horizontalLayout->addWidget(addButton);
    horizontalLayout->addStretch();
    horizontalLayout->addWidget(removeButton);

    verticalLayout->addWidget(formsListWidget);
    verticalLayout->addLayout(horizontalLayout);
    verticalLayout->setSpacing(fit(2));
    verticalLayout->setContentsMargins(fit(3), fit(3), fit(3), fit(3));
    parent->setLayout(verticalLayout);
}

void FormsWidgetPrivate::removeButtonClicked()
{
    auto form = DesignManager::formScene()->mainForm();
    if (!form || !form->form() || form->main())
        return;
    SaveManager::removeForm((Form*)form);
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
    SaveManager::addForm(form);
    rm(tempPath);
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

        auto item = new QListWidgetItem;
        item->setText(_id);
        if (SaveManager::isMain(path))
            item->setIcon(QIcon(":/resources/images/mform.png"));
        else
            item->setIcon(QIcon(":/resources/images/form.png"));
        formsListWidget->addItem(item);
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
    DesignManager::instance()->updateSkin();
    emit parent->currentFormChanged();
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

QSize FormsWidget::sizeHint() const
{
    return QSize(fit(200), fit(10));
}

#include "formswidget.moc"
