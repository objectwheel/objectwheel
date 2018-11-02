#include <preferenceswindow.h>
#include <focuslesslineedit.h>
#include <generalsettingspage.h>
#include <codeeditorsettingspage.h>
#include <generalsettings.h>
#include <interfacesettings.h>
#include <utilityfunctions.h>

#include <QListWidget>
#include <QGridLayout>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QCloseEvent>
#include <QShowEvent>

enum Roles { SettingsPageRole = Qt::UserRole + 1 };

namespace {

void setPageForItem(QListWidgetItem* item, SettingsPage* page)
{
    item->setData(SettingsPageRole, QVariant::fromValue<SettingsPage*>(page));
}

SettingsPage* pageFromItem(QListWidgetItem* item)
{
    if (!item)
        return nullptr;
    return item->data(SettingsPageRole).value<SettingsPage*>();
}
}

PreferencesWindow::PreferencesWindow(QWidget *parent) : QWidget(parent)
  , m_layout(new QGridLayout(this))
  , m_listWidget(new QListWidget(this))
  , m_searchLineEdit(new FocuslessLineEdit(this))
  , m_dialogButtonBox(new QDialogButtonBox(this))
{
    setWindowTitle(tr("Preferences"));
    setAttribute(Qt::WA_QuitOnClose, false);

    addPage(new GeneralSettingsPage(this));
    addPage(new CodeEditorSettingsPage(this));

    m_layout->setSpacing(6);
    m_layout->setContentsMargins(10, 10, 10, 10);
    m_layout->addWidget(m_searchLineEdit, 0, 0, 1, 1);
    m_layout->addWidget(m_listWidget, 1, 0, 1, 1);
    m_layout->addWidget(m_dialogButtonBox, 2, 0, 1, 2);

    m_listWidget->setFixedWidth(150);
    m_listWidget->setIconSize({24, 24});
    m_listWidget->setDragEnabled(false);
    m_listWidget->setSortingEnabled(true);
    m_listWidget->setFocusPolicy(Qt::NoFocus);
    m_listWidget->setTextElideMode(Qt::ElideMiddle);
    m_listWidget->setDragDropMode(QAbstractItemView::NoDragDrop);
    m_listWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_listWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_listWidget->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    m_listWidget->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);

    m_searchLineEdit->setFixedWidth(m_listWidget->width());
    m_searchLineEdit->setFixedHeight(22);
    m_searchLineEdit->setPlaceholderText(tr("Filter"));
    m_searchLineEdit->setClearButtonEnabled(true);
    connect(m_searchLineEdit, &FocuslessLineEdit::textEdited,
            this, &PreferencesWindow::search);

    m_dialogButtonBox->setStandardButtons(QDialogButtonBox::Cancel | QDialogButtonBox::Apply | QDialogButtonBox::Ok);
    m_dialogButtonBox->button(QDialogButtonBox::Ok)->setCursor(Qt::PointingHandCursor);
    m_dialogButtonBox->button(QDialogButtonBox::Apply)->setCursor(Qt::PointingHandCursor);
    m_dialogButtonBox->button(QDialogButtonBox::Cancel)->setCursor(Qt::PointingHandCursor);

    connect(m_listWidget, &QListWidget::currentItemChanged,
            this, [=] (QListWidgetItem* curr, QListWidgetItem* prev) {
        SettingsPage* current = curr ? curr->data(SettingsPageRole).value<SettingsPage*>() : nullptr;
        SettingsPage* previous = prev ? prev->data(SettingsPageRole).value<SettingsPage*>() : nullptr;

        Q_ASSERT(current);

        setCurrentPage(current, previous);
    });

    connect(m_dialogButtonBox->button(QDialogButtonBox::Cancel), &QPushButton::clicked,
            this, &PreferencesWindow::done);
    connect(m_dialogButtonBox->button(QDialogButtonBox::Cancel), &QPushButton::clicked,
            this, &PreferencesWindow::reset);
    connect(m_dialogButtonBox->button(QDialogButtonBox::Apply), &QPushButton::clicked,
            this, &PreferencesWindow::apply);
    connect(m_dialogButtonBox->button(QDialogButtonBox::Apply), &QPushButton::clicked,
            this, &PreferencesWindow::activateCurrent);
    connect(m_dialogButtonBox->button(QDialogButtonBox::Ok), &QPushButton::clicked,
            this, &PreferencesWindow::apply);
    connect(m_dialogButtonBox->button(QDialogButtonBox::Ok), &QPushButton::clicked,
            this, &PreferencesWindow::done);
    connect(GeneralSettings::instance(), &GeneralSettings::designerStateReset,
            this, &PreferencesWindow::resetSettings);
}

