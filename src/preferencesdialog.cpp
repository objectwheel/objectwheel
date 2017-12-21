#include <preferencesdialog.h>
#include <filterlineedit.h>
#include <fit.h>
#include <css.h>
#include <QtWidgets>

PreferencesDialog::PreferencesDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle("Preferences");

    QPalette p(palette());
    p.setColor(backgroundRole(), "#e0e4e7");
    setPalette(p);

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

    QPalette p2(palette());
    p2.setColor(QPalette::Base, "#F3F7FA");
    p2.setColor(QPalette::Highlight, "#d0d4d7");
    p2.setColor(QPalette::Text, "#202427");
    listWidget->setPalette(p2);

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
    connect(bboxButtons->button(QDialogButtonBox::Cancel),
      SIGNAL(clicked(bool)), SLOT(close()));

    bboxButtons->button(QDialogButtonBox::Cancel)->setCursor(Qt::PointingHandCursor);
    bboxButtons->button(QDialogButtonBox::Save)->setCursor(Qt::PointingHandCursor);

    layout = new QGridLayout(this);
    layout->setSpacing(fit::fx(6));
    layout->setContentsMargins(fit::fx(10), fit::fx(10), fit::fx(10), fit::fx(10));
    layout->addWidget(txtFilter, 0, 0);
    layout->addWidget(lblTitle, 0, 1);
    layout->addWidget(listWidget, 1, 0);
    layout->addWidget(scrollArea, 1, 1);
    layout->addWidget(bboxButtons, 2, 0, 1, 2);
}

QSize PreferencesDialog::sizeHint() const
{
    return fit::fx(QSizeF{900, 550}).toSize();
}

void PreferencesDialog::filterList(const QString& text)
{

}
