#include <bindingwidget.h>
#include <listwidget.h>
#include <flatbutton.h>
#include <fit.h>
#include <designerscene.h>
#include <control.h>

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
#include <QPointer>
#include <QJsonObject>

using namespace Fit;

class BindingWidgetPrivate
{
    public:
        BindingWidget* parent;
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
        LineEdit nameEdit;
        QLabel popupTitle;
        LineEdit popupItemNameTextBox;
        ScrollArea popupScrollArea;
        QWidget popupScrollAreaWidget;
        QVBoxLayout popupScrollAreaVLayout;
        QWidget popupScrollAreaEmptyWidget;
        ComboBox targetPropertyCombobox;
        ComboBox sourceItemCombobox;
        ComboBox sourcePropertyCombobox;
        FlatButton popupOkButton;
        QLabel popupSeperateLine;
        bool editMode = false;
        QString editingBindingName;

    public:
        BindingWidgetPrivate(BindingWidget* parent);
        void addBindingWithoutSave(const SaveManager::BindingInf& inf);

    private:
        void removeButtonClicked();
        void addButtonClicked();
        void editButtonClicked();
        void popupHideButtonClicked();
        void popupOkButtonClicked();
        void ensureComboboxVisible(const QObject* obj);
        void updateSourcePropertyCombobox();
};

BindingWidgetPrivate::BindingWidgetPrivate(BindingWidget* p)
    : parent(p)
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
    nameEdit.setPlaceholderText("Binding name");
    nameEdit.show();

    targetPropertyCombobox.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    targetPropertyCombobox.setFixedHeight(fit(40));
    targetPropertyCombobox.setIcon(QIcon(":/resources/images/plug-up.png"));
    targetPropertyCombobox.setColor("#C61717");
    targetPropertyCombobox.setPlaceHolderText("Target property");
    targetPropertyCombobox.show();
    QObject::connect(&targetPropertyCombobox, &ComboBox::popupShowed, [=] {ensureComboboxVisible(&targetPropertyCombobox);});
    QObject::connect(&targetPropertyCombobox, &ComboBox::currentTextChanged, [=] { updateSourcePropertyCombobox(); });

    sourceItemCombobox.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    sourceItemCombobox.setFixedHeight(fit(40));
    sourceItemCombobox.setIcon(QIcon(":/resources/images/down-arrow.png"));
    sourceItemCombobox.setColor("#C61717");
    sourceItemCombobox.setPlaceHolderText("Source item");
    sourceItemCombobox.show();
    QObject::connect(&sourceItemCombobox, &ComboBox::popupShowed, [=] {ensureComboboxVisible(&sourceItemCombobox);});
    QObject::connect(&sourceItemCombobox, &ComboBox::currentTextChanged, [=] { updateSourcePropertyCombobox(); });

    sourcePropertyCombobox.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    sourcePropertyCombobox.setFixedHeight(fit(40));
    sourcePropertyCombobox.setIcon(QIcon(":/resources/images/plug-down.png"));
    sourcePropertyCombobox.setColor("#C61717");
    sourcePropertyCombobox.setPlaceHolderText("Source property");
    sourcePropertyCombobox.show();
    QObject::connect(&sourcePropertyCombobox, &ComboBox::popupShowed, [=] {ensureComboboxVisible(&sourcePropertyCombobox);});

    popupOkButton.setColor(QColor("#6BB64B"));
    popupOkButton.setTextColor(Qt::white);
    popupOkButton.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    popupOkButton.setFixedHeight(fit(30));
    popupOkButton.setText("Add");
    QObject::connect(&popupOkButton, (void(FlatButton::*)(bool))(&FlatButton::clicked), [=] {popupOkButtonClicked();});

    popupTitle.setText("Binding Editor");
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

    popupScrollAreaEmptyWidget.setFixedHeight(fit(10));
    popupScrollAreaEmptyWidget.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    popupSeperateLine.setStyleSheet("border:none;background:#dddddd;");
    popupSeperateLine.setFixedHeight(fit(1));
    popupSeperateLine.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    popupScrollAreaVLayout.setSpacing(fit(2));
    popupScrollAreaVLayout.setContentsMargins(0,0,0,0);
    popupScrollAreaVLayout.addWidget(&targetPropertyCombobox);
    popupScrollAreaVLayout.addWidget(&sourceItemCombobox);
    popupScrollAreaVLayout.addWidget(&sourcePropertyCombobox);
    popupScrollAreaVLayout.addWidget(&popupScrollAreaEmptyWidget);
    popupScrollAreaVLayout.addStretch();

    popupVLayout.setSpacing(fit(6));
    popupVLayout.setContentsMargins(fit(4),0,fit(4),fit(6));
    popupVLayout.addWidget(&popupTitle);
    popupVLayout.addWidget(&nameEdit);
    popupVLayout.addWidget(&popupItemNameTextBox);
    popupVLayout.addWidget(&popupSeperateLine);
    popupVLayout.addWidget(&popupScrollArea);
    popupVLayout.addWidget(&popupOkButton);

    QObject::connect(DesignerScene::instance(), SIGNAL(selectionChanged()), parent, SLOT(handleSelectionChange()));
}
void BindingWidgetPrivate::editButtonClicked()
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
    editingBindingName = bindingListWidget.currentItem()->text();

    auto issuerBinding = SaveManager::getBindingSaves()[editingBindingName].toObject();
    if (issuerBinding.isEmpty())
        return;

    auto controls = DesignerScene::currentPage()->childControls();
    controls << DesignerScene::currentPage();

    DesignerScene::instance()->clearSelection();
    for (auto control : controls)
        if (control->id() == issuerBinding[BINDING_TARGET_ID_LABEL].toString())
            control->setSelected(true);

    nameEdit.setText(editingBindingName);
    targetPropertyCombobox.setCurrentItem(issuerBinding[BINDING_TARGET_PROPERTY_LABEL].toString());
    sourceItemCombobox.setCurrentItem(issuerBinding[BINDING_SOURCE_ID_LABEL].toString());
    sourcePropertyCombobox.setCurrentItem(issuerBinding[BINDING_SOURCE_PROPERTY_LABEL].toString());

    hasPopupOpen = true;
    emit parent->popupShowed();
}

