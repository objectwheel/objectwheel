#include <welcomewindow.h>
#include <windowmanager.h>
#include <stackedlayout.h>
#include <loginwidget.h>
#include <projectswidget.h>
#include <projectdetailswidget.h>
#include <registrationwidget.h>
#include <verificationwidget.h>
#include <succeedwidget.h>
#include <forgetwidget.h>
#include <resetwidget.h>
#include <projecttemplateswidget.h>
#include <aboutwindow.h>
#include <utilityfunctions.h>
#include <serverstatuswidget.h>
#include <appconstants.h>

WelcomeWindow::WelcomeWindow(QWidget* parent) : QWidget(parent)
  , m_stackedLayout(new StackedLayout(this))
  , m_loginWidget(new LoginWidget(this))
  , m_registrationWidget(new RegistrationWidget(this))
  , m_verificationWidget(new VerificationWidget(this))
  , m_succeedWidget(new SucceedWidget(this))
  , m_forgetWidget(new ForgetWidget(this))
  , m_resetWidget(new ResetWidget(this))
  , m_projectsWidget(new ProjectsWidget(this))
  , m_projectTemplatesWidget(new ProjectTemplatesWidget(this))
  , m_projectDetailsWidget(new ProjectDetailsWidget(this))
  , m_serverStatusWidget(new ServerStatusWidget(this))
{
    setWindowTitle(AppConstants::LABEL);
    resize(sizeHint());
    move(UtilityFunctions::centerPos(size()));

    m_serverStatusWidget->adjustSize();
    m_serverStatusWidget->move(width() - m_serverStatusWidget->width() - 8, 8);
    m_serverStatusWidget->raise();
    connect(m_stackedLayout, &StackedLayout::currentChanged,
            m_serverStatusWidget, &ServerStatusWidget::raise);

    m_stackedLayout->addWidget(m_loginWidget);
    m_stackedLayout->addWidget(m_registrationWidget);
    m_stackedLayout->addWidget(m_verificationWidget);
    m_stackedLayout->addWidget(m_succeedWidget);
    m_stackedLayout->addWidget(m_forgetWidget);
    m_stackedLayout->addWidget(m_resetWidget);
    m_stackedLayout->addWidget(m_projectsWidget);
    m_stackedLayout->addWidget(m_projectTemplatesWidget);
    m_stackedLayout->addWidget(m_projectDetailsWidget);
    m_stackedLayout->setCurrentWidget(m_loginWidget);

    /**** ForgetWidget settings ****/
    connect(m_forgetWidget, &ForgetWidget::done, m_resetWidget, &ResetWidget::setEmail);
    connect(m_forgetWidget, &ForgetWidget::back, this, [=]
    {
        m_stackedLayout->setCurrentWidget(m_loginWidget);
    });
    connect(m_forgetWidget, &ForgetWidget::done, this, [=]
    {
        m_stackedLayout->setCurrentWidget(m_resetWidget);
    });
    /**** ResetWidget settings ****/
    connect(m_resetWidget, &ResetWidget::cancel, this, [=]
    {
        m_stackedLayout->setCurrentWidget(m_loginWidget);
    });
    connect(m_resetWidget, &ResetWidget::done, this, [=] {
        m_stackedLayout->setCurrentWidget(m_succeedWidget);
        m_succeedWidget->start();
        m_succeedWidget->update(tr("Succeed"),
                                tr("Your password has been successfully changed.\n"
                                   "You can continue by logging into the application with your new password."));
    });
    /**** LoginWidget settings ****/
    connect(m_loginWidget, &LoginWidget::signup, this, [=]
    {
        m_stackedLayout->setCurrentWidget(m_registrationWidget);
    });
    connect(m_loginWidget, &LoginWidget::resetPassword, this, [=]
    {
        m_stackedLayout->setCurrentWidget(m_forgetWidget);
    });
    connect(m_loginWidget, &LoginWidget::about, this, [=]
    {
        WindowManager::aboutWindow()->show();
        WindowManager::aboutWindow()->activateWindow();
    });
    connect(m_loginWidget, &LoginWidget::done, this, [=]
    {
        m_projectsWidget->refreshProjectList();
        m_stackedLayout->setCurrentWidget(m_projectsWidget);
    });
    /**** ProjectsWidget settings ****/
    connect(m_projectsWidget, &ProjectsWidget::done,
            this, &WelcomeWindow::done);
    connect(m_projectsWidget, &ProjectsWidget::newProject,
            m_projectTemplatesWidget, &ProjectTemplatesWidget::onNewProject);
    connect(m_projectsWidget, &ProjectsWidget::editProject,
            m_projectDetailsWidget, &ProjectDetailsWidget::onEditProject);
    connect(m_projectsWidget, &ProjectsWidget::newProject, this, [=]
    {
        m_stackedLayout->setCurrentWidget(m_projectTemplatesWidget);
    });
    connect(m_projectsWidget, &ProjectsWidget::editProject, this, [=]
    {
        m_stackedLayout->setCurrentWidget(m_projectDetailsWidget);
    });
    /**** ProjectTemplatesWidget settings ****/
    connect(m_projectTemplatesWidget, &ProjectTemplatesWidget::newProject,
            m_projectDetailsWidget, &ProjectDetailsWidget::onNewProject);
    connect(m_projectTemplatesWidget, &ProjectTemplatesWidget::back, this, [=]
    {
        m_projectsWidget->refreshProjectList();
        m_stackedLayout->setCurrentWidget(m_projectsWidget);
    });
    connect(m_projectTemplatesWidget, &ProjectTemplatesWidget::newProject, [=]
    {
        m_stackedLayout->setCurrentWidget(m_projectDetailsWidget);
    });
    /**** ProjectDetailsWidget settings ****/
    connect(m_projectDetailsWidget, &ProjectDetailsWidget::back, this, [=]
    {
        m_stackedLayout->setCurrentWidget(m_projectTemplatesWidget);
    });
    connect(m_projectDetailsWidget, &ProjectDetailsWidget::done, this, [=]
    {
        m_projectsWidget->refreshProjectList(true);
        m_stackedLayout->setCurrentWidget(m_projectsWidget);
    });
    /**** RegistrationWidget settings ****/
    connect(m_registrationWidget, &RegistrationWidget::back, this, [=]
    {
        m_stackedLayout->setCurrentWidget(m_loginWidget);
    });
    connect(m_registrationWidget, &RegistrationWidget::done,
            m_verificationWidget, &VerificationWidget::setEmail);
    connect(m_registrationWidget, &RegistrationWidget::done, this, [=]
    {
        m_stackedLayout->setCurrentWidget(m_verificationWidget);
    });
    /**** VerificationWidget settings ****/
    connect(m_verificationWidget, &VerificationWidget::cancel, this, [=]
    {
        m_stackedLayout->setCurrentWidget(m_loginWidget);
    });
    connect(m_verificationWidget, &VerificationWidget::done, this, [=]
    {
        m_stackedLayout->setCurrentWidget(m_succeedWidget);
        m_succeedWidget->start();
        m_succeedWidget->update(tr("Thank you for registering"),
                                tr("Your registration is completed. Thank you for choosing us.\n"
                                   "You can continue by logging into the application."));
    });
    /**** SucceedWidget settings ****/
    connect(m_succeedWidget, &SucceedWidget::done, this, [=]
    {
        m_stackedLayout->setCurrentWidget(m_loginWidget);
    });
}

QSize WelcomeWindow::sizeHint() const
{
    return QSize(980, 560);
}

void WelcomeWindow::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    m_serverStatusWidget->move(width() - m_serverStatusWidget->width() - 8, 8);
}
