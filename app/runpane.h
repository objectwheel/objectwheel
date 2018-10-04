#ifndef RUNPANE_H
#define RUNPANE_H

#include <QToolBar>

class ConsoleBox;
class DevicesButton;
class RunPaneButton;
class RunPaneLoadingBar;

class RunPane : public QToolBar
{
    Q_OBJECT

public:
    explicit RunPane(ConsoleBox* consoleBox, QWidget *parent = nullptr);

public slots:
    void sweep();

protected:
    void paintEvent(QPaintEvent *event) override;
    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;

private slots:
    void onRunButtonClick();
    void onStopButtonClick();
    void onStopButtonDoubleClick();
    void onProjectsButtonClick();

private:
    ConsoleBox* m_consoleBox;
    RunPaneButton* m_runButton;
    RunPaneButton* m_stopButton;
    DevicesButton* m_devicesButton;
    RunPaneButton* m_projectsButton;
    RunPaneLoadingBar* m_loadingBar;
};

#endif // RUNPANE_H