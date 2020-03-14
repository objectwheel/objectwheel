#ifndef WELCOMEWINDOW_H
#define WELCOMEWINDOW_H

#include <QWidget>

class StackedLayout;
class LoginWidget;
class ProjectsWidget;
class ProjectTemplatesWidget;
class ProjectDetailsWidget;
class RegistrationWidget;
class VerificationWidget;
class SucceedWidget;
class ForgetWidget;
class ResetWidget;
class ServerStatusWidget;

class WelcomeWindow final : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(WelcomeWindow)

public:
    explicit WelcomeWindow(QWidget* parent = nullptr);

    QSize sizeHint() const override;

private:
    void resizeEvent(QResizeEvent* event) override;

signals:
    void done();

private:
    StackedLayout* m_stackedLayout;
    LoginWidget* m_loginWidget;
    RegistrationWidget* m_registrationWidget;
    VerificationWidget* m_verificationWidget;
    SucceedWidget* m_succeedWidget;
    ForgetWidget* m_forgetWidget;
    ResetWidget* m_resetWidget;
    ProjectsWidget* m_projectsWidget;
    ProjectTemplatesWidget* m_projectTemplatesWidget;
    ProjectDetailsWidget* m_projectDetailsWidget;
    ServerStatusWidget* m_serverStatusWidget;
};

#endif // WELCOMEWINDOW_H