#include <welcomewindow.h>
#include <windowmanager.h>
#include <stackedlayout.h>
#include <loginwidget.h>
#include <projectswidget.h>
#include <projectdetailswidget.h>
#include <signupwidget.h>
#include <signupverificationwidget.h>
#include <succeedwidget.h>
#include <resetpasswordwidget.h>
#include <resetverificationwidget.h>
#include <projecttemplateswidget.h>
#include <aboutwindow.h>
#include <utilityfunctions.h>
#include <serverstatuswidget.h>
#include <appconstants.h>
#include <usermanager.h>

WelcomeWindow::WelcomeWindow(QWidget* parent) : QWidget(parent)
  , m_stackedLayout(new StackedLayout(this))
  , m_loginWidget(new LoginWidget(this))
  , m_signupWidget(new SignupWidget(this))
  , m_signupVerificationWidget(new SignupVerificationWidget(this))
  , m_resetPasswordWidget(new ResetPasswordWidget(this))
  , m_resetVerificationWidget(new ResetVerificationWidget(this))
  , m_succeedWidget(new SucceedWidget(this))
  , m_projectsWidget(new ProjectsWidget(this))
  , m_projectTemplatesWidget(new ProjectTemplatesWidget(this))
  , m_projectDetailsWidget(new ProjectDetailsWidget(this))
  , m_serverStatusWidget(new ServerStatusWidget(this))
{
    resize(sizeHint()); // Don't use adjustSize() on Windows
    move(UtilityFunctions::centerPos(size()));
    // setAttribute(Qt::WA_QuitOnClose, false); Since its possible that it may be the last window
    setWindowTitle(AppConstants::LABEL);

    m_serverStatusWidget->adjustSize();
    m_serverStatusWidget->move(width() - m_serverStatusWidget->width() - 8, 8);
    m_serverStatusWidget->raise();
    connect(m_stackedLayout, &StackedLayout::currentChanged,
            m_serverStatusWidget, &ServerStatusWidget::raise);

    m_stackedLayout->setContentsMargins(0, 0, 0, 0);
    m_stackedLayout->addWidget(m_loginWidget);
    m_stackedLayout->addWidget(m_signupWidget);
    m_stackedLayout->addWidget(m_signupVerificationWidget);
    m_stackedLayout->addWidget(m_resetPasswordWidget);
    m_stackedLayout->addWidget(m_resetVerificationWidget);
    m_stackedLayout->addWidget(m_succeedWidget);
    m_stackedLayout->addWidget(m_projectsWidget);
    m_stackedLayout->addWidget(m_projectTemplatesWidget);
    m_stackedLayout->addWidget(m_projectDetailsWidget);
    m_stackedLayout->setCurrentWidget(m_loginWidget);

    /**** ResetPasswordWidget settings ****/
    connect(m_resetPasswordWidget, &ResetPasswordWidget::done,
            m_resetVerificationWidget, &ResetVerificationWidget::setEmail);
    connect(m_resetPasswordWidget, &ResetPasswordWidget::back, this, [=]
    {
        m_stackedLayout->setCurrentWidget(m_loginWidget);
    });
    connect(m_resetPasswordWidget, &ResetPasswordWidget::done, this, [=]
    {
        m_stackedLayout->setCurrentWidget(m_resetVerificationWidget);
    });
    /**** ResetVerificationWidget settings ****/
    connect(m_resetVerificationWidget, &ResetVerificationWidget::cancel, this, [=]
    {
        m_stackedLayout->setCurrentWidget(m_loginWidget);
    });
    connect(m_resetVerificationWidget, &ResetVerificationWidget::done, this, [=] {
        m_stackedLayout->setCurrentWidget(m_succeedWidget);
        m_succeedWidget->play(tr("Succeed"),
                              tr("Your password has been successfully changed.\n"
                                 "You can continue by logging into the application with your new password."));
    });
    /**** LoginWidget settings ****/
    connect(m_loginWidget, &LoginWidget::signup, this, [=]
    {
        m_stackedLayout->setCurrentWidget(m_signupWidget);
    });
    connect(m_loginWidget, &LoginWidget::resetPassword, this, [=]
    {
        m_stackedLayout->setCurrentWidget(m_resetPasswordWidget);
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
        if (UserManager::plan() <= 0)
            emit subscriptionNeeded();
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
    /**** SignupWidget settings ****/
    connect(m_signupWidget, &SignupWidget::back, this, [=]
    {
        m_stackedLayout->setCurrentWidget(m_loginWidget);
    });
    connect(m_signupWidget, &SignupWidget::done,
            m_signupVerificationWidget, &SignupVerificationWidget::setEmail);
    connect(m_signupWidget, &SignupWidget::done, this, [=]
    {
        m_stackedLayout->setCurrentWidget(m_signupVerificationWidget);
    });
    /**** SignupVerificationWidget settings ****/
    connect(m_signupVerificationWidget, &SignupVerificationWidget::cancel, this, [=]
    {
        m_stackedLayout->setCurrentWidget(m_loginWidget);
    });
    connect(m_signupVerificationWidget, &SignupVerificationWidget::done, this, [=]
    {
        m_stackedLayout->setCurrentWidget(m_succeedWidget);
        m_succeedWidget->play(tr("Thank you for registering"),
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
