#include <outputpane.h>
#include <flatbutton.h>
#include <css.h>
#include <fit.h>
#include <centralwidget.h>
#include <issuesbox.h>
#include <searchbox.h>
#include <consolebox.h>

#include <QSplitter>
#include <QSplitterHandle>
#include <QToolBar>
#include <QButtonGroup>
#include <QVBoxLayout>

#define HEIGHT_MIN (fit::fx(100))
#define HEIGHT_MAX (fit::fx(600))
#define SIZE_INITIAL (QSize(fit::fx(300), fit::fx(160)))
#define INTERVAL_SHINE (400)
#define COUNT_BLINK (3)

class OutputPanePrivate : public QObject
{
        Q_OBJECT

    public:
        OutputPanePrivate(OutputPane *parent);

    public slots:
        void handleHideButtonClicked();
        void handleIssuesButtonClicked(bool val);
        void handleSearchButtonClicked(bool val);
        void handleConsoleButtonClicked(bool val);

    public:
        OutputPane* parent;
        QVBoxLayout* layout;
        QToolBar* toolbar;
        FlatButton* hideButton;
        FlatButton* issuesButton;
        FlatButton* searchButton;
        FlatButton* consoleButton;
};

OutputPanePrivate::OutputPanePrivate(OutputPane* parent)
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

    hideButton->setFixedSize(fit::fx(QSizeF(20, 20)).toSize());
    hideButton->setIconSize(QSize(fit::fx(14), fit::fx(14)));
    hideButton->setCursor(Qt::PointingHandCursor);
    hideButton->setToolTip("Hide bar.");
    hideButton->setIcon(QIcon(":/resources/images/down-arrow.png"));
    hideButton->settings().topColor = "#0D74C8";
    hideButton->settings().bottomColor = hideButton->settings().topColor.darker(120);
    hideButton->settings().borderRadius = fit::fx(10);
    connect(hideButton, SIGNAL(clicked(bool)),
      SLOT(handleHideButtonClicked()));

    issuesButton->setCheckable(true);
    issuesButton->setText("Issues");
    issuesButton->setIconSize(QSize(fit::fx(14), fit::fx(14)));
    issuesButton->settings().borderRadius = fit::fx(5);
    issuesButton->setFixedWidth(fit::fx(100));
    issuesButton->setFixedHeight(fit::fx(22));
    issuesButton->setCursor(Qt::PointingHandCursor);
    issuesButton->setToolTip("Show recent issues.");
    issuesButton->setIcon(QIcon(":/resources/images/issues.png"));
    issuesButton->settings().topColor = "#697D8C";
    issuesButton->settings().bottomColor = issuesButton->settings().topColor.darker(120);
    issuesButton->settings().textColor = "#f0f4f7";
    issuesButton->setChecked(true);
    connect(issuesButton, SIGNAL(toggled(bool)),
      SLOT(handleIssuesButtonClicked(bool)));

    searchButton->settings().topColor = "#697D8C";
    searchButton->settings().bottomColor = searchButton->settings().topColor.darker(120);
    searchButton->settings().borderRadius = fit::fx(5);
    searchButton->settings().textColor = "#f0f4f7";
    searchButton->setCheckable(true);
    searchButton->setText("Search");
    searchButton->setIconSize(QSize(fit::fx(14), fit::fx(14)));
    searchButton->setFixedWidth(fit::fx(100));
    searchButton->setFixedHeight(fit::fx(22));
    searchButton->setCursor(Qt::PointingHandCursor);
    searchButton->setToolTip("Search words within project.");
    searchButton->setIcon(QIcon(":/resources/images/search.png"));
    connect(searchButton, SIGNAL(toggled(bool)),
      SLOT(handleSearchButtonClicked(bool)));

    consoleButton->settings().topColor = "#697D8C";
    consoleButton->settings().bottomColor = consoleButton->settings().topColor.darker(120);
    consoleButton->settings().borderRadius = fit::fx(5);
    consoleButton->settings().textColor = "#f0f4f7";
    consoleButton->setCheckable(true);
    consoleButton->setText("Console Output");
    consoleButton->setIconSize(QSize(fit::fx(14), fit::fx(14)));
    consoleButton->setFixedWidth(fit::fx(150));
    consoleButton->setFixedHeight(fit::fx(22));
    consoleButton->setCursor(Qt::PointingHandCursor);
    consoleButton->setToolTip("Show application output.");
    consoleButton->setIcon(QIcon(":/resources/images/console.png"));
    connect(consoleButton, SIGNAL(toggled(bool)),
      SLOT(handleConsoleButtonClicked(bool)));

    auto lspacer = new QWidget;
    lspacer->setFixedSize(fit::fx(1), fit::fx(1));
    auto spacer = new QWidget;
    spacer->setSizePolicy(QSizePolicy::Expanding,
      QSizePolicy::Expanding);
    auto rspacer = new QWidget;
    rspacer->setFixedSize(fit::fx(1), fit::fx(1));

    toolbar->setIconSize(QSize(fit::fx(14), fit::fx(14)));
    toolbar->addWidget(lspacer);
    toolbar->addWidget(issuesButton);
    toolbar->addWidget(searchButton);
    toolbar->addWidget(consoleButton);
    toolbar->setStyleSheet(QString("background: #D5D9DC; border-top: 1px solid #a5a9ac; spacing: %1").arg(fit::fx(5)));
    toolbar->addWidget(spacer);
    toolbar->addWidget(hideButton);
    toolbar->addWidget(rspacer);

    parent->_issuesBox = new IssuesBox(parent);
    parent->_searchBox = new SearchBox(parent);
    parent->_consoleBox = new ConsoleBox(parent);

    parent->_issuesBox->setSizePolicy(QSizePolicy::Expanding,
      QSizePolicy::Expanding);
    parent->_searchBox->setSizePolicy(QSizePolicy::Expanding,
      QSizePolicy::Expanding);
    parent->_consoleBox->setSizePolicy(QSizePolicy::Expanding,
      QSizePolicy::Expanding);

    QTimer::singleShot(100, [=] {
        connect(dW, SIGNAL(modeChanged()),
          parent->_issuesBox, SLOT(refresh()));
        // this->parent->collapse();
    });

    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(parent->_issuesBox);
    layout->addWidget(parent->_searchBox);
    layout->addWidget(parent->_consoleBox);
    layout->addWidget(toolbar);
    toolbar->setFixedHeight(fit::fx(26));
}

