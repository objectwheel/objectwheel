#include <outputpane.h>
#include <flatbutton.h>
#include <centralwidget.h>
#include <issuesbox.h>
#include <searchbox.h>
#include <consolebox.h>

#include <QSplitter>
#include <QSplitterHandle>
#include <QToolBar>
#include <QButtonGroup>
#include <QVBoxLayout>
#include <QTimer>

#define HEIGHT_MIN (100)
#define HEIGHT_MAX (600)
#define SIZE_INITIAL (QSize(300, 160))
#define INTERVAL_SHINE (400)
#define COUNT_BLINK (3)

OutputPane::OutputPane(QWidget *parent) : QWidget(parent)
  , m_layout(new QVBoxLayout(parent))
  , m_toolbar(new QToolBar)
  , m_hideButton(new FlatButton)
  , m_issuesButton(new FlatButton)
  , m_searchButton(new FlatButton)
  , m_consoleButton(new FlatButton)
  , m_lastHeight(SIZE_INITIAL.height())
  , m_collapsed(false)
{
    auto buttonGroup = new QButtonGroup;
    buttonGroup->addButton(m_issuesButton);
    buttonGroup->addButton(m_searchButton);
    buttonGroup->addButton(m_consoleButton);

    m_hideButton->setFixedSize(QSize(20, 20));
    m_hideButton->setIconSize(QSize(14, 14));
    m_hideButton->setCursor(Qt::PointingHandCursor);
    m_hideButton->setToolTip("Hide bar.");
    m_hideButton->setIcon(QIcon(":/images/down-arrow.png"));
    m_hideButton->settings().topColor = "#0D74C8";
    m_hideButton->settings().bottomColor = "#0b6ab8";
    m_hideButton->settings().borderRadius = 10;
    connect(m_hideButton, SIGNAL(clicked(bool)),
            SLOT(handleHideButtonClicked()));

    m_issuesButton->setCheckable(true);
    m_issuesButton->setText("Issues");
    m_issuesButton->setIconSize(QSize(14, 14));
    m_issuesButton->settings().borderRadius = 5;
    m_issuesButton->setFixedWidth(100);
    m_issuesButton->setFixedHeight(22);
    m_issuesButton->setCursor(Qt::PointingHandCursor);
    m_issuesButton->setToolTip("Show recent issues.");
    m_issuesButton->setIcon(QIcon(":/images/issues.png"));
    m_issuesButton->settings().topColor = "#5d6975";
    m_issuesButton->settings().bottomColor = "#515b66";
    m_issuesButton->settings().textColor = Qt::white;
    m_issuesButton->setChecked(true);
    connect(m_issuesButton, SIGNAL(toggled(bool)),
            SLOT(handleIssuesButtonClicked(bool)));

    m_searchButton->settings().topColor = "#5d6975";
    m_searchButton->settings().bottomColor = "#515b66";
    m_searchButton->settings().borderRadius = 5;
    m_searchButton->settings().textColor = Qt::white;
    m_searchButton->setCheckable(true);
    m_searchButton->setText("Search");
    m_searchButton->setIconSize(QSize(14, 14));
    m_searchButton->setFixedWidth(100);
    m_searchButton->setFixedHeight(22);
    m_searchButton->setCursor(Qt::PointingHandCursor);
    m_searchButton->setToolTip("Search words within project.");
    m_searchButton->setIcon(QIcon(":/images/search.png"));
    connect(m_searchButton, SIGNAL(toggled(bool)),
            SLOT(handleSearchButtonClicked(bool)));

    m_consoleButton->settings().topColor = "#5d6975";
    m_consoleButton->settings().bottomColor = "#515b66";
    m_consoleButton->settings().borderRadius = 5;
    m_consoleButton->settings().textColor = Qt::white;
    m_consoleButton->setCheckable(true);
    m_consoleButton->setText("Console Output");
    m_consoleButton->setIconSize(QSize(14, 14));
    m_consoleButton->setFixedWidth(150);
    m_consoleButton->setFixedHeight(22);
    m_consoleButton->setCursor(Qt::PointingHandCursor);
    m_consoleButton->setToolTip("Show application output.");
    m_consoleButton->setIcon(QIcon(":/images/console.png"));
    connect(m_consoleButton, SIGNAL(toggled(bool)),
            SLOT(handleConsoleButtonClicked(bool)));

    auto lspacer = new QWidget;
    lspacer->setFixedSize(1, 1);
    auto spacer = new QWidget;
    spacer->setSizePolicy(QSizePolicy::Expanding,
                          QSizePolicy::Expanding);
    auto rspacer = new QWidget;
    rspacer->setFixedSize(1, 1);

    m_toolbar->setIconSize(QSize(14, 14));
    m_toolbar->addWidget(lspacer);
    m_toolbar->addWidget(m_issuesButton);
    m_toolbar->addWidget(m_searchButton);
    m_toolbar->addWidget(m_consoleButton);
    m_toolbar->setStyleSheet(QString("background: #d4d4d4; border-top: 1px solid #a5a9ac; spacing: %1").arg(5));
    m_toolbar->addWidget(spacer);
    m_toolbar->addWidget(m_hideButton);
    m_toolbar->addWidget(rspacer);

    m_issuesBox = new IssuesBox(this);
    m_searchBox = new SearchBox(this);
    m_consoleBox = new ConsoleBox(this);

    m_issuesBox->setSizePolicy(QSizePolicy::Expanding,
                               QSizePolicy::Expanding);
    m_searchBox->setSizePolicy(QSizePolicy::Expanding,
                               QSizePolicy::Expanding);
    m_consoleBox->setSizePolicy(QSizePolicy::Expanding,
                                QSizePolicy::Expanding);

    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);
    m_layout->addWidget(m_issuesBox);
    m_layout->addWidget(m_searchBox);
    m_layout->addWidget(m_consoleBox);
    m_layout->addWidget(m_toolbar);
    m_toolbar->setFixedHeight(26);

    setMinimumHeight(HEIGHT_MIN);
    setMaximumHeight(HEIGHT_MAX);

    m_activeBox = m_issuesBox;
    m_issuesBox->hide();
    m_searchBox->hide();
    m_consoleBox->hide();

    if (!m_collapsed)
        m_issuesBox->show();

    m_issuesButton->setChecked(true);
}