void BindingWidgetPrivate::addBindingWithoutSave(const SaveManager::BindingInf& inf)
{
    if (inf.targetProperty.isEmpty() || inf.targetId.isEmpty() ||
        inf.sourceId.isEmpty() || inf.sourceProperty.isEmpty() ||
        inf.bindingName.isEmpty())
        return;

    bindingListWidget.addItem(inf.bindingName);
}

void BindingWidgetPrivate::removeButtonClicked()
{
    auto bindingName = bindingListWidget.currentItem()->text();
    QMessageBox msgBox;
    msgBox.setText(QString("<b>This will delete %1 named binding.</b>").arg(bindingName));
    msgBox.setInformativeText("Do you want to continue?");
    msgBox.setStandardButtons(QMessageBox::No | QMessageBox::Yes);
    msgBox.setDefaultButton(QMessageBox::No);
    msgBox.setIcon(QMessageBox::Warning);
    const int ret = msgBox.exec();
    switch (ret) {
        case QMessageBox::Yes: {
            SaveManager::removeBindingSave(bindingName);
            delete bindingListWidget.takeItem(bindingListWidget.currentRow());
            break;
        } default: {
            // Do nothing
            break;
        }
    }
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
    popupHideButton.move(popupWidget.width()-fit(24), 0);
    hasPopupOpen = true;
    emit parent->popupShowed();
}

void BindingWidgetPrivate::popupHideButtonClicked()
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

void BindingWidgetPrivate::popupOkButtonClicked()
{
    if (targetPropertyCombobox.currentItem().isEmpty() ||
        sourceItemCombobox.currentItem().isEmpty() ||
        sourcePropertyCombobox.currentItem().isEmpty() ||
        popupItemNameTextBox.text().isEmpty())
        return;

    if (editMode)
        delete bindingListWidget.takeItem(bindingListWidget.currentRow());

    if (nameEdit.text().isEmpty())
        nameEdit.setText("Binding");

    auto bindingName = nameEdit.text();
    for (int i = 1; bindingListWidget.contains(bindingName); i++)
        bindingName = nameEdit.text() + QString::number(i);

    bindingListWidget.addItem(bindingName);

    SaveManager::BindingInf binf;
    binf.bindingName = bindingName;
    binf.sourceId = sourceItemCombobox.currentItem();
    binf.sourceProperty = sourcePropertyCombobox.currentItem();
    binf.targetId = popupItemNameTextBox.text();
    binf.targetProperty = targetPropertyCombobox.currentItem();

    if (editMode)
        SaveManager::changeBindingSave(editingBindingName, binf);
    else
        SaveManager::addBindingSave(binf);

    parent->clearList();
    parent->handleSelectionChange();
    bindingListWidget.clearSelection();
    popupHideButtonClicked();
}

