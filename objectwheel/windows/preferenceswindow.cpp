#include <preferenceswindow.h>
#include <lineedit.h>
#include <generalsettingspage.h>
#include <codeeditorsettingspage.h>
#include <designersettingspage.h>
#include <utilityfunctions.h>
#include <paintutils.h>

#include <QListWidget>
#include <QGridLayout>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QShowEvent>

enum Roles { SettingsPageRole = Qt::UserRole + 1 };

static void setPageForItem(QListWidgetItem* item, SettingsPage* page)
{
    item->setData(SettingsPageRole, QVariant::fromValue<SettingsPage*>(page));
}

static SettingsPage* pageFromItem(QListWidgetItem* item)
{
    if (!item)
        return nullptr;
    return item->data(SettingsPageRole).value<SettingsPage*>();
}

PreferencesWindow::PreferencesWindow(QWidget *parent) : QWidget(parent)
  , m_layout(new QGridLayout(this))
  , m_listWidget(new QListWidget(this))
  , m_searchLineEdit(new LineEdit(LineEdit::NoFocus, LineEdit::Search, this))
  , m_dialogButtonBox(new QDialogButtonBox(this))
{
    setFocusPolicy(Qt::NoFocus);
    setWindowTitle(tr("Preferences"));
    setWindowModality(Qt::ApplicationModal);
    setAttribute(Qt::WA_QuitOnClose, false);

    addPage(new GeneralSettingsPage(this));
    addPage(new DesignerSettingsPage(this));
    addPage(new CodeEditorSettingsPage(this));

    m_layout->setSpacing(6);
    m_layout->setContentsMargins(10, 10, 10, 10);
    m_layout->addWidget(m_searchLineEdit, 0, 0, 1, 1);
    m_layout->addWidget(m_listWidget, 1, 0, 1, 1);
    m_layout->addWidget(m_dialogButtonBox, 2, 0, 1, 2);

    m_listWidget->setIconSize(QSize(20, 20));
    m_listWidget->setFixedWidth(150);
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
    m_searchLineEdit->setPlaceholderText(tr("Search"));
    m_searchLineEdit->setClearButtonEnabled(true);
    m_searchLineEdit->addAction(PaintUtils::renderOverlaidPixmap(":/images/search.svg", "#595959", QSize(16, 16), this),
                            QLineEdit::LeadingPosition);

    m_dialogButtonBox->setStandardButtons(QDialogButtonBox::Cancel | QDialogButtonBox::Reset
                                          | QDialogButtonBox::Apply | QDialogButtonBox::Ok);
    m_dialogButtonBox->button(QDialogButtonBox::Ok)->setCursor(Qt::PointingHandCursor);
    m_dialogButtonBox->button(QDialogButtonBox::Apply)->setCursor(Qt::PointingHandCursor);
    m_dialogButtonBox->button(QDialogButtonBox::Reset)->setCursor(Qt::PointingHandCursor);
    m_dialogButtonBox->button(QDialogButtonBox::Cancel)->setCursor(Qt::PointingHandCursor);

    resize(sizeHint());
    move(UtilityFunctions::centerPos(size()));

    connect(m_searchLineEdit, &LineEdit::textEdited,
            this, [=] { search(m_searchLineEdit->text()); });
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
            this, &PreferencesWindow::revert);
    connect(m_dialogButtonBox->button(QDialogButtonBox::Reset), &QPushButton::clicked,
            this, &PreferencesWindow::reset);
    connect(m_dialogButtonBox->button(QDialogButtonBox::Apply), &QPushButton::clicked,
            this, &PreferencesWindow::apply);
    connect(m_dialogButtonBox->button(QDialogButtonBox::Apply), &QPushButton::clicked,
            this, &PreferencesWindow::activateCurrent);
    connect(m_dialogButtonBox->button(QDialogButtonBox::Ok), &QPushButton::clicked,
            this, &PreferencesWindow::apply);
    connect(m_dialogButtonBox->button(QDialogButtonBox::Ok), &QPushButton::clicked,
            this, &PreferencesWindow::done);
}

