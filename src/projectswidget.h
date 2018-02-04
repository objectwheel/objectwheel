#ifndef PROJECTSWIDGET_H
#define PROJECTSWIDGET_H

#include <QWidget>

class QLabel;
class ButtonSlice;
class QVBoxLayout;

class ProjectsWidget : public QWidget
{
        Q_OBJECT

    public:
        explicit ProjectsWidget(QWidget* parent = nullptr);

    private:
        QVBoxLayout* _layout;
        QLabel* _iconLabel;
        QLabel* _welcomeLabel;
        QLabel* _versionLabel;
        QLabel* _projectsLabel;
        ButtonSlice* _buttons;
};

#endif // PROJECTSWIDGET_H