void OutputPane::handleHideButtonClicked()
{
    if (m_hideButton->toolTip().contains("Hide")) {
        m_hideButton->setToolTip("Show pane.");
        m_hideButton->setIcon(QIcon(":/images/up-arrow.png"));
        collapse();
    } else {
        m_hideButton->setToolTip("Hide pane.");
        m_hideButton->setIcon(QIcon(":/images/down-arrow.png"));
        expand();
    }
}

void OutputPane::handleIssuesButtonClicked(bool val)
{
    if (val) {
        m_activeBox = m_issuesBox;
        m_issuesBox->hide();
        m_searchBox->hide();
        m_consoleBox->hide();

        if (!m_collapsed)
            m_issuesBox->show();
    }
}

void OutputPane::handleSearchButtonClicked(bool val)
{
    if (val) {
        m_activeBox = m_searchBox;
        m_issuesBox->hide();
        m_searchBox->hide();
        m_consoleBox->hide();

        if (!m_collapsed)
            m_searchBox->show();
    }
}

void OutputPane::handleConsoleButtonClicked(bool val)
{
    if (val) {
        m_activeBox = m_consoleBox;
        m_issuesBox->hide();
        m_searchBox->hide();
        m_consoleBox->hide();

        if (!m_collapsed)
            m_consoleBox->show();
    }
}

void OutputPane::expand()
{
    m_collapsed = false;
    m_activeBox->show();
    setMinimumHeight(HEIGHT_MIN);
    setMaximumHeight(HEIGHT_MAX);
    if (m_splitterHandle)
        m_splitterHandle->setEnabled(true);
    if (m_splitter) {
        auto sizes = m_splitter->sizes();
        sizes[m_splitter->indexOf(this)] = m_lastHeight;
        m_splitter->setSizes(sizes);
    }
}

void OutputPane::collapse()
{
    m_collapsed = true;
    m_issuesBox->hide();
    m_searchBox->hide();
    m_consoleBox->hide();
    setFixedHeight(m_toolbar->height());
    if (m_splitterHandle)
        m_splitterHandle->setDisabled(true);
}

void OutputPane::updateLastHeight()
{
    m_lastHeight = height();
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
                btn->settings().topColor = "#5d6975";
                btn->settings().bottomColor = "#515b66";
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
    return m_searchBox;
}

OutputPane::Box OutputPane::activeBox() const
{
    if (m_activeBox == m_issuesBox)
        return Issues;
    if (m_activeBox == m_searchBox)
        return Search;
    else
        return Console;
}

bool OutputPane::isCollapsed() const
{
    return m_collapsed;
}

ConsoleBox* OutputPane::consoleBox()
{
    return m_consoleBox;
}

IssuesBox* OutputPane::issuesBox()
{
    return m_issuesBox;
}

void OutputPane::setSplitter(QSplitter* splitter)
{
    m_splitter = splitter;
}

void OutputPane::setSplitterHandle(QSplitterHandle* splitterHandle)
{
    m_splitterHandle = splitterHandle;
}

FlatButton* OutputPane::button(OutputPane::Box type)
{
    switch (type) {
    case Issues:
        return m_issuesButton;
    case Search:
        return m_searchButton;
    case Console:
        return m_consoleButton;
    }
    return nullptr;
}

void OutputPane::sweep()
{
    m_lastHeight = SIZE_INITIAL.height();
    m_collapsed = false;

    m_issuesBox->sweep();
    m_searchBox->sweep();
    m_consoleBox->sweep();

    handleIssuesButtonClicked(true);

    m_issuesButton->setText("Issues");
    m_searchButton->setText("Search");
    m_consoleButton->setText("Console Output");
}

#include "outputpane.moc"