void OutputPanePrivate::handleHideButtonClicked()
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

void OutputPanePrivate::handleIssuesButtonClicked(bool val)
{
    if (val) {
        parent->_activeBox = parent->_issuesBox;
        parent->_issuesBox->hide();
        parent->_searchBox->hide();
        parent->_consoleBox->hide();

        if (!parent->_collapsed)
            parent->_issuesBox->show();
    }
}

void OutputPanePrivate::handleSearchButtonClicked(bool val)
{
    if (val) {
        parent->_activeBox = parent->_searchBox;
        parent->_issuesBox->hide();
        parent->_searchBox->hide();
        parent->_consoleBox->hide();

        if (!parent->_collapsed)
            parent->_searchBox->show();
    }
}

void OutputPanePrivate::handleConsoleButtonClicked(bool val)
{
    if (val) {
        parent->_activeBox = parent->_consoleBox;
        parent->_issuesBox->hide();
        parent->_searchBox->hide();
        parent->_consoleBox->hide();

        if (!parent->_collapsed)
            parent->_consoleBox->show();
    }
}

OutputPane::OutputPane(QWidget *parent)
    : QWidget(parent)
    , _d(new OutputPanePrivate(this))
    , _lastHeight(SIZE_INITIAL.height())
    , _collapsed(false)
{
    setMinimumHeight(HEIGHT_MIN);
    setMaximumHeight(HEIGHT_MAX);

    _activeBox = _issuesBox;
    _issuesBox->hide();
    _searchBox->hide();
    _consoleBox->hide();

    if (!_collapsed)
        _issuesBox->show();

    _d->issuesButton->setChecked(true);
}

void OutputPane::expand()
{
    _collapsed = false;
    _activeBox->show();
    setMinimumHeight(HEIGHT_MIN);
    setMaximumHeight(HEIGHT_MAX);
    if (_splitterHandle)
        _splitterHandle->setEnabled(true);
    if (_splitter) {
        auto sizes = _splitter->sizes();
        sizes[_splitter->indexOf(this)] = _lastHeight;
        _splitter->setSizes(sizes);
    }
}

void OutputPane::collapse()
{
    _collapsed = true;
    _issuesBox->hide();
    _searchBox->hide();
    _consoleBox->hide();
    setFixedHeight(_d->toolbar->height());
    if (_splitterHandle)
        _splitterHandle->setDisabled(true);
}

void OutputPane::updateLastHeight()
{
    _lastHeight = height();
}

void OutputPane::shine(OutputPane::Box type)
{
    static QList<Box> shineList;
    if (shineList.contains(type))
        return;
    shineList << type;

    auto timer = new QTimer;
    auto counter = new int(0);
    timer->start(0);

    connect(timer, &QTimer::timeout, this, [=] {
        timer->setInterval(INTERVAL_SHINE);
        if ((*counter)++ < (COUNT_BLINK * 2)) {
            auto btn = button(type);
            if ((*counter) % 2) {
                btn->settings().topColor = "#C2504B";
                btn->settings().bottomColor = "#B34B46";
                btn->triggerSettings();
            } else {
                btn->settings().topColor = "#697D8C";
                btn->settings().bottomColor = btn->settings().topColor.darker(120);
                btn->triggerSettings();
            }
        } else {
            shineList.removeAll(type);
            timer->stop();
            timer->deleteLater();
            delete counter;
        }
    });
}

QSize OutputPane::sizeHint() const
{
    return SIZE_INITIAL;
}

SearchBox* OutputPane::searchBox()
{
    return _searchBox;
}

OutputPane::Box OutputPane::activeBox() const
{
    if (_activeBox == _issuesBox)
        return Issues;
    if (_activeBox == _searchBox)
        return Search;
    else
        return Console;
}

bool OutputPane::isCollapsed() const
{
    return _collapsed;
}

ConsoleBox* OutputPane::consoleBox()
{
    return _consoleBox;
}

IssuesBox* OutputPane::issuesBox()
{
    return _issuesBox;
}

void OutputPane::setSplitter(QSplitter* splitter)
{
    _splitter = splitter;
}

void OutputPane::setSplitterHandle(QSplitterHandle* splitterHandle)
{
    _splitterHandle = splitterHandle;
}

FlatButton* OutputPane::button(OutputPane::Box type)
{
    switch (type) {
        case Issues:
            return _d->issuesButton;
        case Search:
            return _d->searchButton;
        case Console:
            return _d->consoleButton;
    }
    return nullptr;
}

void OutputPane::clear()
{
    _d->handleIssuesButtonClicked(true);
    _d->issuesButton->setText("Issues");
    _d->searchButton->setText("Search");
    _d->consoleButton->setText("Console Output");
    _issuesBox->clear();
    _searchBox->clear();
    _consoleBox->clear();
}

#include "outputpane.moc"
