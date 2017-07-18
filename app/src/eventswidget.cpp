#include <eventswidget.h>
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
#include <scrollarea.h>
#include <QMessageBox>
#include <qmleditor.h>
#include <QQmlExpression>
#include <QPointer>

using namespace Fit;

class EventsWidgetPrivate
{
    public:
        EventsWidget* parent;
        const QList<QQuickItem*>* items;
        QObject* rootObject;
        QQmlContext* rootContext;
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
        QPointer<QQuickItem> lastTargetItem;
        FlatButton popupOkButton;
        bool editMode = false;

        struct Event
        {
                QObject* targetItem;
                QString targetEventname;
                QString eventCode;
                QMetaObject::Connection connection;
                QString connectionName;

                bool operator== (const Event& x) {
                    return (x.eventCode == this->eventCode &&
                            x.targetItem == this->targetItem &&
                            x.targetEventname == this->targetEventname &&
                            x.connection == this->connection &&
                            x.connectionName == this->connectionName);
                }
        };
        QList<Event> events;
        EventsWidgetPrivate(EventsWidget* parent);
        void addEventWithoutSave(const SaveManager::EventInf& inf);

    private slots:
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
    , items(nullptr)
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
}

void EventsWidgetPrivate::addEventWithoutSave(const SaveManager::EventInf& inf)
{
    if (!inf.targetEventname.isEmpty() && !inf.targetId.isEmpty() &&
        !inf.eventCode.isEmpty() && !inf.eventName.isEmpty()) {
        QQuickItem* targetItem = nullptr;
        for (auto item : *items) {
            if (rootContext->nameForObject(item) == inf.targetId) {
                targetItem = item;
            }
        }

        if (!targetItem) return;

        QMetaMethod targetSign;
        for (int i = 0; i < targetItem->metaObject()->methodCount(); i++)
            if (QString(targetItem->metaObject()->method(i).name()) == inf.targetEventname) {
                targetSign = targetItem->metaObject()->method(i);
            }
        if (!targetSign.isValid()) return;

        QMetaObject::Connection connection;
        bool shouldConnect = true;
        for (auto event: events) {
            if (event.targetItem == targetItem &&
                event.targetEventname == inf.targetEventname) {
                shouldConnect = false;
                break;
            }
        }
        if (shouldConnect) {
            connection = QObject::connect(targetItem, targetSign, parent, parent->metaObject()->method(parent->metaObject()->indexOfSlot("processEvents()")));
        }

        auto connectionName = inf.eventName;
        if (connectionName.isEmpty()) {
            connectionName = QString("Event %1").arg(eventsListWidget.count());
        }

        for (int i = 0; i < eventsListWidget.count(); i++) {
            if (eventsListWidget.item(i)->text() == connectionName) {
                connectionName+="+";
                i = -1;
            }
        }

        Event eventData;
        eventData.targetItem = targetItem; //Target item
        eventData.targetEventname = inf.targetEventname;
        eventData.eventCode = inf.eventCode;
        eventData.connection = connection;
        eventData.connectionName = connectionName;
        events << eventData;
        eventsListWidget.addItem(connectionName);
    }
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
            Event event;
            for (auto e : events) {
                if (e.connectionName == connectionName) {
                    event = e;
                }
            }
            SaveManager::removeEventSave(event.connectionName);
            events.removeOne(event);

            bool shouldRemove = true;
            if (event.connection) {
                for (auto& e : events) {
                    if (e.targetItem == event.targetItem &&
                        e.targetEventname == event.targetEventname) {
                        e.connection = event.connection;
                        shouldRemove = false;
                        break;
                    }
                }
                if (shouldRemove) {
                    QObject::disconnect(event.connection);
                }
            }

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

    Event e;
    for (auto event : events)
        if (event.connectionName == eventsListWidget.currentItem()->text())
            e = event;
    parent->selectItem(e.targetItem);

    targetEventCombobox.setCurrentItem(e.targetEventname);
    nameEdit.setText(eventsListWidget.currentItem()->text());
    codeEdit.setText(e.eventCode);
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
    if (!targetEventCombobox.currentItem().isEmpty() &&
        !codeEdit.text().isEmpty() &&
        !popupItemNameTextBox.text().isEmpty()) {
        if (!editMode) {
            if (!lastTargetItem) return;

            QString targetEventname;
            QMetaMethod targetSign;
            for (int i = 0; i < lastTargetItem->metaObject()->methodCount(); i++)
                if (QString(lastTargetItem->metaObject()->method(i).name()) == targetEventCombobox.currentItem()) {
                    targetSign = lastTargetItem->metaObject()->method(i);
                    targetEventname = lastTargetItem->metaObject()->method(i).name();
                }
            if (!targetSign.isValid()) return;

            QMetaObject::Connection connection;
            bool shouldConnect = true;
            for (auto event: events) {
                if (event.targetItem == lastTargetItem &&
                    event.targetEventname == targetEventname) {
                    shouldConnect = false;
                    break;
                }
            }
            if (shouldConnect) {
                connection = QObject::connect(lastTargetItem, targetSign, parent, parent->metaObject()->method(parent->metaObject()->indexOfSlot("processEvents()")));
            }

            auto connectionName = nameEdit.text();
            if (connectionName.isEmpty()) {
                connectionName = QString("Event %1").arg(eventsListWidget.count());
            }

            for (int i = 0; i < eventsListWidget.count(); i++) {
                if (eventsListWidget.item(i)->text() == connectionName) {
                    connectionName+="+";
                    i = -1;
                }
            }

            Event eventData;
            eventData.targetItem = lastTargetItem; //Target item
            eventData.targetEventname = targetEventCombobox.currentItem(); //Target event name
            eventData.eventCode = codeEdit.text();
            eventData.connection = connection;
            eventData.connectionName = connectionName;
            events << eventData;
            SaveManager::EventInf einf;
            einf.eventName = connectionName;
            einf.targetId = rootContext->nameForObject(lastTargetItem);
            einf.targetEventname = targetEventCombobox.currentItem();
            einf.eventCode = codeEdit.text();
            SaveManager::addEventSave(einf);
            eventsListWidget.addItem(connectionName);
            popupHideButtonClicked();
        } else {
            if (!lastTargetItem) return;
            auto connName = eventsListWidget.currentItem()->text();
            Event event;
            for (auto e : events) {
                if (e.connectionName == connName) {
                    event = e;
                }
            }
            events.removeOne(event);

            bool shouldRemove = true;
            if (event.connection) {
                for (auto& e : events) {
                    if (e.targetItem == event.targetItem &&
                        e.targetEventname == event.targetEventname) {
                        e.connection = event.connection;
                        shouldRemove = false;
                        break;
                    }
                }
                if (shouldRemove) {
                    QObject::disconnect(event.connection);
                }
            }

            QString targetEventname;
            QMetaMethod targetSign;
            for (int i = 0; i < lastTargetItem->metaObject()->methodCount(); i++)
                if (QString(lastTargetItem->metaObject()->method(i).name()) == targetEventCombobox.currentItem()) {
                    targetSign = lastTargetItem->metaObject()->method(i);
                    targetEventname = lastTargetItem->metaObject()->method(i).name();
                }
            if (!targetSign.isValid()) return;

            QMetaObject::Connection connection;
            bool shouldConnect = true;
            for (auto event: events) {
                if (event.targetItem == lastTargetItem &&
                    event.targetEventname == targetEventname)
                    shouldConnect = false;
            }
            if (shouldConnect) {
                connection = QObject::connect(lastTargetItem, targetSign, parent, parent->metaObject()->method(parent->metaObject()->indexOfSlot("processEvents()")));
            }

            auto connectionName = nameEdit.text();
            if (connectionName.isEmpty()) {
                connectionName = QString("Event %1").arg(eventsListWidget.count());
            }

            for (int i = 0; i < eventsListWidget.count(); i++) {
                if (eventsListWidget.item(i)->text() != connName && eventsListWidget.item(i)->text() == connectionName) {
                    connectionName+="+";
                    i = -1;
                }
            }

            Event eventData;
            eventData.targetItem = lastTargetItem; //Target item
            eventData.targetEventname = targetEventCombobox.currentItem(); //Target event name
            eventData.eventCode = codeEdit.text();
            eventData.connection = connection;
            eventData.connectionName = connectionName;
            events << eventData;
            SaveManager::EventInf einf;
            einf.eventName = connectionName;
            einf.targetId = rootContext->nameForObject(lastTargetItem);
            einf.targetEventname = targetEventCombobox.currentItem();
            einf.eventCode = codeEdit.text();
            SaveManager::changeEventSave(event.connectionName, einf);
            eventsListWidget.item(eventsListWidget.currentRow())->setText(connectionName);
            popupHideButtonClicked();
        }
    }

}

