#include <preferenceswindow.h>
#include <focuslesslineedit.h>
#include <QtWidgets>

PreferencesWindow::PreferencesWindow(QWidget *parent) : QWidget(parent)
{
    setWindowTitle(tr("Preferences"));
    m_scrollArea = new QScrollArea;
    m_scrollArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    m_listWidget = new QListWidget;
    m_listWidget->setFixedWidth(150);
    m_listWidget->setIconSize({24, 24});
    m_listWidget->setFocusPolicy(Qt::NoFocus);
    m_listWidget->setSortingEnabled(true);
    m_listWidget->setTextElideMode(Qt::ElideMiddle);
    m_listWidget->setDragEnabled(false);
    m_listWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_listWidget->setDragDropMode(QAbstractItemView::NoDragDrop);
    m_listWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_listWidget->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    m_listWidget->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);

    QFont f;
    f.setWeight(QFont::Medium);
    f.setPixelSize(14);
    m_lblTitle = new QLabel;
    m_lblTitle->setFont(f);
    m_lblTitle->setText("Title");

    m_txtFilter = new FocuslessLineEdit;
    m_txtFilter->setPlaceholderText("Filter");
    m_txtFilter->setClearButtonEnabled(true);
    m_txtFilter->setFixedWidth(150);
    m_txtFilter->setFixedHeight(22);
    connect(m_txtFilter, SIGNAL(textChanged(QString)), SLOT(filterList(QString)));

    m_bboxButtons = new QDialogButtonBox;
    m_bboxButtons->setStandardButtons(QDialogButtonBox::Cancel | QDialogButtonBox::Save);
    connect(m_bboxButtons->button(QDialogButtonBox::Cancel), SIGNAL(clicked(bool)), SIGNAL(done()));

    m_bboxButtons->button(QDialogButtonBox::Cancel)->setCursor(Qt::PointingHandCursor);
    m_bboxButtons->button(QDialogButtonBox::Save)->setCursor(Qt::PointingHandCursor);

    m_layout = new QGridLayout(this);
    m_layout->setSpacing(6);
    m_layout->setContentsMargins(10, 10, 10, 10);
    m_layout->addWidget(m_txtFilter, 0, 0);
    m_layout->addWidget(m_lblTitle, 0, 1);
    m_layout->addWidget(m_listWidget, 1, 0);
    m_layout->addWidget(m_scrollArea, 1, 1);
    m_layout->addWidget(m_bboxButtons, 2, 0, 1, 2);
}

void PreferencesWindow::filterList(const QString& /*text*/)
{

}

QSize PreferencesWindow::sizeHint() const
{
    return {1160, 670};
}
