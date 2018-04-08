#include <runpane.h>
#include <flatbutton.h>
#include <loadingbar.h>
#include <fit.h>
#include <windowmanager.h>
#include <projectbackend.h>
#include <interpreterbackend.h>
#include <consolebox.h>

#include <QTime>
#include <QPainter>
#include <QHBoxLayout>

RunPane::RunPane(ConsoleBox* consoleBox, QWidget *parent) : QWidget(parent)
  , m_consoleBox(consoleBox)
  , m_layout(new QHBoxLayout(this))
  , m_loadingBar(new LoadingBar)
  , m_runButton(new FlatButton)
  , m_stopButton(new FlatButton)
  , m_projectsButton(new FlatButton)
{
    m_layout->setSpacing(fit::fx(8));
    m_layout->setContentsMargins(fit::fx(8), 0, fit::fx(8), 0);

    m_layout->addWidget(m_runButton);
    m_layout->addWidget(m_stopButton);
    m_layout->addStretch();
    m_layout->addWidget(m_loadingBar);
    m_layout->addStretch();
    m_layout->addWidget(m_projectsButton);

    m_loadingBar->setFixedSize(fit::fx(QSizeF(481, 24)).toSize());

    m_runButton->setCursor(Qt::PointingHandCursor);
    m_runButton->setToolTip(tr("Run"));
    m_runButton->setIcon(QIcon(":/resources/images/run.png"));
    m_runButton->setFixedSize(fit::fx(QSizeF(38, 24)).toSize());
    m_runButton->settings().iconButton = true;
    connect(m_runButton, SIGNAL(clicked(bool)), SLOT(onRunButtonClick()));

    m_stopButton->setToolTip(tr("Stop"));
    m_stopButton->setCursor(Qt::PointingHandCursor);
    m_stopButton->setIcon(QIcon(":/resources/images/stop.png"));
    m_stopButton->setFixedSize(fit::fx(QSizeF(38, 24)).toSize());
    m_stopButton->settings().iconButton = true;
    connect(m_stopButton, SIGNAL(clicked(bool)), SLOT(onStopButtonClick()));
    connect(m_stopButton, SIGNAL(doubleClick()), SLOT(onStopButtonDoubleClick()));

    m_projectsButton->setToolTip(tr("Show Projects"));
    m_projectsButton->setCursor(Qt::PointingHandCursor);
    m_projectsButton->setIcon(QIcon(":/resources/images/projects.png"));
    m_projectsButton->setFixedSize(fit::fx(QSizeF(38, 24)).toSize());
    m_projectsButton->settings().iconButton = true;
    connect(m_projectsButton, SIGNAL(clicked(bool)), SLOT(onProjectsButtonClick()));

    m_loadingBar->setText(ProjectBackend::instance()->name() + tr(": <b>Ready</b>  |  Welcome to Objectwheel"));
    connect(ProjectBackend::instance(), &ProjectBackend::started, [=] {
        m_loadingBar->setText(ProjectBackend::instance()->name() + tr(": <b>Ready</b>  |  Welcome to Objectwheel"));
    });

    // FIXME
//    connect(SaveBackend::instance(), SIGNAL(doneExecuter(QString)), _loadingBar, SLOT(done(QString))); //TODO
//    connect(SaveBackend::instance(), SIGNAL(busyExecuter(int, QString)), _loadingBar, SLOT(busy(int,QString))); //TODO
}

void RunPane::reset()
{
    onStopButtonClick();
}

void RunPane::onStopButtonClick()
{
    InterpreterBackend::instance()->terminate();
    m_loadingBar->busy(
        0,
        ProjectBackend::instance()->name() +
        tr(": <b>Stopped</b>  |  Finished at ") +
        QTime::currentTime().toString()
    );
}

void RunPane::onStopButtonDoubleClick()
{
    InterpreterBackend::instance()->kill();
    m_loadingBar->busy(
        0,
        ProjectBackend::instance()->name() +
        tr(": <b>Stopped forcefully</b>  |  Finished at ") +
        QTime::currentTime().toString()
    );
}

void RunPane::onRunButtonClick()
{
    m_consoleBox->fade();

    if (!m_consoleBox->isClean())
        m_consoleBox->print("\n");

    m_consoleBox->printFormatted(
        tr("Starting ") +
        ProjectBackend::instance()->name() + "...\n",
        "#025dbf",
        QFont::DemiBold
    );

    m_consoleBox->scrollToEnd();

    InterpreterBackend::instance()->run();
}

void RunPane::onProjectsButtonClick()
{
    WindowManager::instance()->show(WindowManager::Welcome);
}

void RunPane::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QLinearGradient gradient(rect().topLeft(), rect().bottomLeft());
    gradient.setColorAt(0, "#2784E3");
    gradient.setColorAt(1, "#1068C6");

    painter.fillRect(rect(), gradient);

    painter.setPen("#0e5bad");
    painter.drawLine(QRectF(rect()).bottomLeft() + QPointF(0.5, -0.5), QRectF(rect()).bottomRight() + QPointF(-0.5, -0.5));
}