void BindingWidgetPrivate::ensureComboboxVisible(const QObject* obj)
{
    popupScrollArea.ensureWidgetVisible((QWidget*)obj);
}

void BindingWidgetPrivate::updateSourcePropertyCombobox()
{
    sourcePropertyCombobox.clear();
    if (targetPropertyCombobox.currentItem().isEmpty() ||
        sourceItemCombobox.currentItem().isEmpty())
        return;

    auto scene = DesignerScene::instance();
    auto selectedControls = scene->selectedControls();

    if (scene->currentPage()->isSelected())
        selectedControls << scene->currentPage();

    if (selectedControls.isEmpty() ||
        selectedControls.size() > 1 ||
        selectedControls[0]->id().isEmpty()) {
        return;
    }

    QVariant::Type targetType = selectedControls[0]->properties()[targetPropertyCombobox.currentItem()];

    QStringList sourcePropertyList;
    if (scene->currentPage()->id() == sourceItemCombobox.currentItem()) {
        for (auto property : scene->currentPage()->properties().keys())
            if (scene->currentPage()->properties()[property] == targetType)
                sourcePropertyList << property;
    } else {
        for (auto control : scene->currentPage()->childControls())
            if (control->id() == sourceItemCombobox.currentItem()) {
                for (auto property : control->properties().keys())
                    if (control->properties()[property] == targetType)
                        sourcePropertyList << property;
                break;
            }
    }

    for (auto property : sourcePropertyList)
        sourcePropertyCombobox.addItem(property);
}

BindingWidgetPrivate* BindingWidget::m_d = nullptr;

BindingWidget::BindingWidget(QWidget *parent)
    : QWidget(parent)
{
    if (m_d) return;
    m_d = new BindingWidgetPrivate(this);
}

BindingWidget* BindingWidget::instance()
{
    return m_d->parent;
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
    m_d->nameEdit.setText("");
    m_d->popupItemNameTextBox.setText("");
}

void BindingWidget::detachBindingsFor(Control* control)
{
    auto saves = SaveManager::getBindingSaves();
    for (auto key : saves.keys()) {
        auto save = saves[key].toObject();
        if (save[BINDING_SOURCE_ID_LABEL] == control->id() ||
            save[BINDING_TARGET_ID_LABEL] == control->id()) {
            SaveManager::removeBindingSave(key);
            for (int i=0; i < m_d->bindingListWidget.count(); i++) {
                if (m_d->bindingListWidget.item(i)->text() == key)
                    m_d->bindingListWidget.takeItem(i);
            }
        }
    }
}

void BindingWidget::clearAllBindings()
{
    auto saves = SaveManager::getBindingSaves();
    for (auto key : saves.keys()) {
        SaveManager::removeBindingSave(key);
    }
    m_d->bindingListWidget.clear();
    clearList();
}

void BindingWidget::addBindingWithoutSave(const SaveManager::BindingInf& inf)
{
    m_d->addBindingWithoutSave(inf);
}

bool BindingWidget::hasPopupOpen()
{
    return m_d->hasPopupOpen;
}

void BindingWidget::handleSelectionChange()
{
    auto scene = DesignerScene::instance();
    auto selectedControls = scene->selectedControls();
    if (scene->currentPage()->isSelected())
        selectedControls << scene->currentPage();

    clearList();
    if (selectedControls.isEmpty() ||
        selectedControls.size() > 1 ||
        selectedControls[0]->id().isEmpty()) {
        return;
    } else {
        m_d->popupItemNameTextBox.setText(selectedControls[0]->id());
        for (auto property : selectedControls[0]->properties().keys())
            m_d->targetPropertyCombobox.addItem(property);
        for (auto page : scene->pages())
            m_d->sourceItemCombobox.addItem(page->id());
        for (auto control : scene->controls())
            m_d->sourceItemCombobox.addItem(control->id());
    }
}