void PreferencesWindow::setCurrentWidget(PreferencesWindow::Widget w)
{
    SettingsPage* current = page(w);
    Q_ASSERT(current);
    for (int i = 0; i < m_listWidget->count(); ++i) {
        if (SettingsPage* page = pageFromItem(m_listWidget->item(i))) {
            if (page == current) {
                m_listWidget->setCurrentRow(i);
                break;
            }
        }
    }
    current->setWidgetVisible(widget(w));
}

SettingsPage* PreferencesWindow::page(PreferencesWindow::Widget w) const
{
    SettingsWidget* wid = widget(w);
    if (wid == 0)
        return nullptr;
    for (int i = 0; i < m_listWidget->count(); ++i) {
        if (SettingsPage* page = pageFromItem(m_listWidget->item(i))) {
            const QList<SettingsWidget*>& widgets = page->widgets();
            for (SettingsWidget* widget : widgets) {
                if (widget == wid)
                    return page;
            }
        }
    }
    return nullptr;
}

SettingsWidget* PreferencesWindow::widget(PreferencesWindow::Widget w) const
{
    for (int i = 0; i < m_listWidget->count(); ++i) {
        if (SettingsPage* page = pageFromItem(m_listWidget->item(i))) {
            const QList<SettingsWidget*>& widgets = page->widgets();
            for (SettingsWidget* widget : widgets) {
                switch (w) {
                case ToolboxSettingsWidget:
                    if (widget->metaObject()->className() == QStringLiteral("ToolboxSettingsWidget"))
                        return widget;
                    break;
                case SceneSettingsWidget:
                    if (widget->metaObject()->className() == QStringLiteral("SceneSettingsWidget"))
                        return widget;
                    break;
                case InterfaceSettingsWidget:
                    if (widget->metaObject()->className() == QStringLiteral("InterfaceSettingsWidget"))
                        return widget;
                    break;
                case FontColorsSettingsWidget:
                    if (widget->metaObject()->className() == QStringLiteral("FontColorsSettingsWidget"))
                        return widget;
                    break;
                case BehaviorSettingsWidget:
                    if (widget->metaObject()->className() == QStringLiteral("BehaviorSettingsWidget"))
                        return widget;
                    break;
                default:
                    break;
                }
            }
        }
    }
    return nullptr;
}

void PreferencesWindow::apply()
{
    for (int i = 0; i < m_listWidget->count(); ++i) {
        if (SettingsPage* page = pageFromItem(m_listWidget->item(i)))
            page->apply();
    }
}

void PreferencesWindow::revert()
{
    for (int i = 0; i < m_listWidget->count(); ++i) {
        if (SettingsPage* page = pageFromItem(m_listWidget->item(i)))
            page->revert();
    }
}

void PreferencesWindow::reset()
{

    int ret = UtilityFunctions::showMessage(this, tr("Reset all the settings"),
                                            tr("This will reset all the settings to the "
                                               "default values. Some may take affect after "
                                               "restarting the app. Are you sure to continue?"),
                                            QMessageBox::Warning, QMessageBox::Yes |
                                            QMessageBox::No, QMessageBox::No);
    if (ret == QMessageBox::Yes) {
        for (int i = 0; i < m_listWidget->count(); ++i) {
            if (SettingsPage* page = pageFromItem(m_listWidget->item(i)))
                page->reset();
        }
        emit done();
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

void PreferencesWindow::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);

    if (event->isAccepted()) {
        if (!m_listWidget->currentItem())
            m_listWidget->setCurrentRow(0);
        activateCurrent();
    }
}

void PreferencesWindow::closeEvent(QCloseEvent* event)
{
    revert();
    QWidget::closeEvent(event);
}

QSize PreferencesWindow::sizeHint() const
{
    return {780, 460};
}
