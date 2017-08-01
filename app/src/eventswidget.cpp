#include <eventswidget.h>
#include <combobox.h>
#include <listwidget.h>
#include <flatbutton.h>
#include <fit.h>
#include <qmleditor.h>
#include <css.h>
#include <lineedit.h>
#include <scrollarea.h>
#include <windowscene.h>
#include <control.h>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPropertyAnimation>
#include <QEasingCurve>
#include <QTimer>
#include <QComboBox>
#include <QPair>
#include <QLineEdit>
#include <QLabel>
#include <QMessageBox>

using namespace Fit;

class EventsWidgetPrivate
{
    public:
        EventsWidget* parent;
        QVBoxLayout verticalLayout;
        QHBoxLayout horizontalLayout;
        FlatButton addButton;
        FlatButton editButton;
        FlatButton removeButton;
        ListWidget eventsListWidget;
        bool hasPopupOpen = false;

        QWidget popupWidget;
        QVBoxLayout popupVLayout;
        FlatButton popupHideButton;
        LineEdit nameEdit;
        QLabel popupTitle;
        LineEdit popupItemNameTextBox;
        ScrollArea popupScrollArea;
        QWidget popupScrollAreaWidget;
        QVBoxLayout popupScrollAreaVLayout;
        QHBoxLayout codeLayout;
        LineEdit codeEdit;
        FlatButton btnEditCode;
        ComboBox targetEventCombobox;
        FlatButton popupOkButton;
        bool editMode = false;
        QString editingEventName;

    public:
        EventsWidgetPrivate(EventsWidget* parent);
        void addEventWithoutSave(const SaveManager::EventInf& inf);

    private:
        void removeButtonClicked();
        void addButtonClicked();
        void editButtonClicked();
        void popupHideButtonClicked();
        void popupOkButtonClicked();
        void btnEditCodeClicked();
        void ensureComboboxVisible(const QObject* obj);
};

