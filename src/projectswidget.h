#ifndef PROJECTSWIDGET_H
#define PROJECTSWIDGET_H

#include <QWidget>

class QLabel;
class ButtonSlice;
class QVBoxLayout;
class QListWidget;

class ProjectsWidget : public QWidget
{
        Q_OBJECT

    public:
        explicit ProjectsWidget(QWidget* parent = nullptr);

    public:
        void refreshProjectList();

    protected:
        bool eventFilter(QObject *watched, QEvent *event) override;

    private:
        QVBoxLayout* _layout;
        QLabel* _iconLabel;
        QLabel* _welcomeLabel;
        QLabel* _versionLabel;
        QLabel* _projectsLabel;
        QListWidget* _listWidget;
        ButtonSlice* _buttons;
};

#endif // PROJECTSWIDGET_H