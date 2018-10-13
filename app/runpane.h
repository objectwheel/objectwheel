#ifndef RUNPANE_H
#define RUNPANE_H

#include <QToolBar>

class ConsolePane;
class DevicesButton;
class PushButton;
class RunPaneLoadingBar;

class RunPane : public QToolBar
{
    Q_OBJECT

public: // FIXME: ConsolePane* should not be passed to the constructor here
    explicit RunPane(ConsolePane* consolePane, QWidget *parent = nullptr);

public slots:
    void sweep();
    void busy(int progress, const QString& message);
    void done(const QString& message);
    void error(const QString& message);
    void setMessage(const QString& message);

private slots:
    void onRunButtonClick();
    void onStopButtonClick();
    void onProjectsButtonClick();

protected:
    void paintEvent(QPaintEvent *event) override;
    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;

private:
    ConsolePane* m_consolePane;
    PushButton* m_runButton;
    PushButton* m_stopButton;
    DevicesButton* m_devicesButton;
    PushButton* m_projectsButton;
    RunPaneLoadingBar* m_loadingBar;
};

#endif // RUNPANE_H