EventsWidgetPrivate::EventsWidgetPrivate(EventsWidget* p)
    : parent(p)
{
    eventsListWidget.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    eventsListWidget.setFocusPolicy(Qt::NoFocus);
    eventsListWidget.setStyleSheet(CSS::BindingListWidget);
    eventsListWidget.setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    eventsListWidget.setDragDropMode(QAbstractItemView::NoDragDrop);
    eventsListWidget.setSelectionBehavior(QAbstractItemView::SelectRows);
    eventsListWidget.setSelectionMode(ListWidget::SingleSelection);
    eventsListWidget.setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    eventsListWidget.setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    QObject::connect(&eventsListWidget,(void(ListWidget::*)(int))(&ListWidget::currentRowChanged),[=](int i){removeButton.setEnabled(i>=0);});
    QObject::connect(&eventsListWidget,(void(ListWidget::*)(int))(&ListWidget::currentRowChanged),[=](int i){editButton.setEnabled(i>=0);});

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

    editButton.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    editButton.setColor("#0D74C8");
    editButton.setFixedSize(fit(30),fit(30));
    editButton.setRadius(fit(13));
    editButton.setIconSize(QSize(fit(16),fit(16)));
    editButton.setIcon(QIcon(":/resources/images/edit.png"));
    editButton.setDisabled(true);
    QObject::connect(&editButton, (void(FlatButton::*)(bool))(&FlatButton::clicked), [=] {editButtonClicked();});

    horizontalLayout.addWidget(&addButton);
    horizontalLayout.addStretch();
    horizontalLayout.addWidget(&removeButton);
    horizontalLayout.addStretch();
    horizontalLayout.addWidget(&editButton);

    codeLayout.setSpacing(fit(3));
    verticalLayout.addWidget(&eventsListWidget);
    verticalLayout.addLayout(&horizontalLayout);
    verticalLayout.setContentsMargins(fit(6), 0, fit(8), fit(8));
    parent->setLayout(&verticalLayout);

    /* ----------------------------- */
    popupWidget.setParent(parent);
    popupWidget.setObjectName("popupWindow");
    popupWidget.setStyleSheet("#popupWindow{background:#0D74C8;border:none;}");
    popupWidget.setHidden(true);
    popupWidget.setLayout(&popupVLayout);

    popupHideButton.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    popupHideButton.setColor("#C61717");
    popupHideButton.setFixedSize(fit(20),fit(16));
    popupHideButton.setRadius(fit(2));
    popupHideButton.setIconSize(QSize(fit(10),fit(10)));
    popupHideButton.setIcon(QIcon(":/resources/images/minus.png"));
    popupHideButton.setParent(&popupWidget);
    QObject::connect(&popupHideButton, (void(FlatButton::*)(bool))(&FlatButton::clicked), [=] {popupHideButtonClicked();});

    nameEdit.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    nameEdit.setFixedHeight(fit(30));
    nameEdit.setIcon(QIcon(":/resources/images/advanced.png"));
    nameEdit.setPlaceholderText("Event name");
    nameEdit.show();

    codeEdit.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    codeEdit.setFixedHeight(fit(30));
    codeEdit.setIcon(QIcon(":/resources/images/script.png"));
    codeEdit.setPlaceholderText("Event code");
    codeEdit.show();

    codeLayout.addWidget(&codeEdit);

    targetEventCombobox.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    targetEventCombobox.setFixedHeight(fit(40));
    targetEventCombobox.setIcon(QIcon(":/resources/images/plug-up.png"));
    targetEventCombobox.setColor("#C61717");
    targetEventCombobox.setPlaceHolderText("Target event");
    targetEventCombobox.show();
    QObject::connect(&targetEventCombobox, &ComboBox::popupShowed, [=] {ensureComboboxVisible(&targetEventCombobox);});
    QObject::connect(&targetEventCombobox, (void(ComboBox::*)(QString&))(&ComboBox::currentTextChanged), [=] (QString text){
        if (!text.isEmpty()) {
            codeEdit.setEnabled(true);
            btnEditCode.setEnabled(true);
        }
    });

    popupOkButton.setColor(QColor("#6BB64B"));
    popupOkButton.setTextColor(Qt::white);
    popupOkButton.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    popupOkButton.setFixedHeight(fit(30));
    popupOkButton.setText("Add");
    QObject::connect(&popupOkButton, (void(FlatButton::*)(bool))(&FlatButton::clicked), [=] {popupOkButtonClicked();});

    btnEditCode.setColor(QColor("#C61717"));
    btnEditCode.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    btnEditCode.setFixedHeight(fit(31));
    btnEditCode.setFixedWidth(fit(30));
    btnEditCode.setRadius(fit(2));
    btnEditCode.setIcon(QIcon(":/resources/images/edit.png"));
    QObject::connect(&btnEditCode, (void(FlatButton::*)(bool))(&FlatButton::clicked), [=] {btnEditCodeClicked();});
    codeLayout.addWidget(&btnEditCode);

    popupTitle.setText("Event Editor");
    popupTitle.setStyleSheet("background:transparent;color:white;text-decoration:underline;");
    popupItemNameTextBox.setPlaceholderText("Select target item...");
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

    popupScrollAreaVLayout.setSpacing(fit(2));
    popupScrollAreaVLayout.setContentsMargins(0,0,0,0);
    popupScrollAreaVLayout.addWidget(&targetEventCombobox);
    popupScrollAreaVLayout.addLayout(&codeLayout);
    popupScrollAreaVLayout.addStretch();

    popupVLayout.setSpacing(fit(6));
    popupVLayout.setContentsMargins(fit(4),0,fit(4),fit(6));
    popupVLayout.addWidget(&popupTitle);
    popupVLayout.addWidget(&nameEdit);
    popupVLayout.addWidget(&popupItemNameTextBox);
    popupVLayout.addWidget(&popupScrollArea);
    popupVLayout.addWidget(&popupOkButton);

    QObject::connect(WindowScene::instance(), SIGNAL(selectionChanged()), parent, SLOT(handleSelectionChange()));
}

