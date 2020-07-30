#ifndef WELCOMEWINDOW_H
#define WELCOMEWINDOW_H

#include <QWidget>

class StackedLayout;
class LoginWidget;
class SignupWidget;
class SignupVerificationWidget;
class ResetPasswordWidget;
class ResetVerificationWidget;
class SucceedWidget;
class ProjectsWidget;
class ProjectTemplatesWidget;
class ProjectDetailsWidget;
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
    SignupWidget* m_signupWidget;
    SignupVerificationWidget* m_signupVerificationWidget;
    ResetPasswordWidget* m_resetPasswordWidget;
    ResetVerificationWidget* m_resetVerificationWidget;
    SucceedWidget* m_succeedWidget;
    ProjectsWidget* m_projectsWidget;
    ProjectTemplatesWidget* m_projectTemplatesWidget;
    ProjectDetailsWidget* m_projectDetailsWidget;
    ServerStatusWidget* m_serverStatusWidget;
};

#endif // WELCOMEWINDOW_H