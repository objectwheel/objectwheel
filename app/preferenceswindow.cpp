#include <preferenceswindow.h>
#include <focuslesslineedit.h>
#include <settingspage.h>

#include <QListWidget>
#include <QGridLayout>
#include <QPushButton>
#include <QDialogButtonBox>

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
    // m_layout->addWidget(m_scrollArea, 0, 1, 1, 2);
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
}

void PreferencesWindow::search(const QString& /*text*/)
{
    // TODO
}

QSize PreferencesWindow::sizeHint() const
{
    return {850, 480};
}

void PreferencesWindow::addPage(SettingsPage* page)
{
    m_settingsPages.insert(page->title(), page);
}
