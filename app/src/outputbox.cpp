#include <outputbox.h>
#include <flatbutton.h>
#include <css.h>
#include <fit.h>

#include <QSplitterHandle>
#include <QToolBar>
#include <QButtonGroup>
#include <QVBoxLayout>

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
        QMap<PaneType, QWidget*> panes;
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
    toolbar->setFixedHeight(fit(36));
    toolbar->setIconSize(QSize(fit(14), fit(14)));
    toolbar->addWidget(lspacer);
    toolbar->addWidget(issuesButton);
    toolbar->addWidget(searchButton);
    toolbar->addWidget(consoleButton);
    toolbar->addWidget(spacer);
    toolbar->addWidget(hideButton);
    toolbar->addWidget(rspacer);
    toolbar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    panes[ISSUES] = new QWidget;
    panes[SEARCH] = new QWidget;
    panes[CONSOLE] = new QWidget;

    panes[ISSUES]->setSizePolicy(QSizePolicy::Expanding,
      QSizePolicy::Expanding);
    panes[SEARCH]->setSizePolicy(QSizePolicy::Expanding,
      QSizePolicy::Expanding);
    panes[CONSOLE]->setSizePolicy(QSizePolicy::Expanding,
      QSizePolicy::Expanding);

    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(panes[ISSUES]);
    layout->addWidget(panes[SEARCH]);
    layout->addWidget(panes[CONSOLE]);
    layout->addWidget(toolbar);
}

void OutputBoxPrivate::handleHideButtonClicked()
{
    if (hideButton->toolTip().contains("Hide")) {
        hideButton->setToolTip("Show pane.");
        hideButton->setIcon(QIcon(":/resources/images/up-arrow.png"));
        parent->hidePanes();
    } else {
        hideButton->setToolTip("Hide pane.");
        hideButton->setIcon(QIcon(":/resources/images/down-arrow.png"));
        parent->showPanes();
    }
}

void OutputBoxPrivate::handleIssuesButtonClicked(bool val)
{
    if (val)
        parent->showPane(ISSUES);
}

void OutputBoxPrivate::handleSearchButtonClicked(bool val)
{
    if (val)
        parent->showPane(SEARCH);
}

void OutputBoxPrivate::handleConsoleButtonClicked(bool val)
{
    if (val)
        parent->showPane(CONSOLE);
}

OutputBox::OutputBox(QWidget *parent)
    : QWidget(parent)
    , _splitterHandle(nullptr)
    , _d(new OutputBoxPrivate(this))
    , _lastHeight(140)
{
    resize(width(), _lastHeight);
}

void OutputBox::showPane(PaneType pane)
{
    _d->panes[ISSUES]->hide();
    _d->panes[SEARCH]->hide();
    _d->panes[CONSOLE]->hide();

    if (_d->panes.value(pane))
        _d->panes.value(pane)->show();

    switch (pane) {
        case ISSUES:
            _d->issuesButton->setChecked(true);
            break;
        case SEARCH:
            _d->searchButton->setChecked(true);
            break;
        case CONSOLE:
            _d->consoleButton->setChecked(true);
            break;
    }
}

void OutputBox::hidePanes()
{
    _d->panes[ISSUES]->hide();
    _d->panes[SEARCH]->hide();
    _d->panes[CONSOLE]->hide();

    setFixedHeight(fit(27));
    _splitterHandle->setDisabled(true);
}

void OutputBox::showPanes()
{
    setMaximumHeight(fit(600));
    setMinimumHeight(fit(100));
    resize(width(), _lastHeight);
    _splitterHandle->setEnabled(true);
}

void OutputBox::setSplitterHandle(QSplitterHandle* splitterHandle)
{
    _splitterHandle = splitterHandle;
}

#include "outputbox.moc"