void EventsWidgetPrivate::addEventWithoutSave(const SaveManager::EventInf& inf)
{
    if (inf.targetEventname.isEmpty() ||inf.targetId.isEmpty() ||
        inf.eventCode.isEmpty() || inf.eventName.isEmpty())
        return;

    eventsListWidget.addItem(inf.eventName);
}

void EventsWidgetPrivate::removeButtonClicked()
{
    auto connectionName = eventsListWidget.currentItem()->text();
    QMessageBox msgBox;
    msgBox.setText(QString("<b>This will delete %1 named event.</b>").arg(connectionName));
    msgBox.setInformativeText("Do you want to continue?");
    msgBox.setStandardButtons(QMessageBox::No | QMessageBox::Yes);
    msgBox.setDefaultButton(QMessageBox::No);
    msgBox.setIcon(QMessageBox::Warning);
    const int ret = msgBox.exec();
    switch (ret) {
        case QMessageBox::Yes: {
            SaveManager::removeEventSave(connectionName);
            delete eventsListWidget.takeItem(eventsListWidget.currentRow());
            break;
        } default: {
            // Do nothing
            break;
        }
    }
}

void EventsWidgetPrivate::addButtonClicked()
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
    popupHideButton.move(popupWidget.width()-fit(24), 0);
    hasPopupOpen = true;
    emit parent->popupShowed();
}

void EventsWidgetPrivate::editButtonClicked()
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
    popupHideButton.move(popupWidget.width()-fit(24), 0);
    editMode = true;
    editingEventName = eventsListWidget.currentItem()->text();

    auto issuerEvent = SaveManager::getEventSaves()[editingEventName].toObject();
    if (issuerEvent.isEmpty())
        return;

    auto controls = WindowScene::currentWindow()->childControls();
    controls << WindowScene::currentWindow();

    WindowScene::instance()->clearSelection();
    for (auto control : controls)
        if (control->id() == issuerEvent[EVENT_TARGET_ID_LABEL].toString())
            control->setSelected(true);

    nameEdit.setText(editingEventName);
    targetEventCombobox.setCurrentItem(issuerEvent[EVENT_TARGET_EVENTNAME_LABEL].toString());
    codeEdit.setText(QByteArray::fromBase64(QByteArray().insert(0, issuerEvent[EVENT_EVENT_CODE_LABEL].toString())));

    hasPopupOpen = true;
    emit parent->popupShowed();
}

void EventsWidgetPrivate::popupHideButtonClicked()
{
    editMode = false;
    auto animation = new QPropertyAnimation(&popupWidget, "geometry");
    animation->setDuration(500);
    animation->setEndValue(QRect(0, parent->height(), parent->width(), parent->height()));
    animation->setStartValue(QRect(0, 0, parent->width(), parent->height()));
    animation->setEasingCurve(QEasingCurve::OutExpo);
    QObject::connect(animation, SIGNAL(finished()), &popupWidget, SLOT(hide()));
    QObject::connect(animation, SIGNAL(finished()), animation, SLOT(deleteLater()));
    animation->start();
    hasPopupOpen = false;
    emit parent->popupHid();
}

void EventsWidgetPrivate::popupOkButtonClicked()
{
    if (targetEventCombobox.currentItem().isEmpty() ||
        codeEdit.text().isEmpty() ||
        popupItemNameTextBox.text().isEmpty())
        return;

    if (editMode)
        delete eventsListWidget.takeItem(eventsListWidget.currentRow());

    if (nameEdit.text().isEmpty())
        nameEdit.setText("Event");

    auto eventName = nameEdit.text();
    for (int i = 1; eventsListWidget.contains(eventName); i++)
        eventName = nameEdit.text() + QString::number(i);

    eventsListWidget.addItem(eventName);

    SaveManager::EventInf einf;
    einf.eventName = eventName;
    einf.targetId = popupItemNameTextBox.text();
    einf.targetEventname = targetEventCombobox.currentItem();
    einf.eventCode = codeEdit.text();

    if (editMode)
        SaveManager::changeEventSave(editingEventName, einf);
    else
        SaveManager::addEventSave(einf);

    parent->clearList();
    parent->handleSelectionChange();
    eventsListWidget.clearSelection();
    popupHideButtonClicked();
}