void PreferencesWindow::apply()
{
    for (int i = 0; i < m_listWidget->count(); ++i) {
        if (SettingsPage* page = pageFromItem(m_listWidget->item(i)))
            page->apply();
    }
}

void PreferencesWindow::reset()
{
    for (int i = 0; i < m_listWidget->count(); ++i) {
        if (SettingsPage* page = pageFromItem(m_listWidget->item(i)))
            page->reset();
    }
}

void PreferencesWindow::activateCurrent()
{
    if (SettingsPage* page = pageFromItem(m_listWidget->currentItem()))
        page->activateCurrent();
}

void PreferencesWindow::search(const QString& word)
{
    for (int i = 0; i < m_listWidget->count(); ++i) {
        if (SettingsPage* page = pageFromItem(m_listWidget->item(i)))
            m_listWidget->item(i)->setHidden(!word.isEmpty() && !page->containsWord(word));
    }
    QListWidgetItem* item = m_listWidget->itemAt({0, 0});
    if (item && m_listWidget->currentItem()->isHidden())
        m_listWidget->setCurrentItem(item);
}

void PreferencesWindow::addPage(SettingsPage* page)
{
    if (!page)
        return;

    if (page->parentWidget() != this)
        page->setParent(this);

    page->hide();

    auto item = new QListWidgetItem(page->icon(), page->title());
    setPageForItem(item, page);

    m_listWidget->addItem(item);
}

void PreferencesWindow::setCurrentPage(SettingsPage* page, SettingsPage* previous)
{
    if (previous) {
        m_layout->removeWidget(previous);
        previous->hide();
    }
    for (int i = 0; i < m_listWidget->count(); ++i) {
        if (pageFromItem(m_listWidget->item(i)) == page) {
            m_layout->addWidget(page, 0, 1, 2, 1);
            page->show();
            page->activateCurrent();
        }
    }
}

void PreferencesWindow::resetSettings()
{
    InterfaceSettings* settings = GeneralSettings::interfaceSettings();
    settings->begin();
    settings->setValue("PreferencesWindow.Size", sizeHint());
    settings->setValue("PreferencesWindow.Position", UtilityFunctions::centerPos(sizeHint()));
    settings->setValue("PreferencesWindow.Maximized", false);
    settings->setValue("PreferencesWindow.Fullscreen", false);
    settings->end();

    if (isVisible())
        readSettings();
}

void PreferencesWindow::readSettings()
{
    InterfaceSettings* settings = GeneralSettings::interfaceSettings();
    settings->begin();
    resize(settings->value<QSize>("PreferencesWindow.Size", sizeHint()));
    move(settings->value<QPoint>("PreferencesWindow.Position", UtilityFunctions::centerPos(size())));
    if (settings->value<bool>("PreferencesWindow.Fullscreen", false))
        showFullScreen();
    else if (settings->value<bool>("PreferencesWindow.Maximized", false))
        showMaximized();
    else
        showNormal();
    settings->end();
}

void PreferencesWindow::writeSettings()
{
    InterfaceSettings* settings = GeneralSettings::interfaceSettings();
    settings->begin();
    settings->setValue("PreferencesWindow.Size", size());
    settings->setValue("PreferencesWindow.Position", pos());
    settings->setValue("PreferencesWindow.Maximized", isMaximized());
    settings->setValue("PreferencesWindow.Fullscreen", isFullScreen());
    settings->end();
}

void PreferencesWindow::showEvent(QShowEvent* event)
{
    readSettings();
    QWidget::showEvent(event);
    if (event->isAccepted()) {
        if (!m_listWidget->currentItem())
            m_listWidget->setCurrentRow(0);
        activateCurrent();
    }
}

void PreferencesWindow::closeEvent(QCloseEvent* event)
{
    if (GeneralSettings::interfaceSettings()->preserveDesignerState)
        writeSettings();
    QWidget::closeEvent(event);
    if (event->isAccepted())
        reset();
}

QSize PreferencesWindow::sizeHint() const
{
    return {850, 480};
}
