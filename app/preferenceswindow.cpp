#include <preferenceswindow.h>
#include <focuslesslineedit.h>
#include <QtWidgets>

PreferencesWindow::PreferencesWindow(QWidget *parent) : QWidget(parent)
{
    setWindowTitle(tr("Preferences"));
    _scrollArea = new QScrollArea;
    _scrollArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    _listWidget = new QListWidget;
    _listWidget->setFixedWidth(150);
    _listWidget->setIconSize({24, 24});
    _listWidget->setFocusPolicy(Qt::NoFocus);
    _listWidget->setSortingEnabled(true);
    _listWidget->setTextElideMode(Qt::ElideMiddle);
    _listWidget->setDragEnabled(false);
    _listWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    _listWidget->setDragDropMode(QAbstractItemView::NoDragDrop);
    _listWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    _listWidget->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    _listWidget->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);

    QFont f;
    f.setWeight(QFont::Medium);
    f.setPixelSize(14);
    _lblTitle = new QLabel;
    _lblTitle->setFont(f);
    _lblTitle->setText("Title");

    _txtFilter = new FocuslessLineEdit;
    _txtFilter->setPlaceholderText("Filter");
    _txtFilter->setClearButtonEnabled(true);
    _txtFilter->setFixedWidth(150);
    _txtFilter->setFixedHeight(22);
    connect(_txtFilter, SIGNAL(textChanged(QString)), SLOT(filterList(QString)));

    _bboxButtons = new QDialogButtonBox;
    _bboxButtons->setStandardButtons(QDialogButtonBox::Cancel | QDialogButtonBox::Save);
    connect(_bboxButtons->button(QDialogButtonBox::Cancel), SIGNAL(clicked(bool)), SIGNAL(done()));

    _bboxButtons->button(QDialogButtonBox::Cancel)->setCursor(Qt::PointingHandCursor);
    _bboxButtons->button(QDialogButtonBox::Save)->setCursor(Qt::PointingHandCursor);

    _layout = new QGridLayout(this);
    _layout->setSpacing(6);
    _layout->setContentsMargins(10, 10, 10, 10);
    _layout->addWidget(_txtFilter, 0, 0);
    _layout->addWidget(_lblTitle, 0, 1);
    _layout->addWidget(_listWidget, 1, 0);
    _layout->addWidget(_scrollArea, 1, 1);
    _layout->addWidget(_bboxButtons, 2, 0, 1, 2);
}

void PreferencesWindow::filterList(const QString& /*text*/)
{

}

QSize PreferencesWindow::sizeHint() const
{
    return {1160, 670};
}
