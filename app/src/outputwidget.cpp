#include <outputwidget.h>
#include <flatbutton.h>
#include <css.h>
#include <fit.h>
#include <designmanager.h>

#include <QSplitter>
#include <QSplitterHandle>
#include <QToolBar>
#include <QButtonGroup>
#include <QVBoxLayout>

#define HEIGHT_MIN (fit(100))
#define HEIGHT_MAX (fit(600))
#define SIZE_INITIAL (QSize(fit(300), fit(160)))
#define INTERVAL_SHINE (500)
#define COUNT_BLINK (3)

using namespace Fit;

class OutputWidgetPrivate : public QObject
{
        Q_OBJECT
    public:
        OutputWidgetPrivate(OutputWidget *parent);

    public slots:
        void handleHideButtonClicked();
        void handleIssuesButtonClicked(bool val);
        void handleSearchButtonClicked(bool val);
        void handleConsoleButtonClicked(bool val);

    public:
        OutputWidget* parent;
        QMap<BoxType, QWidget*> boxes;
        QVBoxLayout* layout;
        QToolBar* toolbar;
        FlatButton* hideButton;
        FlatButton* issuesButton;
        FlatButton* searchButton;
        FlatButton* consoleButton;
};

OutputWidgetPrivate::OutputWidgetPrivate(OutputWidget* parent)
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
    hideButton->setColor("#697D8C");
    hideButton->setCheckedColor("#4C5A66");
    connect(hideButton, SIGNAL(clicked(bool)),
      SLOT(handleHideButtonClicked()));

    issuesButton->setCheckable(true);
    issuesButton->setText("Issues");
    issuesButton->setFixedWidth(100);
    issuesButton->setFixedHeight(20);
    issuesButton->setCursor(Qt::PointingHandCursor);
    issuesButton->setToolTip("Show recent issues.");
    issuesButton->setIcon(QIcon(":/resources/images/issues.png"));
    issuesButton->setColor("#697D8C");
    issuesButton->setCheckedColor("#4C5A66");
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
    searchButton->setColor("#697D8C");
    searchButton->setCheckedColor("#4C5A66");
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
    consoleButton->setColor("#697D8C");
    consoleButton->setCheckedColor("#4C5A66");
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

    toolbar->setStyleSheet("background: #D5D9DC; border: none;");
    toolbar->setIconSize(QSize(fit(14), fit(14)));
    toolbar->addWidget(lspacer);
    toolbar->addWidget(issuesButton);
    toolbar->addWidget(searchButton);
    toolbar->addWidget(consoleButton);
    toolbar->addWidget(spacer);
    toolbar->addWidget(hideButton);
    toolbar->addWidget(rspacer);

    boxes[Issues] = new IssuesBox(parent);
    boxes[Search] = new SearchBox(parent);
    boxes[Console] = new ConsoleBox(parent);

    boxes.value(Issues)->setSizePolicy(QSizePolicy::Expanding,
      QSizePolicy::Expanding);
    boxes.value(Search)->setSizePolicy(QSizePolicy::Expanding,
      QSizePolicy::Expanding);
    boxes.value(Console)->setSizePolicy(QSizePolicy::Expanding,
      QSizePolicy::Expanding);

    QTimer::singleShot(100, [this] {
        connect(DesignManager::instance(), SIGNAL(modeChanged()),
          boxes.value(Issues), SLOT(refresh()));
    });

    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(boxes.value(Issues));
    layout->addWidget(boxes.value(Search));
    layout->addWidget(boxes.value(Console));
    layout->addWidget(toolbar);
    toolbar->setFixedHeight(fit(26));
}

void OutputWidgetPrivate::handleHideButtonClicked()
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

void OutputWidgetPrivate::handleIssuesButtonClicked(bool val)
{
    if (val) {
        parent->_activeBoxType = Issues;
        for (auto box : boxes)
            box->hide();

        if (!parent->_collapsed)
            boxes.value(Issues)->show();
    }
}

void OutputWidgetPrivate::handleSearchButtonClicked(bool val)
{
    if (val) {
        parent->_activeBoxType = Search;
        for (auto box : boxes)
            box->hide();

        if (!parent->_collapsed)
            boxes.value(Search)->show();
    }
}

void OutputWidgetPrivate::handleConsoleButtonClicked(bool val)
{
    if (val) {
        parent->_activeBoxType = Console;
        for (auto box : boxes)
            box->hide();

        if (!parent->_collapsed)
            boxes.value(Console)->show();
    }
}

OutputWidget::OutputWidget(QWidget *parent)
    : QWidget(parent)
    , _d(new OutputWidgetPrivate(this))
    , _lastHeight(SIZE_INITIAL.height())
    , _activeBoxType(Issues)
    , _collapsed(false)
{
    setMinimumHeight(HEIGHT_MIN);
    setMaximumHeight(HEIGHT_MAX);
    setActiveBox(_activeBoxType);
}

void OutputWidget::setActiveBox(BoxType type)
{
    _activeBoxType = type;
    for (auto box : _d->boxes)
        box->hide();

    if (!_collapsed)
        _d->boxes.value(type)->show();

    button(type)->setChecked(true);
}

void OutputWidget::expand()
{
    _collapsed = false;
    _d->boxes.value(_activeBoxType)->show();
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

void OutputWidget::collapse()
{
    _collapsed = true;
    for (auto box : _d->boxes)
        box->hide();
    setFixedHeight(_d->toolbar->height());
    if (_splitterHandle)
        _splitterHandle->setDisabled(true);
}

void OutputWidget::updateLastHeight()
{
    _lastHeight = height();
}

void OutputWidget::shine(BoxType type)
{
    static QList<BoxType> shineList;
    if (shineList.contains(type))
        return;
    shineList << type;

    auto timer = new QTimer;
    auto counter = new int(0);
    timer->start(INTERVAL_SHINE);

    connect(timer, &QTimer::timeout, this, [=] {
        if ((*counter)++ < (COUNT_BLINK * 2)) {
            auto btn = button(type);
            if ((*counter) % 2) {
                btn->setColor("#C63333");
                btn->setCheckedColor("#C63333");
            } else {
                btn->setColor("#697D8C");
                btn->setCheckedColor("#4C5A66");
            }
        } else {
            shineList.removeAll(type);
            timer->stop();
            timer->deleteLater();
            delete counter;
        }
    });
}

QSize OutputWidget::sizeHint() const
{
    return SIZE_INITIAL;
}

BoxType OutputWidget::activeBoxType() const
{
    return _activeBoxType;
}

bool OutputWidget::collapsed() const
{
    return _collapsed;
}

void OutputWidget::setSplitter(QSplitter* splitter)
{
    _splitter = splitter;
}

void OutputWidget::setSplitterHandle(QSplitterHandle* splitterHandle)
{
    _splitterHandle = splitterHandle;
}

FlatButton* OutputWidget::button(BoxType type)
{
    switch (type) {
        case Issues:
            return _d->issuesButton;
        case Search:
            return _d->searchButton;
        case Console:
            return _d->consoleButton;
    }
}

QWidget* OutputWidget::box(BoxType type)
{
    return _d->boxes[type];
}

#include "outputwidget.moc"
