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

    public slots:
        void refreshProjectList();

    private slots:
        void startProject();
        void onNewButtonClick();
        void onLoadButtonClick();
        void onExportButtonClick();
        void onImportButtonClick();
        void onSettingsButtonClick();

    private:
        bool eventFilter(QObject *watched, QEvent *event) override;

    signals:
        void done();

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