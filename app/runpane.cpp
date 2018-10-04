#include <runpane.h>
#include <loadingbar.h>
#include <windowmanager.h>
#include <projectmanager.h>
#include <runmanager.h>
#include <consolebox.h>
#include <welcomewindow.h>
#include <transparentstyle.h>
#include <devicesbutton.h>
#include <smartspacer.h>
#include <runpanebutton.h>
#include <paintutils.h>

#include <QTime>
#include <QPainter>
#include <QHBoxLayout>
#include <QActionGroup>
#include <QMenu>

using namespace PaintUtils;

// TODO: Ask for "stop task"if main window closes before user closes the running project
RunPane::RunPane(ConsoleBox* consoleBox, QWidget *parent) : QWidget(parent)
  , m_consoleBox(consoleBox)
  , m_layout(new QHBoxLayout(this))
  , m_loadingBar(new LoadingBar)
  , m_runButton(new RunPaneButton)
  , m_stopButton(new RunPaneButton)
  , m_devicesButton(new DevicesButton)
  , m_projectsButton(new RunPaneButton)
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

    m_devicesButton->setCursor(Qt::PointingHandCursor);
    m_devicesButton->setToolTip(tr("Select target device"));

    //    TransparentStyle::attach(this);
    //    QTimer::singleShot(10000, [=]{
    //        TransparentStyle::attach(this);
    //        qDebug() << w->size();
    //    });

    //    m_loadingBar->setFixedSize(QSize(481, 24));

    QIcon icon;
    const QColor active = palette().buttonText().color().darker(180);
    const QColor normal = palette().buttonText().color().lighter(130);

    icon.addPixmap(renderColoredPixmap(":/utils/images/run_small@2x.png", normal, this), QIcon::Normal);
    icon.addPixmap(renderColoredPixmap(":/utils/images/run_small@2x.png", active, this), QIcon::Active);
    m_runButton->setCursor(Qt::PointingHandCursor);
    m_runButton->setToolTip(tr("Run"));
    m_runButton->setIconSize({16, 16});
    m_runButton->setIcon(icon);
    connect(m_runButton, SIGNAL(clicked(bool)), SLOT(onRunButtonClick()));

    icon.addPixmap(renderColoredPixmap(":/utils/images/stop_small@2x.png", normal, this), QIcon::Normal);
    icon.addPixmap(renderColoredPixmap(":/utils/images/stop_small@2x.png", active, this), QIcon::Active);
    m_stopButton->setCursor(Qt::PointingHandCursor);
    m_stopButton->setToolTip(tr("Stop"));
    m_stopButton->setIconSize({16, 16});
    m_stopButton->setIcon(icon);
    connect(m_stopButton, SIGNAL(clicked(bool)), SLOT(onStopButtonClick()));
    // FIXME: connect(m_stopButton, SIGNAL(doubleClicked()), SLOT(onStopButtonDoubleClick()));

    icon.addPixmap(renderMaskedPixmap(":/images/projects.png", normal, this), QIcon::Normal);
    icon.addPixmap(renderMaskedPixmap(":/images/projects.png", active, this), QIcon::Active);
    m_projectsButton->setCursor(Qt::PointingHandCursor);
    m_projectsButton->setToolTip(tr("Show Projects"));
    m_projectsButton->setIconSize({16, 16});
    m_projectsButton->setIcon(icon);
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
