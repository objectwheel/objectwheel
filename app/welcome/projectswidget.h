#ifndef PROJECTSWIDGET_H
#define PROJECTSWIDGET_H

#include <QWidget>

class QLabel;
class ButtonSlice;
class QVBoxLayout;
class QListWidget;
class ProgressBar;
class FilterWidget;

class ProjectsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ProjectsWidget(QWidget* parent = nullptr);
    FilterWidget* filterWidget() const;

public slots:
    void refreshProjectList(bool selectionPreserved = false);

private slots:
    void onNewButtonClick();
    void onLoadButtonClick();
    void onExportButtonClick();
    void onImportButtonClick();
    void onSettingsButtonClick();
    void onProgressChange(int progress);
    void onFilterTextChange(const QString& text);

private:
    void lock();
    void unlock();
    void updateGadgetPositions();

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

signals:
    void done();
    void editProject(const QString& uid);
    void newProject(const QString& projectName);

private:
    QVBoxLayout* m_layout;
    QLabel* m_iconLabel;
    QLabel* m_welcomeLabel;
    QLabel* m_versionLabel;
    QLabel* m_projectsLabel;
    FilterWidget* m_filterWidget;
    QListWidget* m_listWidget;
    ButtonSlice* m_buttons;
    ButtonSlice* m_buttons_2;
    ProgressBar* m_progressBar;
};

#endif //  PROJECTSWIDGET_H