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

using namespace Fit;

class EventsWidgetPrivate
{
    public:
        EventsWidget* parent;
        const QList<QQuickItem*>* items;
        QQmlContext* rootContext;
        QVBoxLayout verticalLayout;
        QHBoxLayout horizontalLayout;
        FlatButton addButton;
        FlatButton editButton;
        FlatButton removeButton;
        ListWidget bindingListWidget;
        bool hasPopupOpen = false;

        QWidget popupWidget;
        QVBoxLayout popupVLayout;
        FlatButton popupHideButton;
        QHBoxLayout codeLayout;
        LineEdit nameEdit;
        LineEdit codeEdit;
        FlatButton btnEditCode;
        QLabel popupTitle;
        LineEdit popupItemNameTextBox;
        QQuickItem* targetItem;
        ScrollArea popupScrollArea;
        QWidget popupScrollAreaWidget;
        QVBoxLayout popupScrollAreaVLayout;
        ComboBox targetEventCombobox;
        QQuickItem* lastTargetItem;
        FlatButton popupOkButton;

        bool editMode = false;

        struct Binding
        {
                QObject* sourceItem;
                QString sourceProperty;
                QObject* targetItem;
                QString targetProperty;
                QMetaObject::Connection connection;
                QString connectionName;

                bool operator== (const Binding& x) {
                    if (x.sourceItem == this->sourceItem &&
                        x.sourceProperty == this->sourceProperty &&
                        x.targetItem == this->targetItem &&
                        x.targetProperty == this->targetProperty &&
                        x.connection == this->connection &&
                        x.connectionName == this->connectionName) {
                        return true;
                    } else {
                        return false;
                    }
                }
        };
        QList<Binding> bindings;
        EventsWidgetPrivate(EventsWidget* parent);
        void addBindingWithoutSave(const SaveManager::BindingInf& inf);

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
    QObject::connect(&bindingListWidget,(void(ListWidget::*)(int))(&ListWidget::currentRowChanged),[=](int i){editButton.setEnabled(i>=0);});

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