void EventsWidgetPrivate::btnEditCodeClicked()
{
    static QMetaObject::Connection conn;
    if (codeEdit.text().isEmpty()) {
        QmlEditor::showTextOnly("// " + rootContext->nameForObject(lastTargetItem) + ".on" +
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

void EventsWidget::selectItem(QObject* const selectedItem)
{
    m_d->lastTargetItem = qobject_cast<QQuickItem*>(selectedItem);
    m_d->targetEventCombobox.clear();
    m_d->popupItemNameTextBox.setText(m_d->rootContext->nameForObject(m_d->lastTargetItem));

    auto metaObject = selectedItem->metaObject();
    for (int i = metaObject->methodCount(); --i;) {
        if (metaObject->method(i).methodType() == QMetaMethod::Signal
            && !metaObject->method(i).name().startsWith("__"))
            m_d->targetEventCombobox.addItem(metaObject->method(i).name());
    }

    setEnabled(true);
}

void EventsWidget::detachEventsFor(QObject* const item)
{
    QListIterator<EventsWidgetPrivate::Event> i(m_d->events);
    while (i.hasNext()) {
        auto event = i.next();
        if (event.targetItem == item) {
            SaveManager::removeEventSave(event.connectionName);
            m_d->events.removeOne(event);
            bool shouldRemove = true;
            if (event.connection) {
                for (auto& e : m_d->events) {
                    if (e.targetItem == event.targetItem &&
                        e.targetEventname == event.targetEventname) {
                        e.connection = event.connection;
                        shouldRemove = false;
                        break;
                    }
                }
                if (shouldRemove) {
                    QObject::disconnect(event.connection);
                }
            }
            for (int i=0; i < m_d->eventsListWidget.count(); i++) {
                if (m_d->eventsListWidget.item(i)->text() == event.connectionName)
                    m_d->eventsListWidget.takeItem(i);
            }
        }
    }
}

void EventsWidget::clearAllEvents()
{
    for (auto item : *m_d->items) {
        detachEventsFor(item);
    }
}

const QList<QQuickItem*>* EventsWidget::itemSource() const
{
    return m_d->items;
}

void EventsWidget::setItemSource(const QList<QQuickItem*>* const items)
{
    m_d->items = items;
}

const QQmlContext* EventsWidget::rootContext() const
{
    return m_d->rootContext;
}

void EventsWidget::setRootContext(QQmlContext* const rootContext)
{
    m_d->rootContext = rootContext;
}

const QObject* EventsWidget::rootObject() const
{
    return m_d->rootObject;
}

void EventsWidget::setRootObject(QObject* const rootObject)
{
    m_d->rootObject = rootObject;
}

void EventsWidget::addEventWithoutSave(const SaveManager::EventInf& inf)
{
    m_d->addEventWithoutSave(inf);
}

bool EventsWidget::hasPopupOpen()
{
    return m_d->hasPopupOpen;
}

void EventsWidget::processEvents()
{
    auto targetSignalIndex = senderSignalIndex();
    auto targetItem = sender();
    if (!targetItem || targetSignalIndex < 0) { qWarning("Event error! 0x00"); return; }

    QString targetEventname;
    for (int i = 0; i < targetItem->metaObject()->methodCount(); i++) {
        if (targetSignalIndex == targetItem->metaObject()->method(i).methodIndex())
            targetEventname = targetItem->metaObject()->method(i).name();
    }
    if (targetEventname.isEmpty()) { qWarning("Event error! 0x01"); return; }

    for (auto event : m_d->events) {
        if (event.targetItem == targetItem &&
            event.targetEventname == targetEventname) {
            QQmlExpression script(m_d->rootContext, event.targetItem, event.eventCode);
            script.evaluate();
        }
    }
}
