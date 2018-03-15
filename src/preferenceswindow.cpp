#include <preferenceswindow.h>
#include <focuslesslineedit.h>
#include <fit.h>
#include <css.h>
#include <QtWidgets>

PreferencesWindow::PreferencesWindow(QWidget *parent) : QWidget(parent)
{
    setWindowTitle(tr("Preferences"));

    QPalette p(palette());
    p.setColor(backgroundRole(), "#e0e4e7");
    setPalette(p);

    _scrollArea = new QScrollArea;
    _scrollArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    _listWidget = new QListWidget;
    _listWidget->setFixedWidth(fit::fx(150));
    _listWidget->setIconSize(fit::fx(QSize{24, 24}));
    _listWidget->setFocusPolicy(Qt::NoFocus);
    _listWidget->setSortingEnabled(true);
    _listWidget->setTextElideMode(Qt::ElideMiddle);
    _listWidget->verticalScrollBar()->setStyleSheet(CSS::ScrollBar);
    _listWidget->setDragEnabled(false);
    _listWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    _listWidget->setDragDropMode(QAbstractItemView::NoDragDrop);
    _listWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    _listWidget->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    _listWidget->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);

    QPalette p2(palette());
    p2.setColor(QPalette::Base, "#F5F9FC");
    p2.setColor(QPalette::Highlight, "#d0d4d7");
    p2.setColor(QPalette::Text, "#202427");
    _listWidget->setPalette(p2);

    QFont f;
    f.setWeight(QFont::Medium);
    f.setPixelSize(fit::fx(14));
    _lblTitle = new QLabel;
    _lblTitle->setFont(f);
    _lblTitle->setText("Title");

    _txtFilter = new FocuslessLineEdit;
    _txtFilter->setPlaceholderText("Filter");
    _txtFilter->setClearButtonEnabled(true);
    _txtFilter->setFixedWidth(fit::fx(150));
    _txtFilter->setFixedHeight(fit::fx(22));
    connect(_txtFilter, SIGNAL(textChanged(QString)), SLOT(filterList(QString)));

    _bboxButtons = new QDialogButtonBox;
    _bboxButtons->setStandardButtons(QDialogButtonBox::Cancel | QDialogButtonBox::Save);
    connect(_bboxButtons->button(QDialogButtonBox::Cancel), SIGNAL(clicked(bool)), SIGNAL(done()));

    _bboxButtons->button(QDialogButtonBox::Cancel)->setCursor(Qt::PointingHandCursor);
    _bboxButtons->button(QDialogButtonBox::Save)->setCursor(Qt::PointingHandCursor);

    _layout = new QGridLayout(this);
    _layout->setSpacing(fit::fx(6));
    _layout->setContentsMargins(fit::fx(10), fit::fx(10), fit::fx(10), fit::fx(10));
    _layout->addWidget(_txtFilter, 0, 0);
    _layout->addWidget(_lblTitle, 0, 1);
    _layout->addWidget(_listWidget, 1, 0);
    _layout->addWidget(_scrollArea, 1, 1);
    _layout->addWidget(_bboxButtons, 2, 0, 1, 2);
}

void PreferencesWindow::filterList(const QString& /*text*/)
{

}
