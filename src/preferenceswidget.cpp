#include <preferenceswidget.h>
#include <filterlineedit.h>
#include <fit.h>
#include <css.h>
#include <QtWidgets>

PreferencesWidget::PreferencesWidget(QWidget *parent) : QWidget(parent)
{
    setWindowTitle("Preferences");

    scrollArea = new QScrollArea;
    scrollArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    listWidget = new QListWidget;
    listWidget->setFixedWidth(fit::fx(150));
    listWidget->setIconSize(fit::fx(QSize{24, 24}));
    listWidget->setFocusPolicy(Qt::NoFocus);
    listWidget->setSortingEnabled(true);
    listWidget->setTextElideMode(Qt::ElideMiddle);
    listWidget->verticalScrollBar()->setStyleSheet(CSS::ScrollBar);
    listWidget->setDragEnabled(false);
    listWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    listWidget->setDragDropMode(QAbstractItemView::NoDragDrop);
    listWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    listWidget->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    listWidget->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);

    QFont f;
    f.setWeight(QFont::Medium);
    f.setPixelSize(fit::fx(14));
    lblTitle = new QLabel;
    lblTitle->setFont(f);
    lblTitle->setText("Title");

    txtFilter = new FilterLineEdit;
    txtFilter->setPlaceholderText("Filter");
    txtFilter->setClearButtonEnabled(true);
    txtFilter->setFixedWidth(fit::fx(150));
    txtFilter->setFixedHeight(fit::fx(22));
    connect(txtFilter, SIGNAL(textChanged(QString)), SLOT(filterList(QString)));

    bboxButtons = new QDialogButtonBox;
    bboxButtons->setStandardButtons(QDialogButtonBox::Cancel | QDialogButtonBox::Save);

    layout = new QGridLayout(this);
    layout->setSpacing(fit::fx(6));
    layout->setContentsMargins(fit::fx(10), fit::fx(10), fit::fx(10), fit::fx(10));
    layout->addWidget(txtFilter, 0, 0);
    layout->addWidget(lblTitle, 0, 1);
    layout->addWidget(listWidget, 1, 0);
    layout->addWidget(scrollArea, 1, 1);
    layout->addWidget(bboxButtons, 2, 0, 1, 2);
}

QSize PreferencesWidget::sizeHint() const
{
    return fit::fx(QSizeF{800, 600}).toSize();
}

void PreferencesWidget::filterList(const QString& text)
{

}
