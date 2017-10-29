#include <outputbox.h>
#include <flatbutton.h>
#include <css.h>
#include <fit.h>

#include <QSplitterHandle>
#include <QToolBar>
#include <QButtonGroup>
#include <QVBoxLayout>

#define HEIGHT_MIN (fit(100))
#define HEIGHT_MAX (fit(600))
#define SIZE_INITIAL (QSize(fit(300), fit(140)))

using namespace Fit;

class OutputBoxPrivate : public QObject
{
        Q_OBJECT
    public:
        OutputBoxPrivate(OutputBox *parent);

    public slots:
        void handleHideButtonClicked();
        void handleIssuesButtonClicked(bool val);
        void handleSearchButtonClicked(bool val);
        void handleConsoleButtonClicked(bool val);

    public:
        OutputBox* parent;
        QMap<BoxType, QWidget*> boxes;
        QVBoxLayout* layout;
        QToolBar* toolbar;
        FlatButton* hideButton;
        FlatButton* issuesButton;
        FlatButton* searchButton;
        FlatButton* consoleButton;
};

OutputBoxPrivate::OutputBoxPrivate(OutputBox* parent)
    : QObject(parent)
    , parent(parent)
    , layout(new QVBoxLayout(parent))
    , toolbar(new QToolBar)
    , hideButton(new FlatButton)
    , issuesButton(new FlatButton)
    , searchButton(new FlatButton)
    , consoleButton(new FlatButton)
{
    auto buttonGroup = new QButtonGroup;
    buttonGroup->addButton(issuesButton);
    buttonGroup->addButton(searchButton);
    buttonGroup->addButton(consoleButton);

    hideButton->setFixedWidth(16);
    hideButton->setFixedHeight(16);
    hideButton->setIconSize(QSize(fit(14), fit(14)));
    hideButton->setRadius(fit(6));
    hideButton->setCursor(Qt::PointingHandCursor);
    hideButton->setToolTip("Hide bar.");
    hideButton->setIcon(QIcon(":/resources/images/down-arrow.png"));
    hideButton->setColor("#495866");
    hideButton->setCheckedColor("#363F47");
    connect(hideButton, SIGNAL(clicked(bool)),
      SLOT(handleHideButtonClicked()));

    issuesButton->setCheckable(true);
    issuesButton->setText("Issues");
    issuesButton->setFixedWidth(100);
    issuesButton->setFixedHeight(20);
    issuesButton->setCursor(Qt::PointingHandCursor);
    issuesButton->setToolTip("Show recent issues.");
    issuesButton->setIcon(QIcon(":/resources/images/issues.png"));
    issuesButton->setColor("#495866");
    issuesButton->setCheckedColor("#363F47");
    issuesButton->setCheckedTextColor("#f0f4f7");
    issuesButton->setTextColor("#f0f4f7");
    issuesButton->setChecked(true);
    connect(issuesButton, SIGNAL(toggled(bool)),
      SLOT(handleIssuesButtonClicked(bool)));

    searchButton->setCheckable(true);
    searchButton->setText("Search");
    searchButton->setFixedWidth(100);
    searchButton->setFixedHeight(20);
    searchButton->setCursor(Qt::PointingHandCursor);
    searchButton->setToolTip("Search words within project.");
    searchButton->setIcon(QIcon(":/resources/images/search.png"));
    searchButton->setColor("#495866");
    searchButton->setCheckedColor("#363F47");
    searchButton->setCheckedTextColor("#f0f4f7");
    searchButton->setTextColor("#f0f4f7");
    connect(searchButton, SIGNAL(toggled(bool)),
      SLOT(handleSearchButtonClicked(bool)));

    consoleButton->setCheckable(true);
    consoleButton->setText("Console Output");
    consoleButton->setFixedWidth(150);
    consoleButton->setFixedHeight(20);
    consoleButton->setCursor(Qt::PointingHandCursor);
    consoleButton->setToolTip("Show application output.");
    consoleButton->setIcon(QIcon(":/resources/images/console.png"));
    consoleButton->setColor("#495866");
    consoleButton->setCheckedColor("#363F47");
    consoleButton->setCheckedTextColor("#f0f4f7");
    consoleButton->setTextColor("#f0f4f7");
    connect(consoleButton, SIGNAL(toggled(bool)),
      SLOT(handleConsoleButtonClicked(bool)));

    auto lspacer = new QWidget;
    lspacer->setFixedSize(fit(1), fit(1));
    auto spacer = new QWidget;
    spacer->setSizePolicy(QSizePolicy::Expanding,
      QSizePolicy::Expanding);
    auto rspacer = new QWidget;
    rspacer->setFixedSize(fit(1), fit(1));

    toolbar->setStyleSheet(CSS::DesignerToolbarR);
    toolbar->setIconSize(QSize(fit(14), fit(14)));
    toolbar->addWidget(lspacer);
    toolbar->addWidget(issuesButton);
    toolbar->addWidget(searchButton);
    toolbar->addWidget(consoleButton);
    toolbar->addWidget(spacer);
    toolbar->addWidget(hideButton);
    toolbar->addWidget(rspacer);

    boxes[Issues] = new QWidget;
    boxes[Search] = new QWidget;
    boxes[Console] = new QWidget;

    boxes[Issues]->setSizePolicy(QSizePolicy::Expanding,
      QSizePolicy::Expanding);
    boxes[Search]->setSizePolicy(QSizePolicy::Expanding,
      QSizePolicy::Expanding);
    boxes[Console]->setSizePolicy(QSizePolicy::Expanding,
      QSizePolicy::Expanding);

    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(10);
    layout->addWidget(boxes[Issues]);
    layout->addWidget(boxes[Search]);
    layout->addWidget(boxes[Console]);
    layout->addWidget(toolbar);
    toolbar->setFixedHeight(fit(26));
}

