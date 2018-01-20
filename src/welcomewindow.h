#ifndef WELCOMEWINDOW_H
#define WELCOMEWINDOW_H

#include <QWidget>

class View;
class QLabel;
class QVBoxLayout;
class LoginWidget;
class ProjectsWidget;
class NewProjectWidget;
class RegistrationWidget;
class VerificationWidget;
class RegistrationSucceedWidget;

class WelcomeWindow : public QWidget
{
        Q_OBJECT

    public:
        enum Screens {
            Login,
            Registration,
            Verification,
            Projects,
            NewProject,
            RegistrationSucceed
        };

    public:
        explicit WelcomeWindow(QWidget* parent = nullptr);

    public slots:
        void showLogin();

    private slots:
        void showProjects();

    signals:
        void done();
        void lazy();
        void busy(const QString& text);

    private:
        View* _view;
        QLabel* _legalLabel;
        QVBoxLayout* _layout;
        LoginWidget* _loginWidget;
        ProjectsWidget* _projectsWidget;
        NewProjectWidget* _newProjectWidget;
        RegistrationWidget* _registrationWidget;
        VerificationWidget* _verificationWidget;
        RegistrationSucceedWidget* _registrationSucceedWidget;
};

#endif // WELCOMEWINDOW_H