#include <runpane.h>
#include <flatbutton.h>
#include <loadingbar.h>
#include <windowmanager.h>
#include <projectmanager.h>
#include <runmanager.h>
#include <consolebox.h>
#include <welcomewindow.h>
#include <transparentstyle.h>
#include <devicesbutton.h>
#include <smartspacer.h>

#include <QTime>
#include <QTimer>
#include <QPainter>
#include <QHBoxLayout>
#include <QActionGroup>
#include <QMenu>
#include <QDebug>

// TODO: Ask for "stop task"if main window closes before user closes the running project
RunPane::RunPane(ConsoleBox* consoleBox, QWidget *parent) : QWidget(parent)
  , m_consoleBox(consoleBox)
  , m_layout(new QHBoxLayout(this))
  , m_loadingBar(new LoadingBar)
  , m_runButton(new FlatButton)
  , m_stopButton(new FlatButton)
  , m_devicesButton(new DevicesButton)
  , m_projectsButton(new FlatButton)
{
    m_layout->setSpacing(8);
    m_layout->setContentsMargins(8, 0, 8, 0);
    m_layout->addWidget(m_runButton);
    m_layout->addWidget(m_stopButton);
    m_layout->addWidget(m_devicesButton);
    m_layout->addStretch();
    m_layout->addWidget(m_loadingBar);
    m_layout->addStretch();
    m_layout->addWidget(new SmartSpacer(Qt::Horizontal, {m_devicesButton}, 47, QSize(150, 24),
                                          m_devicesButton->sizePolicy().horizontalPolicy(),
                                          m_devicesButton->sizePolicy().verticalPolicy(), this));
    m_layout->addWidget(m_projectsButton);

    //    TransparentStyle::attach(this);
    //    QTimer::singleShot(10000, [=]{
    //        TransparentStyle::attach(this);
    //        qDebug() << w->size();
    //    });

    //    m_loadingBar->setFixedSize(QSize(481, 24));

    m_runButton->setCursor(Qt::PointingHandCursor);
    m_runButton->setToolTip(tr("Run"));
    m_runButton->setIcon(QIcon(":/images/run.png"));
    m_runButton->setFixedSize(QSize(39, 24));
    m_runButton->settings().iconButton = true;
    connect(m_runButton, SIGNAL(clicked(bool)), SLOT(onRunButtonClick()));

    m_stopButton->setToolTip(tr("Stop"));
    m_stopButton->setCursor(Qt::PointingHandCursor);
    m_stopButton->setIcon(QIcon(":/images/stop.png"));
    m_stopButton->setFixedSize(QSize(39, 24));
    m_stopButton->settings().iconButton = true;
    connect(m_stopButton, SIGNAL(clicked(bool)), SLOT(onStopButtonClick()));
    connect(m_stopButton, SIGNAL(doubleClick()), SLOT(onStopButtonDoubleClick()));

    m_devicesButton->setCursor(Qt::PointingHandCursor);
    m_devicesButton->setToolTip(tr("Select target device"));

    m_projectsButton->setToolTip(tr("Show Projects"));
    m_projectsButton->setCursor(Qt::PointingHandCursor);
    m_projectsButton->setIcon(QIcon(":/images/projects.png"));
    m_projectsButton->setFixedSize(QSize(39, 24));
    m_projectsButton->settings().iconButton = true;
    connect(m_projectsButton, SIGNAL(clicked(bool)), SLOT(onProjectsButtonClick()));

    connect(ProjectManager::instance(), &ProjectManager::started,
            [=] {
        m_loadingBar->setText(ProjectManager::name() + tr(": <b>Ready</b>  |  Welcome to Objectwheel"));
    });

    // FIXME
    //    connect(SaveManager::instance(), SIGNAL(doneExecuter(QString)), _loadingBar, SLOT(done(QString))); //TODO
    //    connect(SaveManager::instance(), SIGNAL(busyExecuter(int, QString)), _loadingBar, SLOT(busy(int,QString))); //TODO
}

void RunPane::sweep()
{
    onStopButtonClick();
}

void RunPane::onStopButtonClick()
{
    RunManager::terminate();
    m_loadingBar->busy(0, ProjectManager::name() + tr(": <b>Stopped</b>  |  Finished at ") +
                       QTime::currentTime().toString());
}

void RunPane::onStopButtonDoubleClick()
{
    RunManager::kill();
    m_loadingBar->busy(0, ProjectManager::name() + tr(": <b>Stopped forcefully</b>  |  Finished at ") +
                       QTime::currentTime().toString());
}

void RunPane::onRunButtonClick()
{
    m_consoleBox->fade();

    if (!m_consoleBox->isClean())
        m_consoleBox->print("\n");

    m_consoleBox->printFormatted(tr("Starting ") + ProjectManager::name() + "...\n", "#025dbf",
                                 QFont::DemiBold);

    m_consoleBox->scrollToEnd();

    RunManager::run();
}

void RunPane::onProjectsButtonClick()
{
    WindowManager::welcomeWindow()->show();
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
    painter.drawLine(QRectF(rect()).bottomLeft() + QPointF(0.5, -0.5), QRectF(rect()).bottomRight() +
                     QPointF(-0.5, -0.5));
}
