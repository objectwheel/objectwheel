#ifndef PROJECTSWIDGET_H
#define PROJECTSWIDGET_H

#include <QWidget>

class QLabel;
class ButtonSlice;
class QVBoxLayout;
class QListWidget;
class QProgressBar;
class SearchWidget;

class ProjectsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ProjectsWidget(QWidget* parent = nullptr);
    SearchWidget* searchWidget() const;

public slots:
    void refreshProjectList(bool selectionPreserved = false);

private slots:
    void onNewButtonClick();
    void onLoadButtonClick();
    void onExportButtonClick();
    void onImportButtonClick();
    void onSettingsButtonClick();
    void onProgressChange(int progress);
    void onSearchTextChange(const QString& text);
    void onRenderEngineConnectionStatusChange(bool connected);

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
    SearchWidget* m_searchWidget;
    QListWidget* m_listWidget;
    ButtonSlice* m_buttons;
    ButtonSlice* m_buttons_2;
    QProgressBar* m_progressBar;
    bool m_locked;
};

#endif //  PROJECTSWIDGET_H