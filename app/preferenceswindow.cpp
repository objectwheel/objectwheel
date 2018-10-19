#include <preferenceswindow.h>
#include <focuslesslineedit.h>
#include <generalsettingspage.h>

#include <QListWidget>
#include <QGridLayout>
#include <QPushButton>
#include <QDialogButtonBox>

enum Roles { SettingsPageRole = Qt::UserRole + 1 };

PreferencesWindow::PreferencesWindow(QWidget *parent) : QWidget(parent)
  , m_layout(new QGridLayout(this))
  , m_listWidget(new QListWidget(this))
  , m_searchLineEdit(new FocuslessLineEdit(this))
  , m_dialogButtonBox(new QDialogButtonBox(this))
{
    setWindowTitle(tr("Preferences"));

    m_layout->setSpacing(6);
    m_layout->setContentsMargins(10, 10, 10, 10);
    m_layout->addWidget(m_searchLineEdit, 0, 0, 1, 1);
    m_layout->addWidget(m_listWidget, 1, 0, 1, 1);
    m_layout->addWidget(m_dialogButtonBox, 2, 0, 1, 2);

    m_listWidget->setFixedWidth(170);
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

    m_searchLineEdit->setFixedWidth(170);
    m_searchLineEdit->setFixedHeight(22);
    m_searchLineEdit->setPlaceholderText(tr("Filter"));
    m_searchLineEdit->setClearButtonEnabled(true);
    connect(m_searchLineEdit, &FocuslessLineEdit::textEdited,
            this, &PreferencesWindow::search);

    m_dialogButtonBox->setStandardButtons(QDialogButtonBox::Cancel | QDialogButtonBox::Apply | QDialogButtonBox::Ok);
    connect(m_dialogButtonBox->button(QDialogButtonBox::Cancel), &QPushButton::clicked,
            this, &PreferencesWindow::done);

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

    addPage(new GeneralSettingsPage(this));
}

void PreferencesWindow::search(const QString& /*text*/)
{
    // TODO
}

void PreferencesWindow::addPage(SettingsPage* page)
{
    if (page->parentWidget() != this)
        page->setParent(this);

    page->hide();

    auto item = new QListWidgetItem(page->icon(), page->title());
    item->setData(SettingsPageRole, QVariant::fromValue<SettingsPage*>(page));

    m_listWidget->addItem(item);
}

void PreferencesWindow::setCurrentPage(SettingsPage* page, SettingsPage* previous)
{
    if (previous) {
        m_layout->removeWidget(previous);
        previous->hide();
    }
    for (int i = 0; i < m_listWidget->count(); ++i) {
        if (m_listWidget->item(i)->data(SettingsPageRole).value<SettingsPage*>() == page) {
            m_layout->addWidget(page, 0, 1, 2, 1);
            page->show();
        }
    }
}

QSize PreferencesWindow::sizeHint() const
{
    return {850, 480};
}