void EventsWidgetPrivate::btnEditCodeClicked()
{
    static QMetaObject::Connection conn;
    auto scene = WindowScene::instance();
    auto selectedControls = scene->selectedControls();

    if (scene->currentWindow()->isSelected())
        selectedControls << scene->currentWindow();

    if (selectedControls.isEmpty() ||
        selectedControls.size() > 1 ||
        selectedControls[0]->id().isEmpty())
        return;

    if (codeEdit.text().isEmpty()) {
        QmlEditor::showTextOnly("// " + selectedControls[0]->id() + ".on" +
                targetEventCombobox.currentItem().left(1).toUpper() + targetEventCombobox.currentItem().mid(1) + ":\n");
    } else {
        QmlEditor::showTextOnly(codeEdit.text());
    }
    QmlEditor::instance()->raise();
    conn = QObject::connect(QmlEditor::instance(), (void(QmlEditor::*)(QString&))(&QmlEditor::savedTextOnly), [=](QString& text) {
        codeEdit.setText(text);
        QObject::disconnect(conn);
        QmlEditor::hide();
    });
}

void EventsWidgetPrivate::ensureComboboxVisible(const QObject* obj)
{
    popupScrollArea.ensureWidgetVisible((QWidget*)obj);
}

EventsWidgetPrivate* EventsWidget::m_d = nullptr;

EventsWidget::EventsWidget(QWidget *parent)
    : QWidget(parent)
{
    if (m_d) return;
    m_d = new EventsWidgetPrivate(this);
}

EventsWidget* EventsWidget::instance()
{
    return m_d->parent;
}

EventsWidget::~EventsWidget()
{
    delete m_d;
}

void EventsWidget::clearList()
{
    m_d->targetEventCombobox.clear();
    m_d->nameEdit.setText("");
    m_d->popupItemNameTextBox.setText("");
    m_d->codeEdit.setText("");
    m_d->codeEdit.setDisabled(true);
    m_d->btnEditCode.setDisabled(true);
}

void EventsWidget::handleSelectionChange()
{
    auto scene = WindowScene::instance();
    auto selectedControls = scene->selectedControls();
    if (scene->currentWindow()->isSelected())
        selectedControls << scene->currentWindow();

    clearList();
    if (selectedControls.isEmpty() ||
        selectedControls.size() > 1 ||
        selectedControls[0]->id().isEmpty()) {
        return;
    } else {
        m_d->popupItemNameTextBox.setText(selectedControls[0]->id());
        for (auto event : selectedControls[0]->events())
            m_d->targetEventCombobox.addItem(event);
    }
}

void EventsWidget::detachEventsFor(Control* control)
{
    detachEventsFor(control->id());
}

void EventsWidget::detachEventsFor(const QString& id)
{
    auto saves = SaveManager::getEventSaves();
    for (auto key : saves.keys()) {
        auto save = saves[key].toObject();
        if (save[EVENT_TARGET_ID_LABEL] == id) {
            SaveManager::removeEventSave(key);
            for (int i=0; i < m_d->eventsListWidget.count(); i++) {
                if (m_d->eventsListWidget.item(i)->text() == key)
                    m_d->eventsListWidget.takeItem(i);
            }
        }
    }
}

void EventsWidget::clearAllEvents()
{
    auto saves = SaveManager::getEventSaves();
    for (auto key : saves.keys()) {
        SaveManager::removeEventSave(key);
    }
    m_d->eventsListWidget.clear();
    clearList();
}

void EventsWidget::addEventWithoutSave(const SaveManager::EventInf& inf)
{
    m_d->addEventWithoutSave(inf);
}

bool EventsWidget::hasPopupOpen()
{
    return m_d->hasPopupOpen;
}