void OutputBoxPrivate::handleHideButtonClicked()
{
    if (hideButton->toolTip().contains("Hide")) {
        hideButton->setToolTip("Show pane.");
        hideButton->setIcon(QIcon(":/resources/images/up-arrow.png"));
        parent->collapse();
    } else {
        hideButton->setToolTip("Hide pane.");
        hideButton->setIcon(QIcon(":/resources/images/down-arrow.png"));
        parent->expand();
    }
}

void OutputBoxPrivate::handleIssuesButtonClicked(bool val)
{
    if (val) {
        parent->_activeBoxType = Issues;
        for (auto box : boxes)
            box->hide();

        if (!parent->_collapsed)
            boxes.value(Issues)->show();
    }
}

void OutputBoxPrivate::handleSearchButtonClicked(bool val)
{
    if (val) {
        parent->_activeBoxType = Search;
        for (auto box : boxes)
            box->hide();

        if (!parent->_collapsed)
            boxes.value(Search)->show();
    }
}

void OutputBoxPrivate::handleConsoleButtonClicked(bool val)
{
    if (val) {
        parent->_activeBoxType = Console;
        for (auto box : boxes)
            box->hide();

        if (!parent->_collapsed)
            boxes.value(Console)->show();
    }
}

OutputBox::OutputBox(QWidget *parent)
    : QWidget(parent)
    , _d(new OutputBoxPrivate(this))
    , _lastHeight(SIZE_INITIAL.height())
    , _activeBoxType(Issues)
    , _collapsed(false)
{
    setMinimumHeight(HEIGHT_MIN);
    setMaximumHeight(HEIGHT_MAX);
    setActiveBox(_activeBoxType);
}

void OutputBox::setActiveBox(BoxType type)
{
    _activeBoxType = type;
    for (auto box : _d->boxes)
        box->hide();

    if (!_collapsed)
        _d->boxes.value(type)->show();

    switch (type) {
        case Issues:
            _d->issuesButton->setChecked(true);
            break;
        case Search:
            _d->searchButton->setChecked(true);
            break;
        case Console:
            _d->consoleButton->setChecked(true);
            break;
    }
}

void OutputBox::expand()
{
    _collapsed = false;
    _d->boxes.value(_activeBoxType)->show();
    setFixedHeight(_lastHeight);
    setMinimumHeight(HEIGHT_MIN);
    setMaximumHeight(HEIGHT_MAX);
    if (_splitterHandle)
        _splitterHandle->setEnabled(true);
}

void OutputBox::collapse()
{
    _collapsed = true;
    for (auto box : _d->boxes)
        box->hide();
    setFixedHeight(_d->toolbar->height());
    if (_splitterHandle)
        _splitterHandle->setDisabled(true);
}

void OutputBox::updateLastHeight()
{
    _lastHeight = height();
}

QSize OutputBox::sizeHint() const
{
    return SIZE_INITIAL;
}

BoxType OutputBox::activeBoxType() const
{
    return _activeBoxType;
}

bool OutputBox::collapsed() const
{
    return _collapsed;
}

void OutputBox::setSplitterHandle(QSplitterHandle* splitterHandle)
{
    _splitterHandle = splitterHandle;
}

#include "outputbox.moc"
