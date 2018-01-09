#ifndef WELCOMEWINDOW_H
#define WELCOMEWINDOW_H

#include <QWidget>

class View;
class LoginWidget;
class ProjectsWidget;
class NewProjectWidget;
class RegistrationWidget;

class WelcomeWindow : public QWidget
{
        Q_OBJECT

    public:
        enum Screens {
            Login,
            Registration,
            Projects,
            NewProject
        };

    public:
        explicit WelcomeWindow(QWidget* parent = nullptr);

    public slots:
        void showLogin();

    protected:
        void resizeEvent(QResizeEvent* event) override;

    private slots:
        void showProjects();

    signals:
        void done();
        void lazy();
        void busy(const QString& text);

    private:
        View* _view;
        LoginWidget* _loginWidget;
        ProjectsWidget* _projectsWidget;
        NewProjectWidget* _newProjectWidget;
        RegistrationWidget* _registrationWidget;
};

#endif // WELCOMEWINDOW_H