    verticalLayout.addWidget(&bindingListWidget);
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

void EventsWidgetPrivate::addBindingWithoutSave(const SaveManager::BindingInf& inf)
{
//    if (!inf.targetProperty.isEmpty() && !inf.targetId.isEmpty() &&
//        !inf.sourceId.isEmpty() && !inf.sourceProperty.isEmpty() && !inf.bindingName.isEmpty()) {
//        QQuickItem* sourceItem = nullptr;
//        QQuickItem* targetItem = nullptr;
//        for (auto item : *items) {
//            auto itemName = rootContext->nameForObject(item);
//            if (itemName == inf.sourceId) {
//                sourceItem = item;
//            }
//            if (itemName == inf.targetId) {
//                targetItem = item;
//            }
//        }
//        if (!sourceItem || !targetItem) return;

//        QMetaMethod sourceSign;
//        for (int i = 0; i < sourceItem->metaObject()->propertyCount(); i++)
//            if (QString(sourceItem->metaObject()->property(i).name()) == inf.sourceProperty)
//                sourceSign = sourceItem->metaObject()->property(i).notifySignal();
//        if (!sourceSign.isValid()) return;

//        auto connection = QObject::connect(sourceItem, sourceSign, parent, parent->metaObject()->method(parent->metaObject()->indexOfSlot("processBindings()")));
//        auto connectionName = inf.bindingName;

//        Binding bindingData;
//        bindingData.sourceItem = sourceItem; //Source item
//        bindingData.sourceProperty = inf.sourceProperty; //Source property
//        bindingData.targetItem = targetItem; //Target item
//        bindingData.targetProperty = inf.targetProperty; //Target property
//        bindingData.connection = connection;
//        bindingData.connectionName = connectionName;
//        bindings << bindingData;
//        bindingListWidget.addItem(connectionName);
//    }
}

void EventsWidgetPrivate::removeButtonClicked()
{
    auto connectionName = bindingListWidget.currentItem()->text();
    QMessageBox msgBox;
    msgBox.setText(QString("<b>This will delete %1 named event.</b>").arg(connectionName));
    msgBox.setInformativeText("Do you want to continue?");
    msgBox.setStandardButtons(QMessageBox::No | QMessageBox::Yes);
    msgBox.setDefaultButton(QMessageBox::No);
    msgBox.setIcon(QMessageBox::Warning);
    const int ret = msgBox.exec();
    switch (ret) {
        case QMessageBox::Yes: {
            Binding binding;
            for (auto b : bindings) {
                if (b.connectionName == connectionName) {
                    binding = b;
                }
            }
            SaveManager::removeBindingSave(binding.connectionName);
            bindings.removeOne(binding);
            QObject::disconnect(binding.connection);
            delete bindingListWidget.takeItem(bindingListWidget.currentRow());
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
    parent->clearList();
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
    popupHideButton.move(popupWidget.width()-fit(30), fit(10));
    editMode = true;

    Binding b;
    for (auto binding : bindings)
        if (binding.connectionName == bindingListWidget.currentItem()->text())
            b = binding;
    parent->selectItem(b.targetItem);

    targetEventCombobox.setCurrentItem(b.targetProperty);
    nameEdit.setText(bindingListWidget.currentItem()->text());
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
//    if (!targetEventCombobox.currentItem().isEmpty() &&
//        !sourceItemCombobox.currentItem().isEmpty() &&
//        !sourcePropertyCombobox.currentItem().isEmpty()) {
//        if (!editMode) {
//            QQuickItem* sourceItem = nullptr;
//            for (auto item : *items) {
//                auto itemName = rootContext->nameForObject(item);
//                if (itemName == sourceItemCombobox.currentItem()) {
//                    sourceItem = item;
//                }
//            }
//            if (!sourceItem) return;
//            if (!lastTargetItem) return;

//            QMetaMethod sourceSign;
//            for (int i = 0; i < sourceItem->metaObject()->propertyCount(); i++)
//                if (QString(sourceItem->metaObject()->property(i).name()) == sourcePropertyCombobox.currentItem())
//                    sourceSign = sourceItem->metaObject()->property(i).notifySignal();
//            if (!sourceSign.isValid()) return;

//            auto connection = QObject::connect(sourceItem, sourceSign, parent, parent->metaObject()->method(parent->metaObject()->indexOfSlot("processBindings()")));

//            auto connectionName = nameEdit.text();
//            if (connectionName.isEmpty()) {
//                connectionName = QString("Binding %1").arg(bindingListWidget.count());
//            }

//            for (int i = 0; i < bindingListWidget.count(); i++) {
//                if (bindingListWidget.item(i)->text() == connectionName) {
//                    connectionName+="+";
//                    i = -1;
//                }
//            }

//            Binding bindingData;
//            bindingData.sourceItem = sourceItem; //Source item
//            bindingData.sourceProperty = sourcePropertyCombobox.currentItem(); //Source property
//            bindingData.targetItem = lastTargetItem; //Target item
//            bindingData.targetProperty = targetEventCombobox.currentItem(); //Target property
//            bindingData.connection = connection;
//            bindingData.connectionName = connectionName;
//            bindings << bindingData;
//            SaveManager::BindingInf binf;
//            binf.bindingName = connectionName;
//            binf.sourceId = rootContext->nameForObject(sourceItem);
//            binf.sourceProperty = sourcePropertyCombobox.currentItem();
//            binf.targetId = rootContext->nameForObject(lastTargetItem);
//            binf.targetProperty = targetEventCombobox.currentItem();
//            SaveManager::addBindingSave(binf);
//            bindingListWidget.addItem(connectionName);
//            popupHideButtonClicked();
//        } else {
//            auto connName = bindingListWidget.currentItem()->text();
//            Binding binding;
//            for (auto b : bindings) {
//                if (b.connectionName == connName) {
//                    binding = b;
//                }
//            }
//            bindings.removeOne(binding);
//            QObject::disconnect(binding.connection);

//            QQuickItem* sourceItem = nullptr;
//            for (auto item : *items) {
//                auto itemName = rootContext->nameForObject(item);
//                if (itemName == sourceItemCombobox.currentItem()) {
//                    sourceItem = item;
//                }
//            }
//            if (!sourceItem) return;
//            if (!lastTargetItem) return;

//            QMetaMethod sourceSign;
//            for (int i = 0; i < sourceItem->metaObject()->propertyCount(); i++)
//                if (QString(sourceItem->metaObject()->property(i).name()) == sourcePropertyCombobox.currentItem())
//                    sourceSign = sourceItem->metaObject()->property(i).notifySignal();
//            if (!sourceSign.isValid()) return;

//            auto connection = QObject::connect(sourceItem, sourceSign, parent, parent->metaObject()->method(parent->metaObject()->indexOfSlot("processBindings()")));

//            auto connectionName = nameEdit.text();
//            if (connectionName.isEmpty()) {
//                connectionName = QString("Binding %1").arg(bindingListWidget.count());
//            }

//            for (int i = 0; i < bindingListWidget.count(); i++) {
//                if (bindingListWidget.item(i)->text() != connName && bindingListWidget.item(i)->text() == connectionName) {
//                    connectionName+="+";
//                    i = -1;
//                }
//            }

//            Binding bindingData;
//            bindingData.sourceItem = sourceItem; //Source item
//            bindingData.sourceProperty = sourcePropertyCombobox.currentItem(); //Source property
//            bindingData.targetItem = lastTargetItem; //Target item
//            bindingData.targetProperty = targetEventCombobox.currentItem(); //Target property
//            bindingData.connection = connection;
//            bindingData.connectionName = connectionName;
//            bindings << bindingData;
//            SaveManager::BindingInf binf;
//            binf.bindingName = connectionName;
//            binf.sourceId = rootContext->nameForObject(sourceItem);
//            binf.sourceProperty = sourcePropertyCombobox.currentItem();
//            binf.targetId = rootContext->nameForObject(lastTargetItem);
//            binf.targetProperty = targetEventCombobox.currentItem();
//            SaveManager::changeBindingSave(binding.connectionName, binf);
//            bindingListWidget.item(bindingListWidget.currentRow())->setText(connectionName);
//            popupHideButtonClicked();
//        }
//    }

}

void EventsWidgetPrivate::btnEditCodeClicked()
{

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
}

void EventsWidget::selectItem(QObject* const selectedItem)
{
    m_d->lastTargetItem = qobject_cast<QQuickItem*>(selectedItem);
    m_d->targetEventCombobox.clear();
    m_d->targetItem = qobject_cast<QQuickItem*>(selectedItem);
    m_d->popupItemNameTextBox.setText(m_d->rootContext->nameForObject(m_d->targetItem));

    auto metaObject = selectedItem->metaObject();
    for (int i = metaObject->methodCount(); --i;) {
        if (metaObject->method(i).methodType() == QMetaMethod::Signal
            && !metaObject->method(i).name().startsWith("__"))
            m_d->targetEventCombobox.addItem(metaObject->method(i).name());
    }

    setEnabled(true);
}

void EventsWidget::detachBindingsFor(QObject* const item)
{
//    QListIterator<EventsWidgetPrivate::Binding> i(m_d->bindings);
//    while (i.hasNext()) {
//        auto binding = i.next();
//        if (binding.sourceItem == item || binding.targetItem == item) {
//            SaveManager::removeBindingSave(binding.connectionName);
//            disconnect(binding.connection);
//            m_d->bindings.removeOne(binding);
//            for (int i=0; i < m_d->bindingListWidget.count(); i++) {
//                if (m_d->bindingListWidget.item(i)->text() == binding.connectionName)
//                    m_d->bindingListWidget.takeItem(i);
//            }
//        }
//    }
}

void EventsWidget::clearAllBindings()
{
    for (auto item : *m_d->items) {
        detachBindingsFor(item);
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

void EventsWidget::addBindingWithoutSave(const SaveManager::BindingInf& inf)
{
    m_d->addBindingWithoutSave(inf);
}

bool EventsWidget::hasPopupOpen()
{
    return m_d->hasPopupOpen;
}

void EventsWidget::processBindings()
{
//    auto sourceSignalIndex = senderSignalIndex();
//    auto sourceItem = sender();
//    if (!sourceItem || sourceSignalIndex < 0) { qWarning("Event error! 0x00"); return; }

//    QString sourceProperty;
//    for (int i = 0; i < sourceItem->metaObject()->propertyCount(); i++) {
//        if (sourceSignalIndex == sourceItem->metaObject()->property(i).notifySignalIndex())
//            sourceProperty = sourceItem->metaObject()->property(i).name();
//    }
//    if (sourceProperty.isEmpty()) { qWarning("Event error! 0x01"); return; }

//    auto value = sourceItem->property(sourceProperty.toStdString().c_str());
//    for (auto binding : m_d->bindings) {
//        if (binding.sourceItem == sourceItem &&
//            binding.sourceProperty == sourceProperty) {
//            binding.targetItem->setProperty(binding.targetProperty.toStdString().c_str(), value);
//        }
//    }
}
